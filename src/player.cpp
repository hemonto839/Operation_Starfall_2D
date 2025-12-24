#include "player.h"
#include <algorithm> // clamp
#include <cmath>

void Player::Reset(const olc::vf2d& startPos) {
    pos = startPos;
    lives = 3;
    invincibleTimer = 0.0f;
}

void Player::Update(olc::PixelGameEngine* pge, float dt) {
    
    if (invincibleTimer > 0.0f)
        invincibleTimer -= dt;
 
    olc::vf2d dir = { 0.0f, 0.0f };

    if (pge->GetKey(olc::Key::LEFT).bHeld || pge->GetKey(olc::Key::A).bHeld) dir.x -= 1.0f;
    if (pge->GetKey(olc::Key::RIGHT).bHeld || pge->GetKey(olc::Key::D).bHeld) dir.x += 1.0f;
    if (pge->GetKey(olc::Key::UP).bHeld || pge->GetKey(olc::Key::W).bHeld) dir.y -= 1.0f;
    if (pge->GetKey(olc::Key::DOWN).bHeld || pge->GetKey(olc::Key::S).bHeld) dir.y += 1.0f;


    if (dir.mag2() > 0) dir = dir.norm();     // normalize to avoid faster diagonal movement
    pos += dir * speed * dt;                 // FPS independent movement

    // keep player inside screen
    pos.x = std::clamp(pos.x, r, float(pge->ScreenWidth()) - r);
    pos.y = std::clamp(pos.y, r, float(pge->ScreenHeight()) - r);
}

//void Player::Draw(olc::PixelGameEngine* pge) {
//    auto p = pos;
//
//    olc::vf2d v1{ p.x,       p.y - r };
//    olc::vf2d v2{ p.x - r,   p.y + r };
//    olc::vf2d v3{ p.x + r,   p.y + r };
//
//    pge->FillTriangle(v1, v2, v3, olc::CYAN);
//}

void Player::Draw(olc::PixelGameEngine* pge) {
    // flicker while invincible
    if (invincibleTimer > 0.0f) {
        float t = invincibleTimer * 10.0f;
        if (fmodf(t, 2.0f) >= 1.0f)
            return;
    }

    if (decal) {
        // we want sprite height = 2 * r (r = 14 → 28 px tall)
        float desiredDiameter = r * 2.0f;   // 28 if r = 14

        float sw = float(decal->sprite->width);
        float sh = float(decal->sprite->height);

        // uniform scale so HEIGHT becomes 2r
        float scale = desiredDiameter / sh;
        olc::vf2d vScale = { scale, scale };

        // compute scaled size so we can center on pos
        olc::vf2d scaledSize = { sw * scale, sh * scale };
        olc::vf2d drawPos = pos - scaledSize * 0.5f;

        pge->DrawDecal(drawPos, decal, vScale);
    }
    else {
        // fallback triangle ship
        auto p = pos;

        olc::vf2d v1{ p.x,       p.y - r };
        olc::vf2d v2{ p.x - r,   p.y + r };
        olc::vf2d v3{ p.x + r,   p.y + r };

        pge->FillTriangle(v1, v2, v3, olc::CYAN);
    }
}
