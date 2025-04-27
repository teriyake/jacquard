
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
