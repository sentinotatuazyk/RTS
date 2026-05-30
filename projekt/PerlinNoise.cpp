#include "PerlinNoise.h"
#include <random>

PerlinNoise::PerlinNoise(unsigned int seed) {
	std::array<int, 256> permutation;
	for (int i = 0; i < 256; i++) {
		permutation[i] = i;
	}

	std::mt19937 gen(seed);
	std::shuffle(permutation.begin(), permutation.end(), gen);
	for (int i = 0; i < 512; i++) {
		p[i] = permutation[i%256];
	}
}

float PerlinNoise::fade(float t) {
	return t * t * t * (t * (t * 6 - 15) + 10);
}

float PerlinNoise::lerp(float a, float b, float t) {
	return a + t * (b - a);
}

float PerlinNoise::grad(int hash, float x, float y) {
	int h = hash & 3; // 4 possible gradients
	float u = h < 2 ? x : y;
	float v = h < 2 ? y : x;
	return ((h & 1) ? -u : u) + ((h & 2) ?  2.f* -v : 2.f * v);
}

float PerlinNoise::noise(float x, float y) const {
	int X = static_cast<int>(std::floor(x)) & 255;
	int Y = static_cast<int>(std::floor(y)) & 255;
	x -= std::floor(x);
	y -= std::floor(y);
	float u = fade(x);
	float v = fade(y);
	int aa = p[p[X] + Y];
	int ab = p[p[X] + Y + 1];
	int ba = p[p[X + 1] + Y];
	int bb = p[p[X + 1] + Y + 1];
	float res = lerp(
		lerp(grad(aa, x, y), grad(ba, x - 1, y), u),
		lerp(grad(ab, x, y - 1), grad(bb, x - 1, y - 1), u),
		v
	);
	return (res + 1.0f) / 2.0f; // Normalize to [0,1]
}

float PerlinNoise::fbm(float x, float y, int octaves, float persistence) const {
	float total = 0.0f;
	float frequency = 1.0f;
	float amplitude = 1.0f;
	float maxValue = 0.0f; // Used for normalizing result to [0,1]
	for (int i = 0; i < octaves; i++) {
		total += noise(x * frequency, y * frequency) * amplitude;
		maxValue += amplitude;
		amplitude *= persistence;
		frequency *= 2.0f;
	}
	return total / maxValue; // Normalize to [0,1]
}