#version 330 core

uniform float u_Time;

uniform vec3 u_CamPos;
uniform vec3 u_Forward, u_Right, u_Up;
uniform vec2 u_ScreenDims;

// PBR material attributes
uniform vec3 u_Albedo;
uniform float u_Metallic;
uniform float u_Roughness;
uniform float u_AmbientOcclusion;
// Texture maps for controlling some of the attribs above, plus normal mapping
uniform sampler2D u_AlbedoMap;
uniform sampler2D u_MetallicMap;
uniform sampler2D u_RoughnessMap;
uniform sampler2D u_AOMap;
uniform sampler2D u_NormalMap;
// If true, use the textures listed above instead of the GUI slider values
uniform bool u_UseAlbedoMap;
uniform bool u_UseMetallicMap;
uniform bool u_UseRoughnessMap;
uniform bool u_UseAOMap;
uniform bool u_UseNormalMap;

// Image-based lighting
uniform samplerCube u_DiffuseIrradianceMap;
uniform samplerCube u_GlossyIrradianceMap;
uniform sampler2D u_BRDFLookupTexture;

// Loom
uniform int u_WarpThreads;
uniform int u_Shafts;
uniform int u_Treadles;
uniform int u_WeftPicks;

#define MAX_WARP_THREADS 256
#define MAX_SHAFTS 16
#define MAX_TREADLES 16
#define MAX_WEFT_PICKS 256

#define MAX_THREADING_SIZE MAX_WARP_THREADS
#define MAX_TIEUP_SIZE (MAX_SHAFTS * MAX_TREADLES)
#define MAX_TREADLING_SIZE MAX_WEFT_PICKS

uniform int u_Threading[MAX_THREADING_SIZE];
uniform int u_Tieup[MAX_TIEUP_SIZE];
uniform int u_Treadling[MAX_TREADLING_SIZE];

uniform int u_WarpMaterial;
uniform int u_WeftMaterial;
uniform vec3 u_WarpColorBase;
uniform vec3 u_WeftColorBase;
uniform vec2 u_PatternScale;

const int MAT_COTTON = 0;
const int MAT_SILK = 1;
const int MAT_WOOL_YARN = 2;
const int MAT_LINEN = 3;
const int MAT_METALLIC_GOLD = 4;
const int MAT_POLYESTER = 5;

// Varyings
in vec2 fs_UV;
out vec4 out_Col;

const float PI = 3.14159f;

struct Ray
{
	vec3 origin;
	vec3 direction;
};

struct BSDF
{
	vec3 pos;
	vec3 nor;
	vec3 albedo;
	float metallic;
	float roughness;
	float ao;

	float thickness;
	float sssDistortion;
	float sssGlow;
	float sssScale;
	float sssAmbient;
	float maxThicknessCheck;
};

#define SSS_MAX_THICKNESS 0.5
#define SSS_THICKNESS_STEPS 8
#define SSS_DEFAULT_DISTORTION 0.2
#define SSS_DEFAULT_GLOW 4.
#define SSS_DEFAULT_SCALE 1.
#define SSS_DEFAULT_AMBIENT 0.1

#define HIT_THRESHOLD 0.0001

struct MarchResult
{
	float t;
	int hitSomething;
	BSDF bsdf;
};

struct SmoothMinResult
{
	float dist;
	float material_t;
};

float dot2(in vec2 v)
{
	return dot(v, v);
}
float dot2(in vec3 v)
{
	return dot(v, v);
}
float ndot(in vec2 a, in vec2 b)
{
	return a.x * b.x - a.y * b.y;
}

float sceneSDF(vec3 query);

vec3 SDF_Normal(vec3 query)
{
	vec2 epsilon = vec2(0.0, 0.001);
	return normalize(
		vec3(sceneSDF(query + epsilon.yxx) - sceneSDF(query - epsilon.yxx),
			 sceneSDF(query + epsilon.xyx) - sceneSDF(query - epsilon.xyx),
			 sceneSDF(query + epsilon.xxy) - sceneSDF(query - epsilon.xxy)));
}

float SDF_Sphere(vec3 query, vec3 center, float radius)
{
	return length(query - center) - radius;
}

float SDF_Box(vec3 query, vec3 bounds)
{
	vec3 q = abs(query) - bounds;
	return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}

