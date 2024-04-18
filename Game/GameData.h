#ifndef _GAME_DATA_H_
#define _GAME_DATA_H_

#include "GameState.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "TextGO2D.h"
#include "GameObject.h"  // Ensure you have a GameObject base class
#include <vector>

using namespace DirectX;

struct GameData
{
	float m_dt;  // Time step since last frame
	GameState m_GS; // Global GameState

	// Player input
	Keyboard::State m_KBS;
	Mouse::State m_MS;
	Keyboard::KeyboardStateTracker m_KBS_tracker;

	float spawnDelay = 10.0f;
	bool enemySpawned = false;
	TextGO2D* countdownText = nullptr; // To display countdown

	std::vector<GameObject*> m_GameObjects; // List to store all game objects
	int m_score = 0; // Score tracking
};

#endif
