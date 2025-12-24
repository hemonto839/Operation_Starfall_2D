#pragma once
#include "olcPixelGameEngine.h"
#include <random>

struct Enemy {
	olc::vf2d pos;
	olc::vf2d vel;
	float r = 30.0f;  // Bigger size for visibility
	bool alive = true;
	bool inArena = false;
	olc::Decal* decal = nullptr;  // Added for sprite rendering

	void Update(float dt, int screenW, int screenH) {
		if (!alive) return;

		float midY = screenH / 2.0f;

		if (!inArena) {
			// Coming down from top
			pos += vel * dt;
			if (pos.y >= midY) {
				pos.y = midY;
				inArena = true;

				// Random roaming inside top half
				olc::vf2d newVel = { 80.0f, 40.0f }; // default
				// Small randomization using position as a fake "seed"
				if (int(pos.x) % 2 == 0) newVel.x *= -1.0f;
				if (int(pos.y) % 2 == 0) newVel.y *= -1.0f;
				vel = newVel;
			}
		}
		else {
			// Roaming in the first half 
			pos += vel * dt;

			// Left-right wall bounce
			if (pos.x - r < 0.0f) {
				pos.x = r;
				vel.x *= -1.0f;
			}
			if (pos.x + r > screenW) {
				pos.x = screenW - r;
				vel.x *= -1.0f;
			}

			// Top-bottom bounce
			if (pos.y - r < 0.0f) {
				pos.y = r;
				vel.y *= -1.0f;
			}
			if (pos.y + r > midY) {
				pos.y = midY - r;
				vel.y *= -1.0f;
			}
		}

		// Safety: kill enemy if it goes out of bounds
		if (pos.y - r > screenH + 80.0f) {
			alive = false;
		}
	}

	void Draw(olc::PixelGameEngine* pge) {
		if (!alive) return;

		if (decal) {
			// Make sprite height = 2 * r for consistent sizing
			float desiredDiameter = r * 2.8f;
			float sw = float(decal->sprite->width);
			float sh = float(decal->sprite->height);
			float scale = desiredDiameter / sh;

			olc::vf2d vScale = { scale, scale };
			olc::vf2d scaledSize = { sw * scale, sh * scale };
			olc::vf2d drawPos = pos - scaledSize * 0.5f;

			pge->DrawDecal(drawPos, decal, vScale);
		}
		else {
			// Fallback triangle if no sprite loaded
			auto p = pos;
			olc::vf2d e1{ p.x - r, p.y - r };
			olc::vf2d e2{ p.x + r, p.y - r };
			olc::vf2d e3{ p.x,     p.y + r };
			pge->FillTriangle(e1, e2, e3, olc::RED);
		}
	}
};