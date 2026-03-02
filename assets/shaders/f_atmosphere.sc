$input v_worldDir

#include <bgfx_shader.sh>

uniform vec4 u_sunDirIntensity;
uniform vec4 u_atmosphereParams;
uniform vec4 u_zenithColor;
uniform vec4 u_camPos;

// Perez sky distribution function
float perez_func(float cos_theta, float cos_gamma, vec4 ABCDE)
{
    float A = ABCDE.x;
    float B = ABCDE.y;
    float C = ABCDE.z;
    float D = ABCDE.w;
    float E = 0.0; // unused in this simplified form
    float gamma = acos(clamp(cos_gamma, -1.0, 1.0));
    return (1.0 + A * exp(B / (cos_theta + 0.01))) * (1.0 + C * exp(D * gamma) + E * cos_gamma * cos_gamma);
}

// Full Perez with 5 coefficients
float perez(float cos_theta, float cos_gamma, float A, float B, float C, float D, float E)
{
    float gamma = acos(clamp(cos_gamma, -1.0, 1.0));
    return (1.0 + A * exp(B / max(cos_theta, 0.001)))
         * (1.0 + C * exp(D * gamma) + E * cos_gamma * cos_gamma);
}

// NOT FINISHED
void main()
{
    vec3 dir = normalize(v_worldDir);
    vec3 sunDir = normalize(u_sunDirIntensity.xyz);
    float sunIntensity = u_sunDirIntensity.w;
    float T = u_atmosphereParams.x; // turbidity [1..10]

    float cos_theta = max(dir.y, 0.001);
    float cos_gamma = dot(dir, sunDir);

    // Preetham coefficients for Y (luminance)
    float aY =  0.17872 * T - 1.46303;
    float bY = -0.35540 * T + 0.42749;
    float cY = -0.02266 * T + 5.32505;
    float dY =  0.12064 * T - 2.57705;
    float eY = -0.06696 * T + 0.37027;

    // Preetham coefficients for x chromaticity
    float ax =  0.00209 * T - 0.01647;
    float bx = -0.00375 * T + 0.00209;
    float cx = -0.02165 * T + 0.21552;
    float dx = -0.01247 * T - 0.08970;
    float ex = -0.00515 * T + 0.04517;

    // Preetham coefficients for y chromaticity
    float ay =  0.00317 * T - 0.02181;
    float by = -0.00610 * T + 0.00875;
    float cy = -0.01728 * T + 0.36215;
    float dy = -0.02211 * T - 0.03118;
    float ey = -0.00508 * T + 0.05953;

    // Sun zenith angle (angle between sun and vertical)
    float cos_sun_theta = max(sunDir.y, 0.001);
    float cos_sun_gamma_zenith = cos_sun_theta; // cos of angle to zenith from sun

    float F_zenith_Y = perez(1.0, cos_sun_gamma_zenith, aY, bY, cY, dY, eY);
    float F_zenith_x = perez(1.0, cos_sun_gamma_zenith, ax, bx, cx, dx, ex);
    float F_zenith_y = perez(1.0, cos_sun_gamma_zenith, ay, by, cy, dy, ey);

    float F_point_Y = perez(cos_theta, cos_gamma, aY, bY, cY, dY, eY);
    float F_point_x = perez(cos_theta, cos_gamma, ax, bx, cx, dx, ex);
    float F_point_y = perez(cos_theta, cos_gamma, ay, by, cy, dy, ey);

    // xyY at this sky point
    float Yp = u_zenithColor.z * F_point_Y / max(F_zenith_Y, 0.0001);
    float xp = u_zenithColor.x * F_point_x / max(F_zenith_x, 0.0001);
    float yp = u_zenithColor.y * F_point_y / max(F_zenith_y, 0.0001);

    // xyY -> XYZ
    float X = (yp > 0.0001) ? (xp / yp) * Yp : 0.0;
    float Z = (yp > 0.0001) ? ((1.0 - xp - yp) / yp) * Yp : 0.0;
    vec3 xyy_to_xyz = vec3(X, Yp, Z);

    // XYZ -> linear RGB (Rec. 709)
    float r =  3.240479 * xyy_to_xyz.x - 1.537150 * xyy_to_xyz.y - 0.498535 * xyy_to_xyz.z;
    float g = -0.969256 * xyy_to_xyz.x + 1.875991 * xyy_to_xyz.y + 0.041556 * xyy_to_xyz.z;
    float b =  0.055648 * xyy_to_xyz.x - 0.204043 * xyy_to_xyz.y + 1.057311 * xyy_to_xyz.z;
    vec3 sky_color = max(vec3(r, g, b), vec3_splat(0.0));

    // Sun disc
    float sun_disc = smoothstep(0.9995, 0.9999, cos_gamma);
    sky_color += sun_disc * vec3(1.0, 0.9, 0.7) * sunIntensity * 10.0;

    // Horizon fade: below horizon just show horizon color
    float horizon = smoothstep(-0.05, 0.0, dir.y);
    vec3 horizon_color = sky_color * 0.4 + vec3(0.6, 0.5, 0.4) * 0.1;
    sky_color = mix(horizon_color, sky_color, horizon);

    gl_FragColor = vec4(sky_color, 1.0);
}