float SDF_RoundCone(vec3 query, vec3 a, vec3 b, float r1, float r2)
{
	// sampling independent computations (only depend on shape)
	vec3 ba = b - a;
	float l2 = dot(ba, ba);
	float rr = r1 - r2;
	float a2 = l2 - rr * rr;
	float il2 = 1.0 / l2;

	// sampling dependant computations
	vec3 pa = query - a;
	float y = dot(pa, ba);
	float z = y - l2;
	float x2 = dot2(pa * l2 - ba * y);
	float y2 = y * y * l2;
	float z2 = z * z * l2;

	// single square root!
	float k = sign(rr) * rr * rr * x2;
	if (sign(z) * a2 * z2 > k)
		return sqrt(x2 + z2) * il2 - r2;
	if (sign(y) * a2 * y2 < k)
		return sqrt(x2 + y2) * il2 - r1;
	return (sqrt(x2 * a2 * il2) + y * rr) * il2 - r1;
}

float sdTorus(vec3 p, vec2 t)
{
	vec2 q = vec2(length(p.xz) - t.x, p.y);
	return length(q) - t.y;
}

float sdCapsule(vec2 p, vec2 a, vec2 b, float r)
{
	vec2 pa = p - a;
	vec2 ba = b - a;
	float h = clamp(dot(pa, ba) / dot(ba, ba), 0., 1.);
	return length(pa - ba * h) - r;
}

float SDF_Circle(vec2 query, vec2 center, float radius)
{
	return length(query - center) - radius;
}

float SDF_Box2D(vec2 query, vec2 bounds)
{
	vec2 q = abs(query) - bounds;
	return length(max(q, vec2(0.))) + min(max(q.x, q.y), 0.);
}

float sdSegment(in vec2 p, in vec2 a, in vec2 b)
{
	vec2 pa = p - a, ba = b - a;
	float h = clamp(dot(pa, ba) / dot(ba, ba), 0., 1.);
	return length(pa - ba * h);
}

float sdCylinder(vec3 p, vec2 h)
{
	vec2 d = abs(vec2(length(p.xz), p.y)) - h;
	return min(max(d.x, d.y), 0.) + length(max(d, 0.));
}

vec2 rotate(vec2 p, float angle)
{
	angle = angle * PI / 180.;
	float c = cos(angle);
	float s = sin(angle);
	return vec2(c * p.x - s * p.y, s * p.x + c * p.y);
}

vec2 repeat(vec2 query, vec2 cell)
{
	return mod(query + 0.5f * cell, cell) - 0.5f * cell;
}

float smooth_min(float a, float b, float k)
{
	float h = max(k - abs(a - b), 0.0) / k;
	return min(a, b) - h * h * k * 0.25;
}

SmoothMinResult smooth_min_lerp(float a, float b, float k)
{
	float h = max(k - abs(a - b), 0.0) / k;
	float m = h * h * 0.5;
	float s = m * k * 0.5;
	if (a < b)
	{
		return SmoothMinResult(a - s, m);
	}
	return SmoothMinResult(b - s, 1.0 - m);
}
vec3 repeat(vec3 query, vec3 cell)
{
	return mod(query + 0.5 * cell, cell) - 0.5 * cell;
}

float subtract(float d1, float d2)
{
	return max(d1, -d2);
}

float opIntersection(float d1, float d2)
{
	return max(d1, d2);
}

float opOnion(float sdf, float thickness)
{
	return abs(sdf) - thickness;
}

vec3 rotateX(vec3 p, float angle)
{
	angle = angle * 3.14159 / 180.f;
	float c = cos(angle);
	float s = sin(angle);
	return vec3(p.x, c * p.y - s * p.z, s * p.y + c * p.z);
}

vec3 rotateZ(vec3 p, float angle)
{
	angle = angle * 3.14159 / 180.f;
	float c = cos(angle);
	float s = sin(angle);
	return vec3(c * p.x - s * p.y, s * p.x + c * p.y, p.z);
}

vec3 rotateY(vec3 p, float angle)
{
	angle = angle * PI / 180.f;
	float c = cos(angle);
	float s = sin(angle);
	return vec3(c * p.x + s * p.z, p.y, -s * p.x + c * p.z);
}

float hash11(float p)
{
	p = fract(p * .1031);
	p *= p + 33.33;
	p *= p + p;
	return fract(p);
}

vec2 hash21(float p)
{
	vec3 p3 = fract(vec3(p) * vec3(.1031, .1030, .0973));
	p3 += dot(p3, p3.yzx + 33.33);
	return fract((p3.xx + p3.yz) * p3.zy);
}

