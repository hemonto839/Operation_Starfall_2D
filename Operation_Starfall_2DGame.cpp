#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define OLC_PGEX_SOUND
#include "olcPGEX_Sound.h"

#include "src/player.h"
#include "src/asteroid.h"
#include "src/bullet.h"
#include "src/enemy.h"
#include "src/enemy_bullet.h"
#include "src/enemy_boss.h"

#include <vector>
#include <random>
#include <algorithm>
#include <string>
#include <cmath> 

float Dist2(const olc::vf2d& a, const olc::vf2d& b) {
    olc::vf2d d = a - b;
    return d.x * d.x + d.y * d.y;
}

enum class GameState {
    MENU,
    STORY,
    LEVEL_INTRO,
    LEVEL_PLAY,
    PAUSED,
    GAME_OVER
};

// ---  Explosion Struct and Vector ---
struct Explosion {
    olc::vf2d pos;
    olc::Decal* decal;
    float timer = 0.0f;
    float maxTime = 0.25f; // Explosion visible for 0.25 seconds
    float scale = 1.0f;    // To adjust size if needed
};

// --- Story Image Structure ---
struct StorySlide {
    olc::Decal* image;
    std::string text;
};

// ---- Cinematic story motion ----
float storyT = 0.0f;
int lastStoryIndex = -1;

float panX = 30.0f;
float panY = 18.0f;

float panSpeedX = 0.08f;  // slower
float panSpeedY = 0.05f;

float zoomAmp = 0.03f;
float zoomSpeed = 0.04f;

float storyOverscale = 1.15f; // start with 1.15 not 1.25



class SpaceShooter : public olc::PixelGameEngine
{
public:
    SpaceShooter()
    {
        sAppName = "Operation Starfall";
    }

    // --- Story Image Structure ---
    std::vector<StorySlide> storyIntro;
    std::vector<StorySlide> storyLevel2;
    std::vector<StorySlide> storyLevel3;
    std::vector<StorySlide> storyWin;
    std::vector<StorySlide> storyLose;

    // --- Sprites & decals ---
    olc::Sprite* sprBackground = nullptr;
    olc::Decal* decBackground = nullptr;

    olc::Sprite* sprPlayer = nullptr;
    olc::Decal* decPlayer = nullptr;

    olc::Sprite* sprAsteroid = nullptr;
    olc::Decal* decAsteroid = nullptr;

    olc::Sprite* sprEnemy = nullptr;
    olc::Decal* decEnemy = nullptr;

    olc::Sprite* sprBoss = nullptr;
    olc::Decal* decBoss = nullptr;

    olc::Sprite* sprBullet = nullptr;
    olc::Decal* decBullet = nullptr;

    olc::Sprite* sprBoomAsteroid = nullptr;
    olc::Decal* decBoomAsteroid = nullptr;

    olc::Sprite* sprBoomShip = nullptr;
    olc::Decal* decBoomShip = nullptr;

    // Background scroll
    float bgOffset = 0.0f;

    // --- Main Core Parts Objects ---
    Player player;
    std::vector<Asteroid> asteroids;
    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;
    Boss boss;
    std::vector<EnemyBullet> enemyBullets;
    std::vector<Explosion> explosions;

    // Random
    std::mt19937 rng{ std::random_device{}() };

    // Timer & Level
    GameState state = GameState::MENU;
    int currentLevel = 0;

    float introTimer = 0.0f;
    float levelTime = 0.0f;
    const float level1Duration = 25.0f;

    // Spawning timers
    float spawnTimer = 0.0f;
    float spawnRate = 0.1f;
    float fireCoolDown = 0.30f;
    float fireTimer = 0.0f;
    float enemySpawnTimer = 0.0f;
    float enemySpawnRate = 2.0f;
    int maxEnemies = 5;
    float enemyFireTimer = 0.0f;
    float enemyFireCooldown = 1.5f;
    float bossFireTimer = 0.0f;
    float bossFireCooldown = 1.0f;

    // Stats
    int score = 0;
    int hits = 0;
    bool wins = false;
    int enemiesKilled = 0;
    int level2KillTarget = 25;
    int total_enemy_spawn = 0;

    // transition timer (delay)
    float transitionTimer = 0.0f;
    bool isTransitioning = false;
    std::vector<StorySlide>* currentStory = nullptr;
    int storyIndex = 0;

    GameState stateBeforePause = GameState::MENU; // Remember state before pausing
    int pauseSelection = 0; // 0 = Resume, 1 = Exit to Menu

    // Sound IDs
    int sndStory = -1;
    int sndStoryWin = -1;
    int sndShoot = -1;
    int sndLevelComplete = -1;
    int sndMenu = -1;
    int sndPlayerHit = -1;
    int sndExplosion = -1;
    int sndGameOver = -1;

    void spawnEnemy() {
        std::uniform_real_distribution<float> xDist(40.0f, float(ScreenWidth() - 40.0f));
        std::uniform_real_distribution<float> vyDist(80.0f, 130.0f);
        std::uniform_real_distribution<float> vxDist(-40.0f, 40.0f);

        Enemy e;
        e.pos = { xDist(rng), -40.0f };
        e.vel = { vxDist(rng), vyDist(rng) };
        e.r = 20.0f;
        e.alive = true;
        e.inArena = false;
        e.decal = decEnemy;

        enemies.push_back(e);
    }

