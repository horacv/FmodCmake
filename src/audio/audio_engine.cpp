#include "audio_engine.h"

#include <vector>

#if WIN32
#include <combaseapi.h>
#endif

std::unique_ptr<AudioEngine> AudioEngine::sInstance(nullptr);

AudioEngine::AudioEngine()
: StudioSystem(nullptr)
, bIsMainBankLoaded(false)
{}

AudioEngine& AudioEngine::Get()
{
	if (!sInstance)
	{
		sInstance = std::unique_ptr<AudioEngine>(new AudioEngine());
#if WIN32 // Refer to: https://www.fmod.com/docs/2.03/api/platforms-win.html#com
		(void)CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
#endif
	}
	return *sInstance;
}

bool AudioEngine::Initialize()
{
	if (IsInitialized()) { return true; } // Already Initialized

	AudioEngine& audioEngine = Get();
	if (System::create(&audioEngine.StudioSystem) != FMOD_OK) { return false; }

	constexpr int maxChannels = 256;
	FMOD_STUDIO_INITFLAGS studio_init_flags = FMOD_STUDIO_INIT_NORMAL;
	FMOD_INITFLAGS init_flags = FMOD_INIT_NORMAL;

#ifndef NDEBUG
	studio_init_flags = FMOD_STUDIO_INIT_NORMAL | FMOD_STUDIO_INIT_LIVEUPDATE | FMOD_STUDIO_INIT_MEMORY_TRACKING;
	init_flags = FMOD_INIT_NORMAL | FMOD_INIT_PROFILE_ENABLE | FMOD_INIT_MEMORY_TRACKING | FMOD_INIT_PROFILE_METER_ALL;
#endif

	if (audioEngine.StudioSystem->initialize(maxChannels,studio_init_flags, init_flags,nullptr) != FMOD_OK)
	{
		return false;
	}

	// ADDITIONAL PLUGINS
	// Registering the resonance dynamic library as an additional plugin
	// Add additional third-party libraries here
	audioEngine.RegisterAdditionalPlugins({"resonanceaudio"}, "plugins/fmod");

	// MASTER AND STRINGS BANK
	SetSoundBankRootDirectory("assets/soundbanks/" AUDIO_PLATFORM "/");
	audioEngine.bIsMainBankLoaded = LoadSoundBank("Master.bank") && LoadSoundBank("Master.strings.bank");

	return audioEngine.StudioSystem->isValid() && audioEngine.bIsMainBankLoaded;
}

void AudioEngine::RegisterAdditionalPlugins(const std::vector<std::string>& pluginNames, const std::string& rootPath)
{
	if (!StudioSystem->isValid()) { return; }

	FMOD::System* coreSystem = nullptr;
	StudioSystem->getCoreSystem(&coreSystem);
	coreSystem->setPluginPath(rootPath.c_str());

	for (const auto& additionalPluginName : pluginNames)
	{
		uint32_t additionalPluginHandle = 0;
		if (coreSystem->loadPlugin(additionalPluginName.c_str(), &additionalPluginHandle) == FMOD_OK)
		{
			additionalPluginHandles.emplace(std::pair(additionalPluginName, additionalPluginHandle));
		}
	}
}

void AudioEngine::Terminate()
{
	AudioEngine& audioEngine = Get();
	if (audioEngine.StudioSystem->isValid())
	{
		audioEngine.StudioSystem->release();
		audioEngine.StudioSystem = nullptr;
#if WIN32 // Refer to: https://www.fmod.com/docs/2.03/api/platforms-win.html#com
		CoUninitialize();
#endif
	}
}

void AudioEngine::Update()
{
	if (!IsInitialized()) { return; }
	Get().StudioSystem->update();
}

bool AudioEngine::IsInitialized()
{
	const AudioEngine& audioEngine = Get();
	return audioEngine.StudioSystem && audioEngine.StudioSystem->isValid() && audioEngine.bIsMainBankLoaded;
}

void AudioEngine::SetSoundBankRootDirectory(const std::string& directory)
{
	AudioEngine& audioEngine = Get();
	audioEngine.mSoundBankRootDirectory = directory;
}

