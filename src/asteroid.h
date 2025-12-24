#pragma once
#include "olcPixelGameEngine.h"

struct Asteroid {
	olc::vf2d pos, vel;
	float r = 24.0f;
	bool alive = true;

	olc::Decal* decal = nullptr;

	void Update(float dt, int screenH);
	void Draw(olc::PixelGameEngine* pge);

};