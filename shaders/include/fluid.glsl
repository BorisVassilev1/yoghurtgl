struct Particle {
	vec3	position;
	float	padding;
	vec3	velocity;
	float	padding2;
};

uniform uint N;
uniform float cellSize = 1.0f;

const float EPS = 0.01f;

uint spatialHash(in ivec3 cell) {
	uint hash = cell.x * 15823 ^ cell.y * 9737333 ^ cell.z * 71993;
	return hash % N;
}

ivec3 cellFromPosition(in vec3 position) {
	return ivec3(position / cellSize);
}

uint spatialHash(in vec3 position) {
	ivec3 cell = cellFromPosition(position);
	return spatialHash(cell);
}

