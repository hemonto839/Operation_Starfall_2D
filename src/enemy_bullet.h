#pragma once
#include "olcPixelGameEngine.h"

struct EnemyBullet {
	olc::vf2d pos;
	olc::vf2d vel;
	float r = 6.0f;
	bool alive = true;
	olc::Decal* decal = nullptr;  // Added for sprite rendering

	void Update(float dt, int screenH) {
		pos += vel * dt;
		if (pos.y - r > screenH + 10) {
			alive = false;
		}
	}

	void Draw(olc::PixelGameEngine* pge) const {
		if (!alive) return;

		if (decal) {
			// Make bullet sprite sized to 2*r
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
			pge->FillCircle(pos, int(r), olc::RED);
		}
	}
};