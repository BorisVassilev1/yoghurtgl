#ifdef GL_ES
precision highp float;
#endif

#define FLOAT_MAX 3.4028235e+38

struct Material {
	vec3 albedo;
	float	  specular_chance;
	// 16
	
	vec3 emission;
	float	  ior;
	// 32
	
	vec3 transparency_color;
	float	  refraction_chance;
	// 48
	
	float refraction_roughness;
	float specular_roughness;
	// 56 

	int texture_sampler; // useless but here for alignment
	float texture_influence;
}; // 64 bytes all


struct Sphere {
	vec3 position;
	float radius;
	Material mat;
};

struct Ray {
    vec3 origin;
    vec3 direction;
    vec3 attenuation;
};

struct RayHit {
    vec3 pos;
    float dist;
    vec3 normal;
    Material mat;
    bool is_front_face;
};

struct Triangle {
	vec3 v0;
	vec3 v1;
	vec3 v2;
	vec3 normal0;
	vec3 normal1;
	vec3 normal2;
};

struct AABB {
	vec3 min;
	vec3 max;
	vec3 center;
};

struct BVHNode{
	AABB box;
	int obj_index;
};

layout(std430, binding = 2) buffer Vertices {
	float vertices[];
};

layout(std430, binding = 3) buffer Normals {
	float normals[];
};

layout(std430, binding = 4) buffer Indices {
	int indices[];
};

layout(std430, binding = 5) buffer BVH {
	BVHNode nodes[];
}; 

layout(binding = 5) uniform samplerCube skybox;

uniform mat4 cameraMatrix;
uniform vec2 resolution;
uniform float fov = radians(70);
uniform int samples_per_pixel = 1;

uniform uint random_seed;

uniform mat4 bvh_matrix;

float phi = 0.001;

float PI = 3.14159;
float aspect = resolution.y / resolution.x;

int to1d(ivec2 position, ivec2 maximum) {
	return position.x + maximum.x * position.y;
}

int to1d(ivec3 position, ivec3 maximum) {
	return (position.x + maximum.x * position.y) + maximum.y * maximum.x * position.z;
}

float angleBetween(vec3 v1, vec3 v2) {
    return acos(dot(normalize(v1), normalize(v2))); /// 3.14159;
}

uint PCGHash(inout uint seed)
{
    seed = seed * 747796405u + 2891336453u;
    uint word = ((seed >> ((seed >> 28u) + 4u)) ^ seed) * 277803737u;
    return (word >> 22u) ^ word;
}

float randomFloat(inout uint seed)
{
    return float(PCGHash(seed)) / 4294967296.0;
}

vec3 random_in_unit_sphere(inout uint seed) {
	float r1 = randomFloat(seed);
	float r2 = randomFloat(seed);
	float x = cos(2*PI*r1)*2*sqrt(r2*(1-r2));
	float y = sin(2*PI*r1)*2*sqrt(r2*(1-r2));
	float z = 1 - 2*r2 + 0.0000001;
    return vec3(x,y,z);
}

vec3 cosWeightedHemissphereDir(vec3 normal, inout uint seed)
{
    float z = randomFloat(seed) * 2.0 - 1.0;
    float a = randomFloat(seed) * 2.0 * PI;
    float r = sqrt(1.0 - z * z);
    float x = r * cos(a);
    float y = r * sin(a);

    // Convert unit vector in sphere to a cosine weighted vector in hemissphere
    return normalize(normal + vec3(x, y, z));
}

vec3 random_in_unit_disk(inout uint seed) {
	float r = randomFloat(seed);
	float theta = randomFloat(seed) * 2 * PI;
	float x = sqrt(r) * cos(theta);
	float y = sqrt(r) * sin(theta);
	return vec3(x,y,0.0);
}

float getSmallestPositive(float t1, float t2)
{
    // Assumes at least one float > 0
    //return t1 < 0 ? t2 : t1;
    return mix(t1, t2, t1 < 0);
}

