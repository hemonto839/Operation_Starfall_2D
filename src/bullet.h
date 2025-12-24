#pragma once
#include "olcPixelGameEngine.h"

struct Bullet {
	olc::vf2d pos;
	olc::vf2d vel;
	float r = 6.0f;
	bool alive = true;
	olc::Decal* decal = nullptr;

	void Update(float dt) {
		pos += vel * dt;
		if (pos.y < -10.0f) alive = false;
	}

	void Draw(olc::PixelGameEngine* pge) {
		if (!alive) return;

		if (decal) {
			// Make bullet sprite sized to 4*r
			float desiredSize = r * 4.0f;
			float sw = float(decal->sprite->width);
			float sh = float(decal->sprite->height);
			float scale = desiredSize / std::max(sw, sh);

			olc::vf2d vScale = { scale, scale };
			olc::vf2d scaledSize = { sw * scale, sh * scale };
			olc::vf2d drawPos = pos - scaledSize * 0.5f;

			pge->DrawDecal(drawPos, decal, vScale);
		}
		else {
			// Fallback circle
			pge->FillCircle(pos, r, olc::YELLOW);
		}
	}
};