    void spawnAsteroid() {
        std::uniform_real_distribution<float> xDist(10.0f, float(ScreenWidth()) - 10.0f);
        std::uniform_real_distribution<float> vyDist(80.0f, 150.0f);
        std::uniform_real_distribution<float> vxDist(-20.0f, 20.0f);
        std::uniform_real_distribution<float> rDist(24.0f, 40.0f);

        Asteroid a;
        a.pos = { xDist(rng), -30.0f };
        a.vel = { vxDist(rng), vyDist(rng) };
        a.r = rDist(rng);
        a.alive = true;
        a.decal = decAsteroid;

        asteroids.push_back(a);
    }

    void spawnBullet(const olc::vf2d& startpos) {
        Bullet b;
        b.pos = startpos;
        b.vel = { 0.0f, -350.0f };
        b.r = 4.0f;
        b.alive = true;
        b.decal = decBullet;

        bullets.push_back(b);
    }

    void spawnEnemyBullet(const olc::vf2d& startPos) {
        EnemyBullet eb;
        eb.pos = startPos;
        eb.vel = { 0.0f, 220.0f };
        eb.r = 4.0f;
        eb.alive = true;
        eb.decal = decBullet;
        enemyBullets.push_back(eb);
    }

    void spawnBossBullets() {
        if (!boss.alive) return;

        olc::vf2d leftmuzz = boss.pos + olc::vf2d{ -30.0f, boss.r * 0.5f };
        olc::vf2d rightmuzz = boss.pos + olc::vf2d{ 30.0f, boss.r * 0.5f };

        EnemyBullet bl;
        bl.pos = leftmuzz;
        bl.vel = { 0.0f, 260.0f };
        bl.r = 4.0f;
        bl.alive = true;
        bl.decal = decBullet;

        EnemyBullet br = bl;
        br.pos = rightmuzz;

        enemyBullets.push_back(bl);
        enemyBullets.push_back(br);
    }

    void spawnExplosion(const olc::vf2d& pos, olc::Decal* decal, float maxTime, float scale = 1.0f) {
        Explosion e;
        e.pos = pos;
        e.decal = decal;
        e.maxTime = maxTime;
        e.timer = maxTime;
        e.scale = scale;
        explosions.push_back(e);
        olc::SOUND::PlaySample(sndExplosion);
    }

