#ifndef AUDIO_ENGINE_H
#define AUDIO_ENGINE_H

#include "fmod_studio.hpp"

#include <memory>
#include <string>
#include <unordered_map>

using EventInstance = FMOD::Studio::EventInstance;
using System = FMOD::Studio::System;

class AudioEngine
{
	public:
		static AudioEngine& Get();

		bool Initialize();
		void Terminate();

		// Call this every frame on the program's update loop
		static void Update();

		static bool IsInitialized();
		void SetSoundBankRootDirectory(const std::string& directory);
		static bool LoadSoundBank(const std::string& filePath);
		static EventInstance* PlayAudioEvent(const std::string& studioPath,
			bool autoStart = true, bool autoRelease = true);

	private:
		static std::unique_ptr<AudioEngine> sInstance;
		System* StudioSystem;
		bool bIsMainBankLoaded;
		std::string mSoundBankRootDirectory;
		std::unordered_map<std::string, uint32_t> additionalPluginHandles;

		AudioEngine();
		void RegisterAdditionalPlugins(const std::vector<std::string>& pluginNames, const std::string& rootPath);
};

#endif
