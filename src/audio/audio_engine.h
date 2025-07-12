#ifndef AUDIO_ENGINE_H
#define AUDIO_ENGINE_H

#include "fmod_studio.hpp"

#include <memory>
#include <string>
#include <unordered_map>

#include "audio_config.h"

using StudioSystem = FMOD::Studio::System;
using CoreSystem = FMOD::System;

using AudioInstance = FMOD::Studio::EventInstance;
using Audio3DAttributes = FMOD_3D_ATTRIBUTES;
using AudioBank = FMOD::Studio::Bank;
using AudioEventCallback = FMOD_STUDIO_EVENT_CALLBACK;
using AudioCallbackType = FMOD_STUDIO_EVENT_CALLBACK_TYPE;

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
		static bool LoadSoundBankFile(const std::string& filePath);
		static bool LoadSoundBankFile(const std::string& filePath, AudioBank*& outBankPtr);
		static bool UnloadSoundBank(const std::string& studioPath);
		static bool UnloadSoundBank(AudioBank* bank);

		// Events

		static AudioInstance* PlayAudioEvent(const std::string& studioPath,
			const Audio3DAttributes& audio3dAttributes = Audio3DAttributes(),
			void* userData = nullptr,
			AudioEventCallback callback = nullptr,
			AudioCallbackType callbackType = FMOD_STUDIO_EVENT_CALLBACK_ALL,
			bool autoStart = true,
			bool autoRelease = true);

		// Audio Instances

		static bool InstanceStart(AudioInstance* instance);
		static bool InstanceStop(AudioInstance* instance, bool bAllowFadeOut = true);
		static bool InstanceRelease(AudioInstance* instance);
		static bool InstanceSetPaused(AudioInstance* instance, bool bPaused);
		static bool InstanceIsPaused(const AudioInstance* instance, bool& outPaused);

		// Parameters

		static bool SetGlobalParameterByName(const std::string& name,
			float value, bool bIgnoreSeekSpeed = false);
		static bool SetGlobalParameterByNameWithLabel(const std::string& name,
			const std::string& label, bool bIgnoreSeekSpeed = false);
		static bool SetParameterByName(AudioInstance* instance,
			const std::string& name, float value, bool bIgnoreSeekSpeed = false);
		static bool SetParameterByNameWithLabel(AudioInstance* instance,
			const std::string& name, const std::string& label, bool bIgnoreSeekSpeed = false);

	private:
		static std::unique_ptr<AudioEngine> sInstance;
		StudioSystem* StudioSystem;
		bool bMainBanksLoaded;
		std::string mSoundBankRootDirectory;
		std::unordered_map<std::string, uint32_t> additionalPluginHandles;

		AudioEngine();
		void RegisterAdditionalPlugins(const std::vector<std::string>& pluginNames, const std::string& rootPath);

		// Logging
		static FMOD_RESULT F_CALL AudioEngineLogCallback(FMOD_DEBUG_FLAGS flags,
			const char* file, int line, const char* func, const char* message);

};

#endif
