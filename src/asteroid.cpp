#include "asteroid.h"

void Asteroid::Update(float dt, int screenH) {
	pos += vel * dt;
	if (pos.y - r > screenH + 14.0f)
		alive = false;

}

void Asteroid::Draw(olc::PixelGameEngine* pge) {
    if (!alive) return;

    if (decal) {
        // Make sprite height = 2 * r (so visual size matches collision)
        float desiredDiameter = r * 2.0f;  // 28 if r = 14

        float sw = float(decal->sprite->width);
        float sh = float(decal->sprite->height);

        // uniform scale so HEIGHT becomes 2r
        float scale = desiredDiameter / sh;
        olc::vf2d vScale = { scale, scale };

        // scaled size so we can center on pos
        olc::vf2d scaledSize = { sw * scale, sh * scale };
        olc::vf2d drawPos = pos - scaledSize * 0.5f;

        pge->DrawDecal(drawPos, decal, vScale);
    }
    else {
        pge->FillCircle(int(pos.x), int(pos.y), int(r), olc::GREY);
    }
}

