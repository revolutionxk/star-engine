$input v_position, v_normal, v_tangent, v_texcoord0, v_viewDir, v_shadowCoord

#include <bgfx_shader.sh>
#include "material.sc"

void main()
{
    vec4 albedoTex = texture2D(s_texColor, v_texcoord0);
    vec3 normalTex = texture2D(s_texNormal, v_texcoord0).xyz;
    vec3 mrTex = texture2D(s_texMetallicRoughness, v_texcoord0).xyz;
    vec3 emissiveTex = texture2D(s_texEmissive, v_texcoord0).xyz;

    Material mat;

    vec4 albedoLin = vec4(srgbToLinear(albedoTex.rgb), albedoTex.a) * u_baseColor;
    mat.baseColor = mix(u_baseColor, albedoLin, u_texFlags.x);

    mat.metallic = mix(u_materialParams.x, u_materialParams.x * mrTex.b, u_texFlags.z);
    mat.roughness = mix(u_materialParams.y, u_materialParams.y * mrTex.g, u_texFlags.z);

    vec3 Nv = safeNormalize(v_normal, vec3(0.0, 1.0, 0.0));
    vec3 T = safeNormalize(v_tangent, vec3(1.0, 0.0, 0.0));
    vec3 B = cross(Nv, T);
    vec3 nTS = normalTex * 2.0 - 1.0;
    vec3 Nmap = safeNormalize(nTS.x * T + nTS.y * B + nTS.z * Nv, Nv);
    mat.normal = safeNormalize(mix(Nv, Nmap, u_texFlags.y), vec3(0.0, 1.0, 0.0));

    mat.emissive = mix(u_emissive.xyz, u_emissive.xyz * srgbToLinear(emissiveTex), u_texFlags.w);

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
    
    float reflectivity = mat.metallic * (1.0 - mat.roughness);
    gl_FragColor = vec4(color, reflectivity);
}
