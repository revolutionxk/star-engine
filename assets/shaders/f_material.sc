$input v_position, v_normal, v_tangent, v_texcoord0, v_viewDir, v_shadowCoord

#include <bgfx_shader.sh>
#include "material.sc"

void main()
{
    Material mat;
    mat.baseColor = u_baseColor;
    mat.metallic = u_materialParams.x;
    mat.roughness = u_materialParams.y;
    mat.normal = safeNormalize(v_normal, vec3(0.0, 1.0, 0.0));
    mat.emissive = u_emissive.xyz;

    mat = initMaterial(mat);

    vec3 view_dir = safeNormalize(u_camPos.xyz - v_position, vec3(0.0, 0.0, 1.0));

    vec3 sun_dir = safeNormalize(u_envSunDir.xyz, vec3(0.0, 1.0, 0.0));
    float shadow = computeShadow(v_shadowCoord, dot(mat.normal, sun_dir));
    vec3 sun_lighting = evaluateLightingFiltered(v_position, mat.normal, view_dir, mat, -1.0, 0.5) * shadow;
    vec3 punctual_lighting = evaluateLightingFiltered(v_position, mat.normal, view_dir, mat, 0.5, 3.0);
    vec3 direct_lighting = sun_lighting + punctual_lighting;
    vec3 sky_irradiance = u_ambientColor.xyz * u_ambientColor.w;
    vec3 ground_irradiance = u_groundColor.xyz * u_ambientColor.w * 0.5;
    vec3 ambient_lighting = evaluateAmbient(mat.normal, view_dir, mat, sky_irradiance, ground_irradiance);

    float exposure = u_groundColor.w > 0.0 ? u_groundColor.w : 1.0;

    vec3 color = sanitizeColor(ambient_lighting + direct_lighting + mat.emissive, 4096.0) * exposure;
    gl_FragColor = vec4(color, mat.baseColor.a);
}
