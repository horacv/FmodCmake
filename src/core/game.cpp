#include "game.h"

#include "audio/audio_engine.h"
#include "raylib.h"
#include <format>
#include <iostream>

Game::Game()
	: mIsRunning(false)
	, mWindowWidth(1024)
	, mWindowHeight(768)
	, mCurrentMusicBar(0)
	, mCurrentMusicBeat(0)
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
	AudioEngine::LoadSoundBankFile("Music.bank");
	AudioEngine::PlayAudioEvent("event:/MusicTest",{},
		this, GlobalAudioEventCallback, FMOD_STUDIO_EVENT_CALLBACK_TIMELINE_BEAT);

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

	auto [bar, beat] = GetCurrentMusicBarAndBeat();
	const std::string musicBeatText = std::format("Music Bar: {} Beat: {}", bar, beat);
	DrawText(musicBeatText.c_str(),
		static_cast<int>(mWindowWidth * 0.02f),
		static_cast<int>(mWindowHeight * 0.95f),
		24, LIGHTGRAY);

	EndDrawing();
}

FMOD_RESULT Game::GlobalAudioEventCallback(const FMOD_STUDIO_EVENT_CALLBACK_TYPE type,
	FMOD_STUDIO_EVENTINSTANCE* eventInstance, void* properties)
{
	if (type == FMOD_STUDIO_EVENT_CALLBACK_TIMELINE_BEAT)
	{
		const auto* musicProperties = static_cast<FMOD_STUDIO_TIMELINE_BEAT_PROPERTIES*>(properties);
		const auto* audioInstance = reinterpret_cast<AudioInstance*>(eventInstance);
		void* userData;
		if (audioInstance->getUserData(&userData) == FMOD_OK)
		{
			if (const auto game = static_cast<Game*>(userData))
			{
				game->SetCurrentMusicBarAndBeat(musicProperties->bar, musicProperties->beat);
				return FMOD_OK;
			}
		}
	}

	// Add more callback types here
	//...

	//...

	return FMOD_ERR_BADCOMMAND;
}

void Game::SetCurrentMusicBarAndBeat(const int bar, const int beat)
{
	std::lock_guard lock(mMusicDataMutex);
	mCurrentMusicBar = bar;
	mCurrentMusicBeat = beat;
}

std::pair<int, int> Game::GetCurrentMusicBarAndBeat() const
{
	std::lock_guard lock(mMusicDataMutex);
	return {mCurrentMusicBar, mCurrentMusicBeat};
}