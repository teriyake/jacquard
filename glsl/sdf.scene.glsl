const vec3 fabricBounds = vec3(1., 1.5, 0.5);

float sceneSDF(vec3 query)
{
	vec3 p_orig = query;

	float wrinkleFrequency1 = 9.5;
	float wrinkleAmplitude1 = 0.01;
	float noiseFrequency = 2.7;
	float noiseAmplitude = 0.07;
	float displacement =
		sin(p_orig.x * wrinkleFrequency1 + p_orig.y * 0.5) * wrinkleAmplitude1;
	displacement +=
		(fbm(p_orig * noiseFrequency, 3, 0.5) * 2. - 1.) * noiseAmplitude;

	vec3 p_distorted = p_orig;
	p_distorted.z -= displacement;

	ThreadMaterialProperties warpProps = getMaterialProperties(u_WarpMaterial);
	ThreadMaterialProperties weftProps = getMaterialProperties(u_WeftMaterial);

	DraftWeaveInfo weaveInfo = SDF_DraftWeave(p_distorted, warpProps.radius,
											  weftProps.radius, u_PatternScale);
	float weaveSDF = weaveInfo.dist;

	vec3 boxMin = -fabricBounds;
	vec3 boxMax = fabricBounds;
	float boundsSDF = SDF_Box(p_orig, fabricBounds);

	float distToSideEdge =
		min(abs(p_orig.x - boxMax.x), abs(p_orig.x - boxMin.x));
	float sideNoiseInfluence =
		1. - smoothstep(0., SIDE_EDGE_FALLOFF, distToSideEdge);

	float modifiedBoundsSDF = boundsSDF;
	if (sideNoiseInfluence > 0.)
	{
		float sideNoise =
			(fbm(vec3(p_orig.y * SIDE_EDGE_NOISE_FREQ, 1., 1.), 2, 0.5) * 2. -
			 1.);
		sideNoise *= SIDE_EDGE_NOISE_AMP * sideNoiseInfluence;
		modifiedBoundsSDF -= sideNoise;
	}

	float fabricBodySDF = max(weaveSDF, boundsSDF);

	// float topHemSDF = SDF_TopHem(p_orig, boxMin, boxMax);
	// float fringeSDF = SDF_Fringes(p_orig, boxMin, boxMax);

	float topHemSDF = 1e6;
	if (p_orig.y > boxMax.y - TOP_HEM_HEIGHT * 0.5 &&
		p_orig.y < boxMax.y + TOP_HEM_HEIGHT * 1.5 &&
		p_orig.x > boxMin.x - TOP_HEM_ROUNDING &&
		p_orig.x < boxMax.x + TOP_HEM_ROUNDING)
	{
		topHemSDF = SDF_TopHem(p_orig, boxMin, boxMax);
	}

	float fringeSDF = 1e6;
	float fringeCheckMargin = 0.05;
	if (p_orig.y < boxMin.y + fringeCheckMargin &&
		p_orig.y > boxMin.y - FRINGE_LENGTH * (1. + FRINGE_LENGTH_VARIATION) -
					   fringeCheckMargin &&
		p_orig.x > boxMin.x - fringeCheckMargin &&
		p_orig.x < boxMax.x + fringeCheckMargin)
	{
		fringeSDF = SDF_Fringes(p_orig, boxMin, boxMax);
	}

	return min(min(fabricBodySDF, fringeSDF), topHemSDF);
}


