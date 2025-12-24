#pragma once
#include "olcPixelGameEngine.h"
#include <cmath>

struct Boss {
	olc::vf2d pos;
	olc::vf2d vel;
	float r = 60.0f; // Bigger collision radius
	int maxHp = 200;
	int hp = 200;
	bool alive = false;
	bool inArena = false;
	float targetY = 100.0f; // Where the boss stops moving down
	olc::Decal* decal = nullptr;  // Added for sprite rendering

	void Reset(const olc::vf2d& startPos) {
		pos = startPos;
		hp = maxHp;
		alive = true;
		vel = { 0.0f, 70.0f }; // Move down
		inArena = false;
	}

	void Update(float dt, int screenW) {
		if (!alive) return;

		if (!inArena) {
			// Phase 1: Enter from top
			pos += vel * dt;
			if (pos.y >= targetY) {
				pos.y = targetY;
				inArena = true;
				vel = { 60.0f, 0.0f }; // Switch to horizontal movement
			}
		}
		else {
			// Phase 2: Move left-right
			pos += vel * dt;

			// Bounce on left wall 
			if (pos.x - r < 0.0f) {
				pos.x = r;
				vel.x *= -1.0f;
			}
			// Bounce on right wall
			if (pos.x + r > screenW) {
				pos.x = screenW - r;
				vel.x *= -1.0f;
			}
		}
	}

	void Draw(olc::PixelGameEngine* pge) {
		if (!alive) return;

		if (decal) {
			// Make sprite height = 2 * r
			float desiredDiameter = r * 2.0f;
			float sw = float(decal->sprite->width);
			float sh = float(decal->sprite->height);
			float scale = desiredDiameter / sh;

			olc::vf2d vScale = { scale, scale };
			olc::vf2d scaledSize = { sw * scale, sh * scale };
			olc::vf2d drawPos = pos - scaledSize * 0.5f;

			pge->DrawDecal(drawPos, decal, vScale);
		}
		else {
			// Fallback geometric boss
			int x = (int)pos.x;
			int y = (int)pos.y;

			// Body
			pge->FillRect(x - 40, y - 12, 80, 24, olc::DARK_RED);
			// Core
			pge->FillCircle(x, y, 10, olc::YELLOW);
			// Wings
			pge->FillRect(x - 50, y - 4, 10, 8, olc::RED);
			pge->FillRect(x + 40, y - 4, 10, 8, olc::RED);
		}
	}
};