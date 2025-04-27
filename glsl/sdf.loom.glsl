struct ThreadMaterialProperties
{
	float radius;
	float roughness;
	float metallic;
	float fuzzAmount;
	float fuzzScale;
	float colorVariation;
	float sssDistortion;
	float sssGlow;
	float sssScale;
	float sssAmbient;
	float sssMaxThicknessCheck;
};

ThreadMaterialProperties getMaterialProperties(int materialType)
{
	ThreadMaterialProperties props;
	props.sssDistortion = SSS_DEFAULT_DISTORTION;
	props.sssGlow = SSS_DEFAULT_GLOW;
	props.sssScale = SSS_DEFAULT_SCALE;
	props.sssAmbient = SSS_DEFAULT_AMBIENT;
	props.sssMaxThicknessCheck = 0.001;

	if (materialType == MAT_SILK)
	{
		props.radius = 0.005;
		props.roughness = 0.15;
		props.metallic = 0.;
		props.fuzzAmount = 0.05;
		props.fuzzScale = 300.;
		props.colorVariation = 0.05;
		props.sssDistortion = 0.15;
		props.sssGlow = 8.;
		props.sssScale = 0.6;
		props.sssAmbient = 0.02;
	}
	else if (materialType == MAT_WOOL_YARN)
	{
		props.radius = 0.015;
		props.roughness = 0.9;
		props.metallic = 0.;
		props.fuzzAmount = 0.7;
		props.fuzzScale = 150.;
		props.colorVariation = 0.3;
	}
	else if (materialType == MAT_LINEN)
	{
		props.radius = 0.009;
		props.roughness = 0.75;
		props.metallic = 0.;
		props.fuzzAmount = 0.2;
		props.fuzzScale = 250.;
		props.colorVariation = 0.25;
	}
	else if (materialType == MAT_METALLIC_GOLD)
	{
		props.radius = 0.006;
		props.roughness = 0.25;
		props.metallic = 0.9;
		props.fuzzAmount = 0.;
		props.fuzzScale = 300.;
		props.colorVariation = 0.02;
	}
	else if (materialType == MAT_POLYESTER)
	{
		props.radius = 0.007;
		props.roughness = 0.4;
		props.metallic = 0.;
		props.fuzzAmount = 0.02;
		props.fuzzScale = 300.;
		props.colorVariation = 0.01;
	}
	else
	{
		props.radius = 0.008;
		props.roughness = 0.8;
		props.metallic = 0.;
		props.fuzzAmount = 0.3;
		props.fuzzScale = 200.;
		props.colorVariation = 0.15;
	}
	return props;
}


struct DraftWeaveInfo
{
	float dist;
	bool isWarp;
	float zOffset;
	int colIndex;
	int rowIndex;
};


int weave(ivec2 patternIndices)
{
	int warpIdx = patternIndices.x % u_WarpThreads;
	int pickIdx = patternIndices.y % u_WeftPicks;

	if (warpIdx < 0)
		warpIdx += u_WarpThreads;
	if (pickIdx < 0)
		pickIdx += u_WeftPicks;

	int shaft = u_Threading[warpIdx];
	if (shaft < 0 || shaft >= u_Shafts)
		return 0;

	int treadle = u_Treadling[pickIdx];
	if (treadle < 0 || treadle >= u_Treadles)
		return 0;

	int tieupIdx = shaft * u_Treadles + treadle;
	if (tieupIdx < 0 || tieupIdx >= (u_Shafts * u_Treadles))
		return 0;

	int warpIsUp = u_Tieup[tieupIdx];

	return warpIsUp;
}

int getDrawdownValueLoom(vec2 worldPos, vec2 patternScale)
{
	vec2 patternUV = worldPos / patternScale;

	vec2 fractionalUV = fract(patternUV);

	int colIndex = int(floor(fractionalUV.x * float(u_WarpThreads)));
	int rowIndex = int(floor(fractionalUV.y * float(u_WeftPicks)));

	colIndex = clamp(colIndex, 0, u_WarpThreads - 1);
	rowIndex = clamp(rowIndex, 0, u_WeftPicks - 1);

	return weave(ivec2(colIndex, rowIndex));
}