BSDF sceneBSDF(vec3 query, vec3 N_geom)
{
	vec3 p_orig = query;
	vec3 boxMin = -fabricBounds;
	vec3 boxMax = fabricBounds;

	float fringeDist = SDF_Fringes(p_orig, boxMin, boxMax);
	if (fringeDist < HIT_THRESHOLD * 1.5)
	{
		ThreadMaterialProperties fringeProps =
			getMaterialProperties(u_WarpMaterial);
		vec3 finalNormal = N_geom;
		vec3 albedo = pow(u_WarpColorBase, vec3(2.2));
		float metallic = fringeProps.metallic;
		float roughness = fringeProps.roughness;
		float ao = 0.85;
		float fuzzAmount = fringeProps.fuzzAmount * 1.2;
		float fuzzScale = fringeProps.fuzzScale;
		float colorVariation = fringeProps.colorVariation;

		if (fuzzAmount > 0. || colorVariation > 0.)
		{
			vec3 noisePos = query * fuzzScale;
			float noiseVal = fbm(noisePos, 4, 0.5);
			if (fuzzAmount > 0.)
			{
				float noiseX = fbm(noisePos + vec3(13.7, 0, 0), 4, 0.5);
				float noiseY = fbm(noisePos + vec3(0, 23.1, 0), 4, 0.5);
				float noiseZ = fbm(noisePos + vec3(5.1, 9.3, 1.7), 4, 0.5);
				vec3 tangentNoise = vec3(noiseX, noiseY, noiseZ) * 2. - 1.;
				if (length(tangentNoise) > 1e-5)
				{
					vec3 perturbation = normalize(
						tangentNoise - N_geom * dot(N_geom, tangentNoise));
					finalNormal = normalize(N_geom + perturbation * fuzzAmount);
				}
				roughness += abs(noiseVal * 2. - 1.) * fuzzAmount * 0.5;
			}
			if (colorVariation > 0.)
			{
				albedo *= (1. + (noiseVal * 2. - 1.) * colorVariation);
			}
			roughness = clamp(roughness, 0.05, 1.);
			albedo = clamp(albedo, 0., 1.);
		}

		return BSDF(query, finalNormal, albedo, metallic, roughness, ao, 0.,
					SSS_DEFAULT_DISTORTION, SSS_DEFAULT_GLOW, SSS_DEFAULT_SCALE,
					SSS_DEFAULT_AMBIENT, 0.);
	}

	float hemDist = SDF_TopHem(p_orig, boxMin, boxMax);
	if (hemDist < HIT_THRESHOLD * 1.5)
	{
		ThreadMaterialProperties hemProps =
			getMaterialProperties(u_WeftMaterial);

		vec3 finalNormal = N_geom;
		vec3 albedo = pow(u_WeftColorBase, vec3(2.2));
		float metallic = hemProps.metallic;
		float roughness = hemProps.roughness * 1.1;
		float ao = 0.9;
		float fuzzAmount = hemProps.fuzzAmount * 0.5;
		float fuzzScale = hemProps.fuzzScale;
		float colorVariation = hemProps.colorVariation * 0.8;

		if (fuzzAmount > 0. || colorVariation > 0.)
		{
			vec3 noisePos = query * fuzzScale;
			float noiseVal = fbm(noisePos, 4, 0.5);
			if (fuzzAmount > 0.)
			{
				float noiseX = fbm(noisePos + vec3(13.7, 0, 0), 4, 0.5);
				float noiseY = fbm(noisePos + vec3(0, 23.1, 0), 4, 0.5);
				float noiseZ = fbm(noisePos + vec3(5.1, 9.3, 1.7), 4, 0.5);
				vec3 tangentNoise = vec3(noiseX, noiseY, noiseZ) * 2. - 1.;
				if (length(tangentNoise) > 1e-5)
				{
					vec3 perturbation = normalize(
						tangentNoise - N_geom * dot(N_geom, tangentNoise));
					finalNormal = normalize(N_geom + perturbation * fuzzAmount);
				}
				roughness += abs(noiseVal * 2. - 1.) * fuzzAmount * 0.5;
			}
			if (colorVariation > 0.)
			{
				albedo *= (1. + (noiseVal * 2. - 1.) * colorVariation);
			}
			roughness = clamp(roughness, 0.05, 1.);
			albedo = clamp(albedo, 0., 1.);
		}

		return BSDF(query, finalNormal, albedo, metallic, roughness, ao, 0.,
					SSS_DEFAULT_DISTORTION, SSS_DEFAULT_GLOW, SSS_DEFAULT_SCALE,
					SSS_DEFAULT_AMBIENT, 0.);
	}

	return BSDF_DraftWeave(query, N_geom);
}