void intersectSphere(in Ray r, in Sphere sphere, inout RayHit rec) {
	// source: https://antongerdelan.net/opengl/raycasting.html
	float t1 = FLOAT_MAX, t2 = FLOAT_MAX;
	vec3 sphereToRay = r.origin - sphere.position.xyz;
	float b = dot(r.direction, sphereToRay);
	float c = dot(sphereToRay, sphereToRay) - sphere.radius * sphere.radius;
	float disc = b * b - c;
	
	if(disc < 0) return;
	
	float squareRoot = sqrt(disc);
	t1 = -b - squareRoot;
	t2 = -b + squareRoot;
	
	if(t2 > 0.0001 && t1 < rec.dist) {
		rec.dist = getSmallestPositive(t1, t2);
		rec.pos = r.origin + rec.dist * r.direction;
    	rec.normal = (rec.pos - sphere.position.xyz) / sphere.radius;
		rec.mat = sphere.mat;
		rec.is_front_face = abs(rec.dist - t2) > 0.001;
		rec.normal = (float(rec.is_front_face) * 2. - 1.) * rec.normal;
	}
}



void intersectGroundPlane(in Ray ray, in float PlaneHeight, inout RayHit hit) {
	float t = (ray.origin.y - PlaneHeight) / -ray.direction.y;
	if(t > 0.001 && t < hit.dist) {
		vec3 pos = t * ray.direction + ray.origin;
		if(pos.x < 10 && pos.x > -10 && pos.z < 10 && pos.z > -10) {

		hit.dist = t;
		hit.normal = vec3(0.0, 1.0, 0.0);
		hit.mat = Material(vec3(1.0), .5, vec3(0.), 0., vec3(0.), 0., 0., 1.0, 0, 0.);
		float d = dot(ray.direction, hit.normal); 
		hit.is_front_face = d < 0.0;
		hit.normal = -(step(0.0, d) * 2.0 - 1.0) * hit.normal;
		}
	}
}

float intersect(in vec3 orig, in vec3 dir, in vec3 v0, in vec3 v1, in vec3 v2, out vec2 UV) { // graphicon.org/html/2012/conference/EN2%20-%20Graphics/gc2012Shumskiy.pdf
	vec3 e1 = v1 - v0;
	vec3 e2 = v2 - v0;
	vec3 normal = normalize(cross(e1, e2));
	float b = dot(normal, dir);
	vec3 w0 = orig - v0;
	float a = -dot(normal, w0);
	float t = a / b;
	vec3 p = orig + t * dir;
	float uu, uv, vv, wu, wv, inverseD;
	uu = dot(e1, e1);
	uv = dot(e1, e2);
	vv = dot(e2, e2);
	vec3 w = p - v0;
	wu = dot(w, e1);
	wv = dot(w, e2);
	inverseD = uv * uv - uu * vv;
	inverseD = 1.0f / inverseD;
	float u = (uv * wv - vv * wu) * inverseD;
	if (u < 0.0 || u > 1.0)
		return -1.0f;
	float v = (uv * wu - uu * wv) * inverseD;
	if (v < 0.0 || (u + v) > 1.0)
		return -1.0f;
	
	UV = vec2(u,v);
	return t;
}

uniform Material geometry_material = Material(vec3(1.), .2, vec3(0.), 0.99, vec3(0.1), 0.0, 0.0, 0.1, 0, 0.);

bool intersectTriangle(in Ray ray, in vec3 v0, in vec3 v1, in vec3 v2, in vec3 normal0, in vec3 normal1, in vec3 normal2, inout RayHit hit) {
	vec3 orig = ray.origin;
	vec3 dir = ray.direction;
	vec2 uv;
	
	float t = intersect(orig, dir, v0, v1, v2, uv);
	
	if(t > 0.0001 && t < hit.dist) {
		hit.dist = t;
		
		hit.mat = geometry_material;
		
		hit.normal = normalize((1. - uv.x - uv.y) * normal0 + uv.x * normal1 + uv.y * normal2);
		
		float d = dot(ray.direction, hit.normal); 
		hit.is_front_face = d < 0.0;
		hit.normal = -(step(0.0, d) * 2.0 - 1.0) * hit.normal;
		return true;
	}
	return false;
}

