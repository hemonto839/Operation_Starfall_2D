#pragma once
#include "olcPixelGameEngine.h"

struct Player {
	olc::vf2d pos;
	float speed = 180.0f;
	float r = 30.0f;
	int lives = 3;

	float invincibleTimer = 0.0f; // for flicker

	olc::Decal* decal = nullptr;

	void Reset(const olc::vf2d& startPos);
	void Update(olc::PixelGameEngine* pge, float dt);
	void Draw(olc::PixelGameEngine* pge);
};