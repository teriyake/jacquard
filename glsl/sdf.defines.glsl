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