bool AudioEngine::LoadSoundBank(const std::string& filePath)
{
	const AudioEngine& audioEngine = Get();
	if (!audioEngine.StudioSystem->isValid()) { return false; }

	const std::string fullBankPath = audioEngine.mSoundBankRootDirectory + filePath;

	FMOD::Studio::Bank* Bank = nullptr;
	const FMOD_RESULT Result = audioEngine.StudioSystem->loadBankFile(fullBankPath.c_str(),
		FMOD_STUDIO_LOAD_BANK_NORMAL, &Bank);

	return Result == FMOD_OK;
}

EventInstance* AudioEngine::PlayAudioEvent(const std::string& studioPath,
	const Audio3DAttributes& audio3dAttributes,
	const bool autoStart,
	const bool autoRelease)
{
	if (!IsInitialized()) { return nullptr; }

	FMOD::Studio::EventDescription* description = nullptr;
	EventInstance* instance = nullptr;

	if (Get().StudioSystem->getEvent(studioPath.c_str(), &description) != FMOD_OK) { return nullptr; }
	if (description->createInstance(&instance) != FMOD_OK) { return nullptr; }
	instance->set3DAttributes(&audio3dAttributes);

	if (autoStart)
	{
		instance->start();
		if (autoRelease)
		{
			instance->release();
		}
	}

	return instance;
}

bool AudioEngine::InstanceStart(EventInstance* audioInstance)
{
	if (!(IsInitialized() && audioInstance && audioInstance->isValid())) { return false; }
	return audioInstance->start() == FMOD_OK;
}

bool AudioEngine::InstanceStop(EventInstance* audioInstance, const bool bAllowFadeOut)
{
	if (!(IsInitialized() && audioInstance && audioInstance->isValid())) { return false; }
	return audioInstance->stop(bAllowFadeOut ? FMOD_STUDIO_STOP_ALLOWFADEOUT : FMOD_STUDIO_STOP_IMMEDIATE) == FMOD_OK;
}

bool AudioEngine::InstanceRelease(EventInstance* audioInstance)
{
	if (!(IsInitialized() && audioInstance && audioInstance->isValid())) { return false; }
	return audioInstance->release() == FMOD_OK;
}

bool AudioEngine::InstanceSetPaused(EventInstance* audioInstance, const bool bPaused)
{
	if (!(IsInitialized() && audioInstance && audioInstance->isValid())) { return false; }
	return audioInstance->setPaused(bPaused) == FMOD_OK;
}

bool AudioEngine::InstanceIsPaused(const EventInstance* audioInstance, bool& outPaused)
{
	if (!(IsInitialized() && audioInstance && audioInstance->isValid())) { return false; }
	return audioInstance->getPaused(&outPaused) == FMOD_OK;
}

bool AudioEngine::SetGlobalParameterByName(const std::string& name,
			const float value, const bool bIgnoreSeekSpeed)
{
	if (!IsInitialized()) { return false; }
	const FMOD_RESULT result = Get().StudioSystem->setParameterByName(name.c_str(), value, bIgnoreSeekSpeed);
	return result == FMOD_OK;
}

bool AudioEngine::SetGlobalParameterByNameWithLabel(const std::string& name,
	const std::string& label, const bool bIgnoreSeekSpeed)
{
	if (!IsInitialized()) { return false; }
	const FMOD_RESULT result = Get().StudioSystem->setParameterByNameWithLabel(name.c_str(), label.c_str(), bIgnoreSeekSpeed);
	return result == FMOD_OK;
}

bool AudioEngine::SetParameterByNameWithLabel(EventInstance* audioInstance,
			const std::string& name, const std::string& label, const bool bIgnoreSeekSpeed)
{
	if (!(audioInstance && audioInstance->isValid() && IsInitialized())) { return false; }
	const FMOD_RESULT result = audioInstance->setParameterByNameWithLabel(name.c_str(), label.c_str(), bIgnoreSeekSpeed);
	return result == FMOD_OK;
}

bool AudioEngine::SetParameterByName(EventInstance* audioInstance,
			const std::string& name, const float value, const bool bIgnoreSeekSpeed)
{
	if (!(audioInstance && audioInstance->isValid() && IsInitialized())) { return false; }

	const FMOD_RESULT result = audioInstance->setParameterByName(name.c_str(), value, bIgnoreSeekSpeed);
	return result == FMOD_OK;
}
