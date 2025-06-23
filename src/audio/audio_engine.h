#ifndef AUDIO_ENGINE_H
#define AUDIO_ENGINE_H

#include "fmod_studio.hpp"

#include <memory>
#include <string>
#include <unordered_map>

using System = FMOD::Studio::System;

// Audio Event Instance
using EventInstance = FMOD::Studio::EventInstance;
using Audio3DAttributes = FMOD_3D_ATTRIBUTES;

class AudioEngine
{
	public:
		static AudioEngine& Get();

		static bool Initialize();
		static void Terminate();

		// Call this every frame on the program's update loop
		static void Update();
		static bool IsInitialized();

		// Soundbanks

		static void SetSoundBankRootDirectory(const std::string& directory);
		static bool LoadSoundBank(const std::string& filePath);

		// Events

		static EventInstance* PlayAudioEvent(const std::string& studioPath,
			const Audio3DAttributes& audio3dAttributes = Audio3DAttributes(),
			bool autoStart = true,
			bool autoRelease = true);

		// Audio Instances

		static bool InstanceStart(EventInstance* audioInstance);
		static bool InstanceStop(EventInstance* audioInstance, bool bAllowFadeOut = true);
		static bool InstanceRelease(EventInstance* audioInstance);
		static bool InstanceSetPaused(EventInstance* audioInstance, bool bPaused);
		static bool InstanceIsPaused(const EventInstance* audioInstance, bool& outPaused);

		// Parameters

		static bool SetGlobalParameterByName(const std::string& name,
			float value, bool bIgnoreSeekSpeed = false);
		static bool SetGlobalParameterByNameWithLabel(const std::string& name,
			const std::string& label, bool bIgnoreSeekSpeed = false);
		static bool SetParameterByName(EventInstance* audioInstance,
			const std::string& name, float value, bool bIgnoreSeekSpeed = false);
		static bool SetParameterByNameWithLabel(EventInstance* audioInstance,
			const std::string& name, const std::string& label, bool bIgnoreSeekSpeed = false);

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