    bool OnUserCreate() override
    {
        olc::SOUND::InitialiseAudio();
        // Load sounds
        sndStory = olc::SOUND::LoadAudioSample("assets/audio/story.wav");
        sndStoryWin = olc::SOUND::LoadAudioSample("assets/audio/story_win.wav");
        sndShoot = olc::SOUND::LoadAudioSample("assets/audio/shoot.wav");
        sndLevelComplete = olc::SOUND::LoadAudioSample("assets/audio/level_complete.wav");
        sndMenu = olc::SOUND::LoadAudioSample("assets/audio/menu.wav");
        sndPlayerHit = olc::SOUND::LoadAudioSample("assets/audio/player_hit.wav");
        sndExplosion = olc::SOUND::LoadAudioSample("assets/audio/explosion.wav");
        sndGameOver = olc::SOUND::LoadAudioSample("assets/audio/game_over.wav");

        // load story images 
        StorySlide slide;

        // Intro story (4 images)
        slide.image = new olc::Decal(new olc::Sprite("assets/story/intro1.png"));
        slide.text = "Earth is under siege by an alien invasion force.";
        storyIntro.push_back(slide);

        slide.image = new olc::Decal(new olc::Sprite("assets/story/intro2.png"));
        slide.text = "You are humanity's last hope, piloting the experimental starfighter.";
        storyIntro.push_back(slide);

        slide.image = new olc::Decal(new olc::Sprite("assets/story/intro3.png"));
        slide.text = "Navigate through the asteroid belt and eliminate all threats!";
        storyIntro.push_back(slide);

        slide.image = new olc::Decal(new olc::Sprite("assets/story/intro4.png"));
        slide.text = "Navigate through the asteroid belt and eliminate all threats!";
        storyIntro.push_back(slide);

        slide.image = new olc::Decal(new olc::Sprite("assets/story/intro5.png"));
        slide.text = "Navigate through the asteroid belt and eliminate all threats!";
        storyIntro.push_back(slide);

        // Level 2 story (3 images)
        slide.image = new olc::Decal(new olc::Sprite("assets/story/level2_1.png"));
        slide.text = "You've cleared the asteroid belt! Enemy fighters approaching...";
        storyLevel2.push_back(slide);

        slide.image = new olc::Decal(new olc::Sprite("assets/story/level2_2.png"));
        slide.text = "Eliminate all enemy ships to proceed!";
        storyLevel2.push_back(slide);

        slide.image = new olc::Decal(new olc::Sprite("assets/story/level2_3.png"));
        slide.text = "Eliminate all enemy ships to proceed!";
        storyLevel2.push_back(slide);

        // Level 3 story (2 images)
        slide.image = new olc::Decal(new olc::Sprite("assets/story/level3_1.png"));
        slide.text = "The enemy fleet has been decimated!";
        storyLevel3.push_back(slide);

        slide.image = new olc::Decal(new olc::Sprite("assets/story/level3_2.png"));
        slide.text = "But their mothership has entered Earth's orbit. Destroy it!";
        storyLevel3.push_back(slide);

        slide.image = new olc::Decal(new olc::Sprite("assets/story/level3_3.png"));
        slide.text = "But their mothership has entered Earth's orbit. Destroy it!";
        storyLevel3.push_back(slide);

        // Win story (4 images)
        slide.image = new olc::Decal(new olc::Sprite("assets/story/win1.png"));
        slide.text = "The mothership explodes in a brilliant flash!";
        storyWin.push_back(slide);

        slide.image = new olc::Decal(new olc::Sprite("assets/story/win2.png"));
        slide.text = "Earth is saved! You are a hero!";
        storyWin.push_back(slide);

        slide.image = new olc::Decal(new olc::Sprite("assets/story/win3.png"));
        slide.text = "Earth is saved! You are a hero!";
        storyWin.push_back(slide);

        slide.image = new olc::Decal(new olc::Sprite("assets/story/win4.png"));
        slide.text = "Earth is saved! You are a hero!";
        storyWin.push_back(slide);

        // Lose story (2 images)
        slide.image = new olc::Decal(new olc::Sprite("assets/story/lose1.png"));
        slide.text = "Your ship takes critical damage...";
        storyLose.push_back(slide);

        slide.image = new olc::Decal(new olc::Sprite("assets/story/lose2.png"));
        slide.text = "Humanity falls to the invasion...";
        storyLose.push_back(slide);

        slide.image = new olc::Decal(new olc::Sprite("assets/story/lose3.png"));
        slide.text = "Humanity falls to the invasion...";
        storyLose.push_back(slide);

        // Load sprites
        sprBackground = new olc::Sprite("assets/sprites/bg_space.png");
        decBackground = new olc::Decal(sprBackground);

        sprPlayer = new olc::Sprite("assets/sprites/player_ship.png");
        decPlayer = new olc::Decal(sprPlayer);

        sprAsteroid = new olc::Sprite("assets/sprites/asteroid.png");
        decAsteroid = new olc::Decal(sprAsteroid);

        sprEnemy = new olc::Sprite("assets/sprites/enemy_ship.png");
        decEnemy = new olc::Decal(sprEnemy);

        sprBoss = new olc::Sprite("assets/sprites/boss_ship.png");
        decBoss = new olc::Decal(sprBoss);

        sprBullet = new olc::Sprite("assets/sprites/bullet.png");
        decBullet = new olc::Decal(sprBullet);

        sprBoomAsteroid = new olc::Sprite("assets/sprites/boom_asteroid.png");
        decBoomAsteroid = new olc::Decal(sprBoomAsteroid);

        sprBoomShip = new olc::Sprite("assets/sprites/boom_ship.png");
        decBoomShip = new olc::Decal(sprBoomShip);

        state = GameState::MENU;
        return true;
    }

    void ResetGame() {
        score = 0;
        hits = 0;
        player.lives = 3;
        enemiesKilled = 0;
        total_enemy_spawn = 0;

        asteroids.clear();
        bullets.clear();
        enemies.clear();
        enemyBullets.clear();

        enemySpawnTimer = 0.0f;
        enemyFireTimer = 0.0f;
        bossFireTimer = 0.0f;

        boss.hp = boss.maxHp;
        wins = false;
        boss.alive = false;

        currentStory = &storyIntro;
        storyIndex = 0;
        state = GameState::STORY;
    }

    void startLevel(int lvl) {
        currentLevel = lvl;
        levelTime = 0.0f;
        spawnTimer = 0.0f;
        enemySpawnTimer = 0.0f;
        enemyFireTimer = 0.0f;
        enemiesKilled = 0;
        total_enemy_spawn = 0;

        bullets.clear();
        asteroids.clear();
        enemies.clear();
        enemyBullets.clear();

        if (currentLevel == 1) {
            spawnRate = 0.5f;
        }
        else if (currentLevel == 2) {
            spawnRate = 0.7f;
            enemySpawnRate = 2.0f;
        }
        else if (currentLevel == 3) {
            spawnRate = 0.9f;
            enemySpawnRate = 2.5f;

            boss.Reset({ ScreenWidth() / 2.0f, -60.0f });
            boss.decal = decBoss;

            bossFireCooldown = 1.2f;
            bossFireTimer = 1.0f;
        }

        player.decal = decPlayer;
        player.Reset({ ScreenWidth() / 2.0f, ScreenHeight() - 60.0f });
    }