bool intersectAABB(in Ray ray, in AABB box) {
	// Source: https://medium.com/@bromanz/another-view-on-the-classic-ray-aabb-intersection-algorithm-for-bvh-traversal-41125138b525
    float t1 = -FLOAT_MAX;
    float t2 = FLOAT_MAX;

    vec3 t0s = (box.min - ray.origin) * (1.0 / ray.direction);
    vec3 t1s = (box.max - ray.origin) * (1.0 / ray.direction);

    vec3 tsmaller = min(t0s, t1s);
    vec3 tbigger = max(t0s, t1s);

    t1 = max(t1, max(tsmaller.x, max(tsmaller.y, tsmaller.z)));
    t2 = min(t2, min(tbigger.x, min(tbigger.y, tbigger.z)));
    return t1 <= t2;
}

Triangle get_triangle(in int index) {
	
	int i = index * 3;
	
	int i0 = indices[i    ] * 3;
	int i1 = indices[i + 1] * 3;
	int i2 = indices[i + 2] * 3;
	
	
	vec3 v0 = vec3(vertices[i0    ],
				   vertices[i0 + 1],
				   vertices[i0 + 2]);
	vec3 v1 = vec3(vertices[i1    ],
				   vertices[i1 + 1],
				   vertices[i1 + 2]);
	vec3 v2 = vec3(vertices[i2    ],
				   vertices[i2 + 1],
				   vertices[i2 + 2]);
	
	v0 = (bvh_matrix * vec4(v0, 1.0)).xyz;
	v1 = (bvh_matrix * vec4(v1, 1.0)).xyz;
	v2 = (bvh_matrix * vec4(v2, 1.0)).xyz;
	
	vec3 normal0 = vec3(normals[i0    ],
					   	normals[i0 + 1],
					   	normals[i0 + 2]);
	vec3 normal1 = vec3(normals[i1    ],
					   	normals[i1 + 1],
					   	normals[i1 + 2]);
	vec3 normal2 = vec3(normals[i2    ],
					   	normals[i2 + 1],
					   	normals[i2 + 2]);
					   	
	normal0 = (bvh_matrix * vec4(normal0, 0.0)).xyz;
	normal1 = (bvh_matrix * vec4(normal1, 0.0)).xyz;
	normal2 = (bvh_matrix * vec4(normal2, 0.0)).xyz;
	
	return Triangle(v0, v1, v2, normal0, normal1, normal2);
}

bool intersectTriangle(in Ray ray, in Triangle t, inout RayHit hit) {
	return intersectTriangle(ray, t.v0, t.v1, t.v2, t.normal0, t.normal1, t.normal2, hit);
}

float schlick_reflectance(in vec3 incident, in vec3 normal, in float eta) {
	float r0 = (1 - eta) / (1 + eta);
	r0 *= r0;
	float cos_angle = dot(-incident, normal);
	return r0 + (1-r0)*pow((1 - cos_angle),5);
}

Ray disperse_reflect(in Ray in_ray, in RayHit hit, in vec3 reflectivity, in float fuzz, inout uint random_seed) {
	vec3 dir = normalize(mix(reflect(in_ray.direction, hit.normal), cosWeightedHemissphereDir(hit.normal, random_seed), fuzz));
	Ray out_ray = Ray(hit.pos + phi * hit.normal, dir, reflectivity * in_ray.attenuation);
	return out_ray;
}

Ray disperse_refract(in Ray in_ray, in RayHit hit, in float eta, inout uint random_seed) {
	vec3 dir = normalize(mix(refract(in_ray.direction, hit.normal, eta), cosWeightedHemissphereDir(hit.normal, random_seed), hit.mat.refraction_roughness));
	return Ray(hit.pos - phi * hit.normal, dir, in_ray.attenuation);
}

vec3 get_sky_color(in Ray ray) {
	// float t = 0.5*(ray.direction.y + 1.0);
    // return mix(vec3(1.0), vec3(1.0, 1.0, 1.0), t) * 1.0;
    
    // return vec3(0.0);
    // return vec3(1.0);
    return textureLod(skybox, ray.direction, 3.).xyz;
    
    // vec3 sun_direction = normalize(vec3(1.,2.,1.));
    // float angle = dot(sun_direction, ray.direction);
    //vec3 sun_color = vec3(1000.);
    // return mix(sun_color, vec3(.1), float(angle <= .9999));
}
