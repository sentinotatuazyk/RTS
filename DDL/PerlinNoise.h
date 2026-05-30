#pragma once

#include <cmath>
#include <array>

class PerlinNoise {
public:
	PerlinNoise(unsigned int seed = 0);

	float noise(float x, float y) const;

	float fbm(float x, float y, int octaves, float persistence = 0.5f) const;

private:
	std::array<int, 512> p;
	static float fade(float t);
	static float lerp(float a, float b, float t);
	static float grad(int hash, float x, float y);
};