    void updateCurrentLevel(float dt) {

       // Don't update if paused
        if (state == GameState::PAUSED) {
            return;
        }

       // Transition Freeze
        if (isTransitioning) {
            transitionTimer -= dt;
            return;
        }

    // Normal Game Update (When not transitioning)

        levelTime += dt;

        // Player update
        player.Update(this, dt);

        // Auto-shooting 
        fireTimer -= dt;
        if (fireTimer <= 0.0f) {
            olc::vf2d spawnPos = player.pos + olc::vf2d{ 0.0, -player.r };
            spawnBullet(spawnPos);
            olc::SOUND::PlaySample(sndShoot);
            fireTimer = fireCoolDown;
        }

        // Spawn asteroids
        spawnTimer -= dt;
        if (spawnTimer <= 0.0f) {
            spawnAsteroid();
            spawnTimer = spawnRate;
        }

        // Spawn enemies (Level 2 and 3)
        if (currentLevel == 2 || currentLevel == 3) {
            enemySpawnTimer -= dt;
            if (enemySpawnTimer <= 0.0f) {
                int aliveEnemies = 0;
                for (auto& e : enemies) {
                    if (e.alive) aliveEnemies++;
                }

                bool canSpawn = true;
                if (currentLevel == 2 && total_enemy_spawn >= level2KillTarget)
                    canSpawn = false;

                if (canSpawn && aliveEnemies < maxEnemies) {
                    total_enemy_spawn++;
                    spawnEnemy();
                }
                enemySpawnTimer = enemySpawnRate;
            }
        }

        // Update bullets
        for (auto& b : bullets) {
            if (b.alive) b.Update(dt);
        }

        // Update asteroids
        for (auto& a : asteroids) {
            if (a.alive) a.Update(dt, ScreenHeight());
        }

        // Update enemies 
        for (auto& e : enemies) {
            if (e.alive) e.Update(dt, ScreenWidth(), ScreenHeight());
        }

        // Enemy shooting
        if (currentLevel == 2 || currentLevel == 3) {
            enemyFireTimer -= dt;
            if (enemyFireTimer <= 0.0f) {
                for (auto& e : enemies) {
                    if (!e.alive) continue;
                    olc::vf2d muz = e.pos + olc::vf2d{ 0.0f, e.r };
                    spawnEnemyBullet(muz);
                }
                enemyFireTimer = enemyFireCooldown;
            }
        }

        // Update enemy bullets
        for (auto& eb : enemyBullets) {
            if (eb.alive) eb.Update(dt, ScreenHeight());
        }
        

        // Boss update and shooting
        if (currentLevel == 3 && boss.alive) {
            boss.Update(dt, ScreenWidth());

            bossFireTimer -= dt;
            if (bossFireTimer <= 0.0f) {
                spawnBossBullets();
                bossFireTimer = bossFireCooldown;
            }
        }

        // Update Explosions
        for (auto& exp : explosions) {
            exp.timer -= dt;
        }

        // ===== COLLISION DETECTION =====

        // Asteroid vs bullets
        for (auto& b : bullets) {
            if (!b.alive) continue;
            for (auto& a : asteroids) {
                if (!a.alive) continue;
                float hitR = b.r + a.r;
                if (Dist2(b.pos, a.pos) <= hitR * hitR) {
                    b.alive = false;
                    spawnExplosion(a.pos, decBoomAsteroid, 0.25f, a.r * 2.0f / sprBoomAsteroid->width);
                    a.alive = false;
                    score += 5;
                    break;
                }
            }
        }

        // Enemy vs bullets
        for (auto& b : bullets) {
            if (!b.alive) continue;
            for (auto& e : enemies) {
                if (!e.alive) continue;
                float hitR = b.r + e.r;
                if (Dist2(b.pos, e.pos) <= hitR * hitR) {
                    b.alive = false;
                    e.alive = false;
                    spawnExplosion(e.pos, decBoomShip, 0.35f, (e.r * 2.0f) / sprBoomShip->width);
                    score += 10;
                    enemiesKilled += 1;
                    break;
                }
            }
        }

        // Asteroid vs player
        for (auto& a : asteroids) {
            if (!a.alive) continue;
            float hitR = a.r + player.r;
            if (Dist2(a.pos, player.pos) <= hitR * hitR) {
                if (player.invincibleTimer <= 0.0f) {
                    a.alive = false;
                    spawnExplosion(a.pos, decBoomAsteroid, 0.25f, a.r * 2.0f / sprBoomAsteroid->width);
                    hits++;
                    player.lives--;
                    player.invincibleTimer = 2.0f;
                    olc::SOUND::PlaySample(sndPlayerHit);

                    if (player.lives <= 0) {
                        spawnExplosion(player.pos, decBoomShip, 0.35f, (player.r * 2.0f) / sprBoomShip->width);
                        olc::SOUND::PlaySample(sndGameOver);

                        if (!isTransitioning) {
                            isTransitioning = true;
                            transitionTimer = 2.0f;
                            wins = false;
                        }
                    }
                }
                else {
                    a.alive = false;
                    spawnExplosion(a.pos, decBoomAsteroid, 0.25f, a.r * 2.0f / sprBoomAsteroid->width);
                }
            }
        }

        // Enemy vs player
        for (auto& e : enemies) {
            if (!e.alive) continue;
            float hitR = e.r + player.r;
            if (Dist2(e.pos, player.pos) <= hitR * hitR) {
                e.alive = false;
                spawnExplosion(e.pos, decBoomShip, 0.35f, (e.r * 2.0f) / sprBoomShip->width);
                if (player.invincibleTimer <= 0.0f) {
                    hits++;
                    player.lives--;
                    player.invincibleTimer = 2.0f;
                    olc::SOUND::PlaySample(sndPlayerHit);

                    if (player.lives <= 0) {
                        spawnExplosion(player.pos, decBoomShip, 0.35f, (e.r * 2.0f) / sprBoomShip->width);
                        olc::SOUND::PlaySample(sndGameOver);
                        
                        if (!isTransitioning) {
                            isTransitioning = true;
                            transitionTimer = 2.0f;
                            wins = false;
                        }
                    }
                }
            }
        }

        // Enemy bullets vs player
        for (auto& eb : enemyBullets) {
            if (!eb.alive) continue;
            float hitR = eb.r + player.r;
            if (Dist2(eb.pos, player.pos) <= hitR * hitR) {
                eb.alive = false;
                if (player.invincibleTimer <= 0.0f) {
                    hits++;
                    player.lives--;
                    player.invincibleTimer = 2.0f;
                    olc::SOUND::PlaySample(sndPlayerHit);

                    if (player.lives <= 0) {
                        spawnExplosion(player.pos, decBoomShip, 0.35f, (player.r * 2.0f) / sprBoomShip->width);
                        olc::SOUND::PlaySample(sndGameOver);

                        if (!isTransitioning) {
                            isTransitioning = true;
                            transitionTimer = 2.0f;
                            wins = false;
                        }
                      
                    }
                }
            }
        }

        // Player bullets vs boss
        if (currentLevel == 3 && boss.alive) {
            for (auto& b : bullets) {
                if (!b.alive) continue;
                float hitR = b.r + boss.r;
                if (Dist2(b.pos, boss.pos) <= hitR * hitR) {
                    b.alive = false;
                    boss.hp -= 5;
                    score += 25;

                    if (boss.hp <= 0) {
                        boss.hp = 0;
                        boss.alive = false;
                        spawnExplosion(boss.pos, decBoomShip, 0.35f, (boss.r * 2.0f) / sprBoomShip->width);

                        if (!isTransitioning) {
                            isTransitioning = true;
                            transitionTimer = 2.0f;
                            wins = true;
                        }
                    }
                    break;
                }
            }
        }

        // Boss vs player
        if (currentLevel == 3 && boss.alive) {
            float hitR = boss.r + player.r;
            if (Dist2(boss.pos, player.pos) <= hitR * hitR) {
                if (player.invincibleTimer <= 0.0f) {
                    hits++;
                    player.lives--;
                    player.invincibleTimer = 2.0f;
                    olc::SOUND::PlaySample(sndPlayerHit);

                    if (player.lives <= 0) {
                        spawnExplosion(boss.pos, decBoomShip, 0.35f, (boss.r * 2.0f) / sprBoomShip->width);
                        olc::SOUND::PlaySample(sndGameOver);

                        if (!isTransitioning) {
                            isTransitioning = true;
                            transitionTimer = 2.0f;
                            wins = false;
                        }
                    }
                }
            }
        }

        // Clean up dead objects
        bullets.erase(
            std::remove_if(bullets.begin(), bullets.end(),
                [](const Bullet& b) {return !b.alive; }),
            bullets.end()
        );

        asteroids.erase(
            std::remove_if(asteroids.begin(), asteroids.end(),
                [](const Asteroid& b) {return !b.alive; }),
            asteroids.end()
        );

        enemies.erase(
            std::remove_if(enemies.begin(), enemies.end(),
                [](const Enemy& e) { return !e.alive; }),
            enemies.end()
        );

        enemyBullets.erase(
            std::remove_if(enemyBullets.begin(), enemyBullets.end(),
                [](const EnemyBullet& eb) { return !eb.alive; }),
            enemyBullets.end()
        );
        explosions.erase(
            std::remove_if(explosions.begin(), explosions.end(),
                [](const Explosion& exp) {return exp.timer <= 0.0f; }),
            explosions.end()
        );
    }