DraftWeaveInfo SDF_DraftWeave(vec3 query, float warpRadius, float weftRadius,
							  vec2 patternScale)
{
	if (u_WarpThreads <= 0 || u_WeftPicks <= 0)
	{
		return DraftWeaveInfo(1e6, false, 0., -1, -1);
	}
	float spacingX = patternScale.x / float(u_WarpThreads);
	float spacingY = patternScale.y / float(u_WeftPicks);
	float halfSpacingX = spacingX * 0.5;
	float halfSpacingY = spacingY * 0.5;

	int drawdownVal = getDrawdownValueLoom(query.xy, patternScale);
	float warpZOffset, weftZOffset;
	float offsetAmount = (warpRadius + weftRadius) * 0.5 * 0.9;

	if (drawdownVal > 0)
	{
		warpZOffset = offsetAmount;
		weftZOffset = -offsetAmount;
	}
	else
	{
		warpZOffset = -offsetAmount;
		weftZOffset = offsetAmount;
	}

	vec3 qWarp = query;
	qWarp.x = mod(query.x + halfSpacingX, spacingX) - halfSpacingX;
	qWarp.z -= warpZOffset;
	float dWarp = length(qWarp.xz) - warpRadius;

	vec3 qWeft = query;
	qWeft.y = mod(query.y + halfSpacingY, spacingY) - halfSpacingY;
	qWeft.z -= weftZOffset;
	float dWeft = length(qWeft.yz) - weftRadius;

	vec2 patternUV = query.xy / patternScale;
	vec2 fractionalUV = fract(patternUV);
	int colIndex = int(floor(fractionalUV.x * float(u_WarpThreads)));
	int rowIndex = int(floor(fractionalUV.y * float(u_WeftPicks)));
	colIndex = clamp(colIndex, 0, u_WarpThreads - 1);
	rowIndex = clamp(rowIndex, 0, u_WeftPicks - 1);

	DraftWeaveInfo result;
	if (dWarp < dWeft)
	{
		result.dist = dWarp;
		result.isWarp = true;
		result.zOffset = warpZOffset;
	}
	else
	{
		result.dist = dWeft;
		result.isWarp = false;
		result.zOffset = weftZOffset;
	}
	result.colIndex = colIndex;
	result.rowIndex = rowIndex;
	return result;
}

BSDF BSDF_DraftWeave(vec3 query, vec3 N_geom)
{
	ThreadMaterialProperties warpProps = getMaterialProperties(u_WarpMaterial);
	ThreadMaterialProperties weftProps = getMaterialProperties(u_WeftMaterial);

	DraftWeaveInfo hitInfo = SDF_DraftWeave(query, warpProps.radius,
											weftProps.radius, u_PatternScale);

	vec3 baseColorLinear;
	float metallic;
	float baseRoughness;
	float currentFuzzAmount;
	float currentFuzzScale;
	float currentColorVariation;
	float ao = 0.9;
	float sssThickness = 0.;
	float sssDistortion;
	float sssGlow;
	float sssScale;
	float sssAmbient;
	float sssMaxThicknessCheck;

	ThreadMaterialProperties currentProps;

	if (hitInfo.isWarp)
	{
		currentProps = warpProps;
		baseColorLinear = pow(u_WarpColorBase, vec3(2.2));
	}
	else
	{
		currentProps = weftProps;
		baseColorLinear = pow(u_WeftColorBase, vec3(2.2));
	}

	baseRoughness = currentProps.roughness;
	metallic = currentProps.metallic;
	currentFuzzAmount = currentProps.fuzzAmount;
	currentFuzzScale = currentProps.fuzzScale;
	currentColorVariation = currentProps.colorVariation;
	sssDistortion = currentProps.sssDistortion;
	sssGlow = currentProps.sssGlow;
	sssScale = currentProps.sssScale;
	sssAmbient = currentProps.sssAmbient;
	sssMaxThicknessCheck = currentProps.sssMaxThicknessCheck;


	if (sssScale > 0.)
	{
		sssThickness = hitInfo.isWarp ? 0.3 : 0.7;
	}

	if (metallic > 0.5 &&
		((hitInfo.isWarp && u_WarpMaterial == MAT_METALLIC_GOLD) ||
		 (!hitInfo.isWarp && u_WeftMaterial == MAT_METALLIC_GOLD)))
	{
		baseColorLinear = pow(vec3(1., 0.71, 0.29), vec3(2.2));
	}


	vec3 finalNormal = N_geom;
	vec3 albedo = baseColorLinear;
	float roughness = baseRoughness;

	if (currentFuzzAmount > 0. || currentColorVariation > 0.)
	{
		vec3 noisePos = query * currentFuzzScale;
		float noiseVal = fbm(noisePos, 4, 0.5);

		if (currentFuzzAmount > 0.)
		{
			float noiseX = fbm(noisePos + vec3(13.7, 0, 0), 4, 0.5);
			float noiseY = fbm(noisePos + vec3(0, 23.1, 0), 4, 0.5);
			float noiseZ = fbm(noisePos + vec3(5.1, 9.3, 1.7), 4, 0.5);
			vec3 tangentNoise = vec3(noiseX, noiseY, noiseZ) * 2. - 1.;
			if (length(tangentNoise) > 1e-5)
			{
				vec3 perturbation = normalize(
					tangentNoise - N_geom * dot(N_geom, tangentNoise));
				finalNormal =
					normalize(N_geom + perturbation * currentFuzzAmount);
			}
			float noiseMag = abs(noiseVal * 2. - 1.);
			roughness += noiseMag * currentFuzzAmount * 0.5;
		}

		if (currentColorVariation > 0.)
		{
			albedo *= (1. + (noiseVal * 2. - 1.) * currentColorVariation);
		}

		roughness = clamp(roughness, 0.05, 1.);
		albedo = clamp(albedo, 0., 1.);
	}

	return BSDF(query, finalNormal, albedo, metallic, roughness, ao,
				sssThickness, sssDistortion, sssGlow, sssScale, sssAmbient,
				sssMaxThicknessCheck);
}


