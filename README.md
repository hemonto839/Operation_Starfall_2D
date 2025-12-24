# 🚀 Operation Starfall 2D

**Operation Starfall 2D** is a complete **2D vertical scrolling space-shooter game** developed in **C++** using the **olc::PixelGameEngine**.  
The game puts the player in command of humanity’s last hope against a sudden and devastating alien invasion, delivering a classic arcade-style experience with structured progression, narrative storytelling, and a climactic boss battle.

---

## 🎮 Game Overview

- **Genre:** 2D Vertical Scrolling Shooter (Shmup)
- **Platform:** Windows (x64)
- **Language:** C++ (Modern Standard)
- **Engine:** olc::PixelGameEngine (PGE)
- **Game Mode:** Single Player

Operation Starfall 2D is designed to replicate the intensity of classic arcade shooters while maintaining clean architecture, efficient collision handling, and a complete start-to-finish gameplay loop.

---

## 🧩 Core Features

- 🎯 **Three-Level Progression**
  - Level 1: Asteroid Belt (Survival-based)
  - Level 2: Frontier Zone (Enemy patrol & kill target)
  - Level 3: Orbital Siege (Boss fight)

- 🧠 **Game State Machine**
  - MENU → STORY → LEVEL INTRO → GAMEPLAY → WIN / LOSE
  - Smooth transitions using a structured state system

- 💥 **Real-Time Combat**
  - Auto-fire player weapon system
  - Enemy bullets and boss firing patterns
  - Explosion effects on destruction

- ❤️ **Lives & Invincibility System**
  - Player starts with 3 lives
  - Temporary invincibility after taking damage

- 🧮 **Scoring System**
  - Asteroids: +5 points
  - Enemy ships: +10 points
  - Boss hit (per bullet): +25 points

- 📖 **Story Presentation System**
  - Narrative slides before levels and endings
  - Two definitive endings: **Win** or **Lose**

- ⏸️ **Pause Menu & Audio Support**
  - In-game pause functionality
  - Background sound integration

---

## 🎮 Controls

| Action | Key |
|------|-----|
| Move Up | ↑ Arrow |
| Move Down | ↓ Arrow |
| Move Left | ← Arrow |
| Move Right | → Arrow |
| Confirm / Continue | ENTER |
| Pause Game | ESC |

---

## 🏗️ Technical Highlights

- **Efficient Entity Management**
  - Uses `std::vector` containers
  - Cleaned with the Erase–Remove Idiom

- **Optimized Collision Detection**
  - Radius-based circle collision
  - Distance-squared (`Dist2`) checks (no costly square roots)

- **Clean Architecture**
  - Modular entities (Player, Enemy, Boss, Bullets, Explosions)
  - Central game loop with delta-time simulation

---

## ▶️ How to Play (Windows)

1. Download the latest release from **GitHub Releases**
2. Extract the ZIP file
3. Run:

Also there is provided a zip file with exe file anyone can directly run it to check !!