    bool OnUserUpdate(float dt) override
    {
        Clear(olc::BLACK);

        // Handle ESC key to pause/unpause
        if (GetKey(olc::Key::ESCAPE).bPressed) {
            olc::SOUND::PlaySample(sndMenu);

            if (state == GameState::LEVEL_PLAY || state == GameState::LEVEL_INTRO || state == GameState::STORY) {
                stateBeforePause = state;
                state = GameState::PAUSED;
                pauseSelection = 0;
            }
            else if (state == GameState::PAUSED) {
                state = stateBeforePause;
            }
        }

        switch (state)
        {

        case GameState::MENU:
        {
            DrawString(ScreenWidth() / 2 - 130, ScreenHeight() / 2 - 60,
                "OPERATION STARFALL", olc::WHITE, 2);
            DrawString(ScreenWidth() / 2 - 100, ScreenHeight() / 2 - 10,
                "Press ENTER to Start", olc::YELLOW, 1);
            DrawString(ScreenWidth() / 2 - 120, ScreenHeight() / 2 + 30,
                "Arrow Keys / WASD to Move", olc::CYAN, 1);
            DrawString(ScreenWidth() / 2 - 90, ScreenHeight() / 2 + 50,
                "Auto-Fire Enabled!", olc::GREEN, 1);

            if (GetKey(olc::Key::ENTER).bPressed) {
                olc::SOUND::PlaySample(sndMenu);
                ResetGame();
            }
            break;
        }
        case GameState::STORY:
        {   
            // Play story music once when entering story
            static bool storyMusicPlayed = false;
            if (!storyMusicPlayed) {
                olc::SOUND::PlaySample(sndStory, true); // true = loop
                storyMusicPlayed = true;
            }

            // Draw current story image
            if (currentStory && storyIndex < currentStory->size()) {
                StorySlide& slide = (*currentStory)[storyIndex];

                // Reset cinematic motion on slide change
                if (storyIndex != lastStoryIndex) {
                    lastStoryIndex = storyIndex;
                    storyT = 0.0f;
                }
                storyT += dt;


                // Fit image to screen
                float scale = std::min(
                    float(ScreenWidth()) / slide.image->sprite->width,
                    float(ScreenHeight() - 100) / slide.image->sprite->height
                );

                scale = std::min(scale, 1.8f);

                // Smooth cinematic pan
                float offX = panX * std::sin(2.0f * 3.14159f * panSpeedX * storyT);
                float offY = panY * std::cos(2.0f * 3.14159f * panSpeedY * storyT);

                // Gentle zoom
                float zoom = 1.0f + zoomAmp * std::sin(2.0f * 3.14159f * zoomSpeed * storyT);
                float finalScale = scale * storyOverscale * zoom;

                // Final size
                olc::vf2d size = {
                    slide.image->sprite->width * finalScale,
                    slide.image->sprite->height * finalScale
                };

                // Center + drift
                olc::vf2d basePos = {
                    (ScreenWidth() - size.x) / 2.0f,
                    (ScreenHeight() - 100 - size.y) / 2.0f
                };

                olc::vf2d pos = basePos + olc::vf2d{ offX, offY };
                pos.y -= 40.0f;

                DrawDecal(pos, slide.image, { finalScale, finalScale });




                // Draw text box at bottom
                FillRect(0, ScreenHeight() - 80, ScreenWidth(), 80,
                    olc::Pixel(0, 0, 0, 220));
                DrawRect(0, ScreenHeight() - 80, ScreenWidth(), 80, olc::WHITE);

                // Center the text
                int textWidth = slide.text.length() * 8;
                int textX = (ScreenWidth() - textWidth) / 2;
                DrawString(textX, ScreenHeight() - 60, slide.text, olc::WHITE, 1);

                // Instruction
                DrawString(ScreenWidth() / 2 - 100, ScreenHeight() - 30,
                    "Press ENTER to continue", olc::YELLOW, 1);
            }

            if (GetKey(olc::Key::ENTER).bPressed) {
                olc::SOUND::PlaySample(sndMenu);
                storyIndex++;

                // Check if story is complete
                if (currentStory && storyIndex >= currentStory->size()) {
                    // Determine what comes next
                    if (currentStory == &storyIntro) {
                        startLevel(1);  // ← Explicitly level 1
                        introTimer = 0.0f;
                        state = GameState::LEVEL_INTRO;
                    }
                    else if (currentStory == &storyLevel2) {
                        startLevel(2);  // ← Explicitly level 2
                        introTimer = 0.0f;
                        state = GameState::LEVEL_INTRO;
                    }
                    else if (currentStory == &storyLevel3) {
                        startLevel(3);  // ← Explicitly level 3
                        introTimer = 0.0f;
                        state = GameState::LEVEL_INTRO;
                    }
                    else if (currentStory == &storyWin || currentStory == &storyLose) {
                        // Go to game over
                        state = GameState::GAME_OVER;
                    }
                }
            }
            break;
        }

        case GameState::LEVEL_INTRO:
        {
            introTimer += dt;
            bool visible = fmodf(introTimer * 4.0f, 2.0f) < 1.0f;

            std::string title;
            if (currentLevel == 1)
                title = "LEVEL 1: ASTEROID BELT";
            else if (currentLevel == 2)
                title = "LEVEL 2: FRONTIER ZONE";
            else if (currentLevel == 3)
                title = "LEVEL 3: ORBITAL SIEGE";

            if (visible) {
                int tw = int(title.size() * 8 * 2);
                int x = ScreenWidth() / 2 - tw / 2;
                int y = ScreenHeight() / 2 - 10;
                DrawString(x, y, title, olc::WHITE, 2);
            }

            DrawString(ScreenWidth() / 2 - 90, ScreenHeight() / 2 + 30,
                "Press ENTER to Begin", olc::YELLOW, 1);

            if (GetKey(olc::Key::ENTER).bPressed) {
                olc::SOUND::PlaySample(sndMenu);
                state = GameState::LEVEL_PLAY;
            }
            break;
        }

        case GameState::LEVEL_PLAY:
        {
            
            // 2. DRAW BACKGROUND (Lowest layer)
            if(state != GameState::PAUSED && !isTransitioning){
            bgOffset += 40.0f * dt;
                if (bgOffset >= sprBackground->height)
                    bgOffset -= sprBackground->height;
            }

            // Draw background decals
            SetDecalMode(olc::DecalMode::ADDITIVE);
            DrawDecal({ 0.0f, -bgOffset }, decBackground);
            DrawDecal({ 0.0f, -bgOffset + sprBackground->height }, decBackground);

            SetDecalMode(olc::DecalMode::NORMAL);

            // 3. RUN GAME LOGIC/UPDATE/COLLISIONS
            updateCurrentLevel(dt);

            // 4. DRAW ENTITIES (Middle layers)
            for (auto& a : asteroids) a.Draw(this);
            for (auto& e : enemies) e.Draw(this);
            if (boss.alive && currentLevel == 3) boss.Draw(this);
            for (auto& eb : enemyBullets) eb.Draw(this);
            for (auto& b : bullets) b.Draw(this);
            player.Draw(this); // Draw Player on top of other entities

            for (auto& exp : explosions) {
                // Ensure additive blending for glowing explosions
                SetDecalMode(olc::DecalMode::ADDITIVE);

                olc::vf2d size = { exp.decal->sprite->width * exp.scale, exp.decal->sprite->height * exp.scale };

                // Draw decal centered on the entity's position
                DrawDecal(
                    exp.pos - size / 2.0f,
                    exp.decal,
                    { exp.scale, exp.scale }
                );
            }

            // 5. DRAW HUD (Top layer)

            // Solid black background for main HUD (left side), reduced size, higher opacity (240)
            FillRect(0, 0, 220, 115, olc::Pixel(0, 0, 0, 240));
            DrawRect(0, 0, 220, 115, olc::WHITE); // Border

            std::string lvlText;
            if (currentLevel == 1)
                lvlText = "LEVEL 1: ASTEROID BELT";
            else if (currentLevel == 2)
                lvlText = "LEVEL 2: FRONTIER ZONE";
            else if (currentLevel == 3)
                lvlText = "LEVEL 3: ORBITAL SIEGE";

            // Draw Level Text (Slightly larger scale 2.0x)
            DrawString(8, 8, lvlText, olc::WHITE, 1.5f);
            DrawLine(8, 25, 212, 25, olc::Pixel(100, 100, 100)); // Thin separator line

            // Draw Stats (Consistent 1.5x scale)
            DrawString(8, 35, "Score: " + std::to_string(score), olc::YELLOW, 1.5f);
            DrawString(8, 55, "Lives: " + std::to_string(player.lives), olc::GREEN, 1.5f);
            DrawString(8, 75, "Hits Taken: " + std::to_string(hits), olc::RED, 1.5f);

            // Objective display (larger scale 2.0x for focus)
            if (currentLevel == 1) {
                float targetDuration = level1Duration;
                int timeLeft = int(std::max(0.0f, targetDuration - levelTime));
                DrawString(8, 95, "TIME: " + std::to_string(timeLeft) + "s", olc::CYAN, 1.8f);
            }
            else if (currentLevel == 2) {
                int killed = enemiesKilled;
                int target = level2KillTarget;
                DrawString(8, 95, "KILLS: " + std::to_string(killed) + "/" + std::to_string(target), olc::CYAN, 1.8f);
            }
            else if (currentLevel == 3) {
                // --- Right HUD Panel (Boss HP) ---
                int barW = 200;
                int barH = 15; // Slightly thinner bar
                int barX = ScreenWidth() - barW - 15; // Move closer to right edge
                int barY = 25; // Move higher up

                float hpRatio = boss.alive ? float(boss.hp) / float(boss.maxHp) : 0.0f;
                int hpW = int(barW * hpRatio);

                // Background box for boss HP area (Condensed to height 60)
                FillRect(barX - 10, barY - 25, barW + 20, 60, olc::Pixel(0, 0, 0, 240));
                DrawRect(barX - 10, barY - 25, barW + 20, 60, olc::WHITE);

                // Label above bar
                // Adjusted Y-coordinate (-15) to sit closer to the bar
                DrawString(barX + 65, barY - 15, "BOSS HP", olc::WHITE, 1.0f); // Reduced text scale for max compactness

                // HP bar outline
                DrawRect(barX - 2, barY - 2, barW + 4, barH + 4, olc::WHITE);
                // Background
                FillRect(barX, barY, barW, barH, olc::VERY_DARK_RED);
                // Current HP
                if (hpW > 0) {
                    olc::Pixel hpColor = hpRatio > 0.5f ? olc::GREEN : (hpRatio > 0.25f ? olc::YELLOW : olc::RED);
                    FillRect(barX, barY, hpW, barH, hpColor);
                }

                // HP text below bar
                // Adjusted Y-coordinate (+18) to sit closer to the bar
                std::string hpText = std::to_string(boss.hp) + " / " + std::to_string(boss.maxHp);
                DrawString(barX + 55, barY + 18, hpText, olc::WHITE, 1.5f);
            }


            // 6. LEVEL COMPLETE CHECK
            if (player.lives > 0) {
                // Player is alive - check level completion
                if (currentLevel == 1) {
                    if (levelTime >= level1Duration) {
                        if (!isTransitioning) {
                            isTransitioning = true;
                            transitionTimer = 2.0f;
                            olc::SOUND::PlaySample(sndLevelComplete);
                        }
                        else if (transitionTimer <= 0.0f) {
                            isTransitioning = false;
                            currentLevel = 2;  // ← ADD THIS LINE
                            currentStory = &storyLevel2;
                            storyIndex = 0;
                            state = GameState::STORY;
                        }
                    }
                }
                else if (currentLevel == 2) {
                    if (enemiesKilled >= level2KillTarget) {
                        if (!isTransitioning) {
                            isTransitioning = true;
                            transitionTimer = 2.0f;
                            olc::SOUND::PlaySample(sndLevelComplete);
                        }
                        else if (transitionTimer <= 0.0f) {
                            isTransitioning = false;
                            currentLevel = 3; 
                            currentStory = &storyLevel3;
                            storyIndex = 0;
                            state = GameState::STORY;
                        }
                    }
                }
                else if (currentLevel == 3) {
                    if (!boss.alive && wins) {
                        if (!isTransitioning) {
                            isTransitioning = true;
                            transitionTimer = 2.0f;
                            olc::SOUND::PlaySample(sndLevelComplete);
                        }
                        else if (transitionTimer <= 0.0f) {
                            isTransitioning = false;
                            currentStory = &storyWin;
                            storyIndex = 0;
                            state = GameState::STORY;
                        }
                    }
                }
            }
            else {
                // PLAYER IS DEAD 
                if (!isTransitioning) {
                    isTransitioning = true;
                    transitionTimer = 2.0f;
                    wins = false;
                    olc::SOUND::PlaySample(sndGameOver);
                }
                else if (transitionTimer <= 0.0f) {
                    isTransitioning = false;
                    currentStory = &storyLose;
                    storyIndex = 0;
                    state = GameState::STORY;
                }

            }
            break;
        }

        case GameState::PAUSED:
        {
            // Just draw solid black background - no need to show what's behind
            FillRect(0, 0, ScreenWidth(), ScreenHeight(), olc::Pixel(0, 0, 0, 255));

            // Draw pause menu box
            int boxW = 400;
            int boxH = 300;
            int boxX = ScreenWidth() / 2 - boxW / 2;
            int boxY = ScreenHeight() / 2 - boxH / 2;

            FillRect(boxX, boxY, boxW, boxH, olc::Pixel(20, 20, 40));
            DrawRect(boxX, boxY, boxW, boxH, olc::WHITE);

            DrawString(boxX + boxW / 2 - 40, boxY + 30, "PAUSED", olc::YELLOW, 3);

            std::string option1 = "RESUME";
            std::string option2 = "EXIT TO MENU";

            olc::Pixel color1 = (pauseSelection == 0) ? olc::GREEN : olc::WHITE;
            olc::Pixel color2 = (pauseSelection == 1) ? olc::GREEN : olc::WHITE;

            DrawString(boxX + boxW / 2 - 40, boxY + 120, option1, color1, 2);
            DrawString(boxX + boxW / 2 - 90, boxY + 170, option2, color2, 2);

            DrawString(boxX + 60, boxY + boxH - 40, "UP/DOWN to select, ENTER to confirm", olc::CYAN, 1);

            if (GetKey(olc::Key::UP).bPressed || GetKey(olc::Key::W).bPressed) {
                olc::SOUND::PlaySample(sndMenu);
                pauseSelection = 0;
            }
            if (GetKey(olc::Key::DOWN).bPressed || GetKey(olc::Key::S).bPressed) {
                olc::SOUND::PlaySample(sndMenu);
                pauseSelection = 1;
            }

            if (GetKey(olc::Key::ENTER).bPressed) {
                olc::SOUND::PlaySample(sndMenu);
                if (pauseSelection == 0) {
                    state = stateBeforePause;
                }
                else if (pauseSelection == 1) {
                    state = GameState::MENU;
                }
            }

            break;
        }

        case GameState::GAME_OVER:
        {
            std::string line1 = wins ? "MISSION COMPLETE!" : "MISSION FAILED";
            std::string line2 = wins ? "Earth is Saved!" : "Earth has Fallen";
            std::string line3 = "Final Score: " + std::to_string(score);
            std::string line4 = "Press ENTER for Menu";

            int x1 = ScreenWidth() / 2 - int(line1.size()) * 8;
            int x2 = ScreenWidth() / 2 - int(line2.size()) * 8;
            int x3 = ScreenWidth() / 2 - int(line3.size()) * 8;
            int x4 = ScreenWidth() / 2 - int(line4.size()) * 8;

            olc::Pixel color1 = wins ? olc::GREEN : olc::RED;
            olc::Pixel color2 = wins ? olc::YELLOW : olc::DARK_RED;

            DrawString(x1, ScreenHeight() / 2 - 40, line1, color1, 2);
            DrawString(x2, ScreenHeight() / 2 - 5, line2, color2, 1);
            DrawString(x3, ScreenHeight() / 2 + 25, line3, olc::WHITE, 1);
            DrawString(x4, ScreenHeight() / 2 + 55, line4, olc::CYAN, 1);

            if (GetKey(olc::Key::ENTER).bPressed) {
                olc::SOUND::PlaySample(sndMenu);
                state = GameState::MENU;
            }
            break;
        }
        }

        return true;
    }
};

int main()
{
    SpaceShooter game;
    if (game.Construct(900, 600, 1, 1))
        game.Start();
    return 0;
}