#define FRINGE_LENGTH 0.15
#define FRINGE_ANGLE_VARIATION 10.
#define FRINGE_LENGTH_VARIATION 0.2
#define FRINGE_COUNT_X 180

float wrinkleFrequency1 = 9.5;
float wrinkleAmplitude1 = 0.001;
float noiseFrequency = 2.7;
float noiseAmplitude = 0.07;
float SDF_Fringes(vec3 p, vec3 boxMin, vec3 boxMax)
{
	float fringeSpacing = (u_PatternScale.x / float(u_WarpThreads) * 1.5);
	float fringeRadius = getMaterialProperties(u_WarpMaterial).radius * 0.8;

	if (p.y > boxMin.y + fringeRadius || p.x < boxMin.x - fringeSpacing ||
		p.x > boxMax.x + fringeSpacing)
	{
		return 1e6;
	}

	float totalWidth = boxMax.x - boxMin.x;
	float actualSpacing = totalWidth / float(FRINGE_COUNT_X);

	float fringeIndexContinuous = (p.x - boxMin.x) / actualSpacing;
	float nearestFringeIndex = round(fringeIndexContinuous);
	float nearestFringeCenterX =
		boxMin.x + (nearestFringeIndex + 0.5) * actualSpacing;

	float hashInput = nearestFringeIndex * 0.123;
	float lengthVar = (hash11(hashInput) - 0.5) * 2. * FRINGE_LENGTH *
					  FRINGE_LENGTH_VARIATION;
	float angleVar =
		(hash11(hashInput + 1.) - 0.5) * 2. * FRINGE_ANGLE_VARIATION;

	float currentFringeLength = FRINGE_LENGTH + lengthVar;

	vec3 fringeTopCenterOrig = vec3(nearestFringeCenterX, boxMin.y, 0.);

	float startDisplacement = sin(fringeTopCenterOrig.x * wrinkleFrequency1 +
								  fringeTopCenterOrig.y * 0.5) *
							  wrinkleAmplitude1;
	startDisplacement +=
		(fbm(fringeTopCenterOrig * noiseFrequency, 3, 0.5) * 2. - 1.) *
		noiseAmplitude;
	vec3 fringeStart =
		vec3(nearestFringeCenterX, boxMin.y - fringeRadius, startDisplacement);

	vec3 fringeEndBase = fringeStart + vec3(0., -currentFringeLength, 0.);

	vec3 dir = normalize(fringeEndBase - fringeStart);
	vec3 rotatedDir = rotateZ(dir, angleVar);
	vec3 fringeEnd = fringeStart + rotatedDir * currentFringeLength;


	float fringeDist = SDF_RoundCone(p, fringeStart, fringeEnd, fringeRadius,
									 fringeRadius * 0.8);

	return fringeDist;
}
#define SIDE_EDGE_NOISE_AMP 0.015
#define SIDE_EDGE_NOISE_FREQ 25.
#define SIDE_EDGE_FALLOFF 0.04

#define TOP_HEM_HEIGHT 0.03
#define TOP_HEM_DEPTH 0.015
#define TOP_HEM_ROUNDING 0.005

float SDF_TopHem(vec3 p, vec3 boxMin, vec3 boxMax)
{
	float hemCenterY = boxMax.y + TOP_HEM_HEIGHT * 0.4;
	float hemHalfLengthX = (boxMax.x - boxMin.x) * 0.5 + TOP_HEM_ROUNDING;

	vec3 displacementQueryPoint = vec3(p.x, boxMax.y, 0.);
	float pDisplacement = sin(displacementQueryPoint.x * wrinkleFrequency1 +
							  displacementQueryPoint.y * 0.5) *
						  wrinkleAmplitude1;
	pDisplacement +=
		(fbm(displacementQueryPoint * noiseFrequency, 3, 0.5) * 2. - 1.) *
		noiseAmplitude;

	vec3 hemCenter_Undisplaced = vec3(0., hemCenterY, 0.);
	vec3 hemBounds = vec3(hemHalfLengthX, TOP_HEM_HEIGHT * 0.5, TOP_HEM_DEPTH);

	vec3 p_mod = p;
	p_mod.z -= pDisplacement;

	vec3 q =
		abs(p_mod - hemCenter_Undisplaced) - (hemBounds - TOP_HEM_ROUNDING);
	float roundedBoxDist = length(max(q, 0.)) +
						   min(max(q.x, max(q.y, q.z)), 0.) - TOP_HEM_ROUNDING;

	return roundedBoxDist;
}