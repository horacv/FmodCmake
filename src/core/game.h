#ifndef GAME_HPP
#define GAME_HPP

#include "audio/audio_engine.h"
#include <mutex>

class Game
{
	public:
		Game();

		void Initialize();
		void Run();
		void Terminate() const;
		[[nodiscard]] bool IsRunning() const;

	private:
		bool mIsRunning;

		float mWindowWidth;
		float mWindowHeight;

		mutable std::mutex mMusicDataMutex;
		int mCurrentMusicBar;
		int mCurrentMusicBeat;

		void Update();
		void ProcessInput();
		void Start();
		void Render() const;

		/**
		 * Audio Event Callback.
		 * Refer to: https://www.fmod.com/docs/2.03/api/core-api-system.html#system_setcallback
		 * "System callbacks can be called by a variety of FMOD threads,
		 *  so make sure any code executed inside the callback is thread safe"
		 */
		static FMOD_RESULT F_CALL GlobalAudioEventCallback(FMOD_STUDIO_EVENT_CALLBACK_TYPE type,
			FMOD_STUDIO_EVENTINSTANCE* eventInstance, void* properties);

		void SetCurrentMusicBarAndBeat(int bar, int beat);
		std::pair<int, int> GetCurrentMusicBarAndBeat() const;
};
#endif