vec3 hash31(float p)
{
	vec3 p3 = fract(vec3(p) * vec3(.1031, .1030, .0973));
	p3 += dot(p3, p3.yzx + 33.33);
	return fract((p3.xxy + p3.yzz) * p3.zyx);
}

vec3 hash33(vec3 p3)
{
	p3 = fract(p3 * vec3(.1031, .1030, .0973));
	p3 += dot(p3, p3.yxz + 33.33);
	return fract((p3.xxy + p3.yxx) * p3.zyx);
}

float nnnoise(vec3 x)
{
	vec3 i = floor(x);
	vec3 f = fract(x);

	vec3 u = f * f * (3. - 2. * f);
	float n =
		mix(mix(mix(hash11(dot(i + vec3(0, 0, 0), vec3(1, 57, 113))),
					hash11(dot(i + vec3(1, 0, 0), vec3(1, 57, 113))), u.x),
				mix(hash11(dot(i + vec3(0, 1, 0), vec3(1, 57, 113))),
					hash11(dot(i + vec3(1, 1, 0), vec3(1, 57, 113))), u.x),
				u.y),
			mix(mix(hash11(dot(i + vec3(0, 0, 1), vec3(1, 57, 113))),
					hash11(dot(i + vec3(1, 0, 1), vec3(1, 57, 113))), u.x),
				mix(hash11(dot(i + vec3(0, 1, 1), vec3(1, 57, 113))),
					hash11(dot(i + vec3(1, 1, 1), vec3(1, 57, 113))), u.x),
				u.y),
			u.z);
	return n;
}

float fbm(vec3 x, int octaves, float persistence)
{
	float total = 0.;
	float frequency = 1.;
	float amplitude = 1.;
	float maxValue = 0.;

	for (int i = 0; i < octaves; ++i)
	{
		total += (nnnoise(x * frequency) - 0.5f) * amplitude;
		maxValue += amplitude;
		amplitude *= persistence;
		frequency *= 2.;
	}

	return total / maxValue;
}

float worleyNoise(vec3 p)
{
	vec3 i = floor(p);
	vec3 f = fract(p);
	float min_dist = 1.;

	for (int x = -1; x <= 1; ++x)
	{
		for (int y = -1; y <= 1; ++y)
		{
			for (int z = -1; z <= 1; ++z)
			{
				vec3 neighbor = vec3(float(x), float(y), float(z));
				vec3 point_pos = hash33(i + neighbor);
				vec3 diff = neighbor + point_pos - f;
				min_dist = min(min_dist, dot(diff, diff));
			}
		}
	}
	return sqrt(min_dist);
}

vec3 domainWarp(vec3 p, float strength, float scale)
{
	float noiseX = fbm(p * scale + vec3(12.3, 45.6, 78.9), 4, 0.5);
	float noiseY = fbm(p * scale + vec3(98.7, 65.4, 32.1), 4, 0.5);
	float noiseZ = fbm(p * scale + vec3(55.5, 22.2, 88.8), 4, 0.5);

	return vec3(noiseX, noiseY, noiseZ) * strength;
}

vec3 cartesianToSpherical(vec3 p)
{
	float r = length(p);
	float theta = acos(p.y / r);
	float phi = atan(p.z, p.x);
	if (phi < 0.)
		phi += 2. * PI;
	return vec3(r, theta, phi);
}

vec2 sphereUV(vec3 p)
{
	p = normalize(p);
	float u = 0.5 + atan(p.z, p.x) / (2. * PI);
	float v = 0.5 - asin(p.y) / PI;
	return vec2(u, v);
}

float calculateSdfThickness(vec3 P, vec3 N, float max_dist_check)
{
	float traveled_dist = 0.;
	vec3 startP = P - N * 0.0001 * 2.;
	float measured_thickness = max_dist_check;

	for (int i = 0; i < SSS_THICKNESS_STEPS; ++i)
	{
		vec3 currentP = startP - N * traveled_dist;
		float dist_to_surface = sceneSDF(currentP);

		if (dist_to_surface > -0.0001 * 2.)
		{
			measured_thickness = traveled_dist;
			break;
		}

		float step_dist =
			max(abs(dist_to_surface),
				max_dist_check / float(SSS_THICKNESS_STEPS) * 0.5);
		traveled_dist += step_dist;

		if (traveled_dist >= max_dist_check)
		{
			measured_thickness = max_dist_check;
			break;
		}
	}

	if (isnan(measured_thickness) || isinf(measured_thickness))
		return 0.;

	return 1. - smoothstep(0., max_dist_check, measured_thickness);
}

