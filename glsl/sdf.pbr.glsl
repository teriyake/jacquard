void handleMaterialMaps(inout vec3 albedo, inout float metallic,
						inout float roughness, inout float ambientOcclusion,
						inout vec3 normal, inout float opacity)
{
	if (u_UseAlbedoMap)
	{
		albedo = pow(texture(u_AlbedoMap, fs_UV).rgb, vec3(2.2));
	}
	if (u_UseMetallicMap)
	{
		metallic = texture(u_MetallicMap, fs_UV).r;
	}
	if (u_UseRoughnessMap)
	{
		roughness = texture(u_RoughnessMap, fs_UV).r;
	}
	if (u_UseAOMap)
	{
		ambientOcclusion = texture(u_AOMap, fs_UV).r;
	}
	if (u_UseNormalMap)
	{
		vec3 T = normalize(u_Forward);
		vec3 B = normalize(u_Right);
		vec3 N = normalize(u_Up);
		mat3 TBN = mat3(T, B, N);

		normal = texture(u_NormalMap, fs_UV).rgb;
		normal = normalize(normal * 2. - 1.);
		normal = normalize(TBN * normal);
	}
	/*
	if(u_UseOpacityMap)
	{
		opacity = texture(u_OpacityMap, fs_UV).r;
	}
	*/
}

vec3 FresnelRoughness(float cosTheta, vec3 R, float roughness)
{
	return R + (max(vec3(1. - roughness), R) - R) *
				   pow(clamp(1. - cosTheta, 0., 1.), 5.);
}

vec3 calculateSSS(BSDF bsdf, vec3 wo, vec3 N, vec3 backlightIrradiance)
{
	vec3 lightDir = -wo;

	vec3 scatteredLightDir = lightDir + N * bsdf.sssDistortion;
	float lightReachingEye =
		pow(clamp(dot(wo, -scatteredLightDir), 0., 1.), bsdf.sssGlow) *
		bsdf.sssScale;
	float attenuation = 1.;
#define ATTENUATION 1
#if ATTENUATION
	attenuation = max(0., dot(N, lightDir) + dot(wo, -lightDir));
#endif
	float totalLight =
		attenuation * (lightReachingEye + bsdf.sssAmbient) * bsdf.thickness;

	return bsdf.albedo * backlightIrradiance * totalLight;
}

vec3 metallic_plastic_LTE(BSDF bsdf, vec3 wo)
{
	vec3 N = bsdf.nor;
	vec3 albedo = bsdf.albedo;
	float metallic = bsdf.metallic;
	float roughness = bsdf.roughness;
	float ambientOcclusion = bsdf.ao;
	float opacity = 1.;

	handleMaterialMaps(albedo, metallic, roughness, ambientOcclusion, N,
					   opacity);

	vec3 wh = N;
	vec3 wi = normalize(reflect(-wo, wh));

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);

	vec3 kS = FresnelRoughness(max(dot(wh, wo), 0.), F0, roughness);
	vec2 BRDF =
		texture(u_BRDFLookupTexture, vec2(max(dot(wh, wo), 0.), roughness)).rg;
	vec3 specular = textureLod(u_GlossyIrradianceMap, wi, roughness * 4.).rgb *
					(kS * BRDF.x + BRDF.y);
	specular = clamp(specular, 0., 100.);

	vec3 kD = vec3(1.) - kS;
	kD *= (1. - metallic);
	vec3 irradiance = texture(u_DiffuseIrradianceMap, N).rgb;
	vec3 diffuse = irradiance * albedo;

	vec3 ambient = (kD * diffuse + specular) * ambientOcclusion;

	vec3 sssContribution = vec3(0.);
	if (metallic < 0.9f && bsdf.thickness > 0.)
	{
		vec3 backlightIrradiance = texture(u_DiffuseIrradianceMap, -wo).rgb;
		backlightIrradiance = max(backlightIrradiance, vec3(0.));

		sssContribution = calculateSSS(bsdf, wo, N, backlightIrradiance);

		sssContribution *= (1. - metallic);
	}

	vec3 Lo = ambient + sssContribution;

	return Lo;
}
