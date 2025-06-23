#include "game.h"

#include "audio/audio_engine.h"
#include "raylib.h"
#include <iostream>

Game::Game()
: mIsRunning(false)
, mWindowWidth(1024)
, mWindowHeight(768)
{}

void Game::Initialize()
{
	AudioEngine::Initialize(); // AUDIO ENGINE INIT
	AudioEngine::Initialize();

	SetTargetFPS(60);
	InitWindow(static_cast<int>(mWindowWidth), static_cast<int>(mWindowHeight), "FMOD is Alive!");
	mIsRunning = true;

	std::cout << "Game Initialized" << std::endl;
}

void Game::Run()
{
	Start();
	while (IsRunning())
	{
		ProcessInput();
		Update();
		Render();
	}
}

void Game::Terminate() const
{
	CloseWindow();

	AudioEngine::Terminate(); // AUDIO ENGINE TERM

	std::cout << "Game destroyed" << std::endl;
}

bool Game::IsRunning() const
{
	return mIsRunning;
}

void Game::Update()
{
	AudioEngine::Update(); // AUDIO ENGINE UPDATE
}

void Game::ProcessInput()
{
	if (WindowShouldClose())
	{
		mIsRunning = false;
	}
}

void Game::Start()
{
	AudioEngine::LoadSoundBank("Music.bank");
	AudioEngine::PlayAudioEvent("event:/MusicTest");

	std::cout << "Game Started" << std::endl;
}

void Game::Render() const
{
	BeginDrawing();
	ClearBackground(DARKGRAY);

	DrawText("FMOD is Alive!",
		static_cast<int>(mWindowWidth * 0.2f),
		static_cast<int>(mWindowHeight * 0.45f),
		90, LIGHTGRAY);

	EndDrawing();
}