BSDF plasticSSS(vec3 query)
{
	BSDF result =
		BSDF(query, SDF_Normal(query), vec3(1., 1., 1.), 0., 0.2f, 1., 0.,
			 SSS_DEFAULT_DISTORTION, SSS_DEFAULT_GLOW, SSS_DEFAULT_SCALE,
			 SSS_DEFAULT_AMBIENT, SSS_MAX_THICKNESS);

	result.pos = query;
	result.nor = SDF_Normal(query);
	result.albedo = vec3(1., 1., 1.);
	result.metallic = 0.;
	result.roughness = 0.2f;
	result.ao = 1.;
	if (result.metallic < 0.1f)
	{
		result.maxThicknessCheck = 0.085;
		result.thickness =
			calculateSdfThickness(query, result.nor, result.maxThicknessCheck);
		/*
		result.thickness = 2.;
		result.sssDistortion = 2.;
		result.sssGlow = 6.;
		result.sssScale = 3.;
		*/
	}
	else
	{
		result.thickness = 0.;
	}
	return result;
}
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


#define FOVY 45 * PI / 180.f
Ray rayCast()
{
	vec2 ndc = fs_UV;
	ndc = ndc * 2.f - vec2(1.f);

	float aspect = u_ScreenDims.x / u_ScreenDims.y;
	vec3 ref = u_CamPos + u_Forward;
	vec3 V = u_Up * tan(FOVY * 0.5);
	vec3 H = u_Right * tan(FOVY * 0.5) * aspect;
	vec3 p = ref + H * ndc.x + V * ndc.y;

	return Ray(u_CamPos, normalize(p - u_CamPos));
}

#define MAX_ITERATIONS 128
#define MIN_STEP (HIT_THRESHOLD * 0.5)
#define MAX_DIST 100.

MarchResult raymarch(Ray ray)
{
	float t = 0.001;
	int hit = 0;
	BSDF hitBSDF =
		BSDF(vec3(0.), vec3(0., 1., 0.), vec3(0.5), 0., 0.5, 1., 0.,
			 SSS_DEFAULT_DISTORTION, SSS_DEFAULT_GLOW, SSS_DEFAULT_SCALE,
			 SSS_DEFAULT_AMBIENT, SSS_MAX_THICKNESS);
	float hit_t = MAX_DIST;

	float initialDist = sceneSDF(ray.origin);
	if (initialDist < HIT_THRESHOLD)
	{
		hit = 1;
		hit_t = 0.;
		vec3 hitNormal = SDF_Normal(ray.origin);
		hitBSDF = sceneBSDF(ray.origin, hitNormal);
		hitBSDF.pos = ray.origin;
		return MarchResult(hit_t, hit, hitBSDF);
	}

	for (int i = 0; i < MAX_ITERATIONS; ++i)
	{
		vec3 p = ray.origin + ray.direction * t;
		float dist = sceneSDF(p);

		if (dist < HIT_THRESHOLD)
		{
			hit = 1;
			hit_t = t;

			vec3 hitNormal = SDF_Normal(p);

			float normalOffset = max(HIT_THRESHOLD * 0.5, dist * 0.1);
			vec3 safe_p = p + hitNormal * normalOffset;

			hitBSDF = sceneBSDF(safe_p, hitNormal);
			hitBSDF.pos = safe_p;

			break;
		}

		if (t > MAX_DIST)
		{
			hit_t = MAX_DIST;
			break;
		}

		t += max(MIN_STEP, dist * 0.8);
	}

	return MarchResult(hit_t, hit, hitBSDF);
}


void main()
{
	Ray ray = rayCast();
	MarchResult result = raymarch(ray);
	if (result.hitSomething <= 0 || result.t >= MAX_DIST)
	{
		out_Col = vec4(0.);
	}
	else
	{
		BSDF bsdf = result.bsdf;

		vec3 color = metallic_plastic_LTE(bsdf, -ray.direction);
		color = color / (color + vec3(1.));
		color = pow(color, vec3(1. / 2.2));

		out_Col = vec4(color, result.hitSomething > 0 ? 1. : 0.);
	}
}
 
