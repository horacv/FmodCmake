#include "audio_engine.h"

#include <chrono>
#include <iostream>
#include <ostream>
#include <vector>

#if WIN32
#include <combaseapi.h>
#endif

std::unique_ptr<AudioEngine> AudioEngine::sInstance(nullptr);

AudioEngine::AudioEngine()
: StudioSystem(nullptr)
, bMainBanksLoaded(false)
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
	if (StudioSystem::create(&audioEngine.StudioSystem) != FMOD_OK) { return false; }

	AudioConfig config;
	if (!config.LoadConfigFile("config/audio_engine.ini")) { return false; }

	CoreSystem* coreSystem = nullptr;
	if (audioEngine.StudioSystem->getCoreSystem(&coreSystem) != FMOD_OK) { return false; }

	std::unordered_map<std::string, FMOD_SPEAKERMODE> speakerModes{
		{"Stereo", FMOD_SPEAKERMODE_STEREO},
		{"5.1", FMOD_SPEAKERMODE_5POINT1},
		{"7.1", FMOD_SPEAKERMODE_7POINT1},
		{"7.1.4", FMOD_SPEAKERMODE_7POINT1POINT4}
	};

	FMOD_SPEAKERMODE outputFormat = FMOD_SPEAKERMODE_STEREO;
	{
		auto it = speakerModes.find(config.GetString("System", "OutputFormat"));
		if (it != speakerModes.end())
		{
			outputFormat = it->second;
		}
	}

	const int maxChannelCount = config.GetInt("System", "MaxChannelCount", 128);
	const int realChannelCount = config.GetInt("Advanced", "RealChannelCount", 64);
	const int sampleRate = config.GetInt("System", "SampleRate");
	const int dspBufferLength = config.GetInt("System", "DSPBufferLength");
	const int dspBufferCount = config.GetInt("System", "DSPBufferCount");

	coreSystem->setSoftwareChannels(realChannelCount);
	coreSystem->setDSPBufferSize(dspBufferLength, dspBufferCount);
	coreSystem->setSoftwareFormat(sampleRate, outputFormat, 0);

	FMOD_STUDIO_INITFLAGS studio_init_flags = FMOD_STUDIO_INIT_NORMAL;
	FMOD_INITFLAGS init_flags = FMOD_INIT_NORMAL;

#ifndef NDEBUG
	if (config.GetBool("System", "EnableLiveUpdate")) { studio_init_flags |= FMOD_STUDIO_INIT_LIVEUPDATE; }
	if (config.GetBool("System", "EnableMemoryTracking")) { studio_init_flags |= FMOD_STUDIO_INIT_MEMORY_TRACKING; }
#endif

	std::unordered_map<std::string, FMOD_DEBUG_FLAGS> loggingLevels{
		{"None", FMOD_DEBUG_LEVEL_NONE},
		{"Log", FMOD_DEBUG_LEVEL_LOG},
		{"Warning", FMOD_DEBUG_LEVEL_WARNING},
		{"Error", FMOD_DEBUG_LEVEL_ERROR}
	};

	FMOD_DEBUG_FLAGS loggingLevel = FMOD_DEBUG_LEVEL_NONE;
	{
		auto it = loggingLevels.find(config.GetString("System", "LoggingLevel"));
		if (it != loggingLevels.end()) { loggingLevel = it->second; }
	}

#ifndef NDEBUG // Logging only available in the Debug config (fmodstudioL and fmodL dynamic libs)
	FMOD::Debug_Initialize(loggingLevel, FMOD_DEBUG_MODE_CALLBACK, AudioEngineLogCallback);
#endif

	std::string bankKey = config.GetString("Advanced", "StudioBankKey");

	FMOD_STUDIO_ADVANCEDSETTINGS studioAdvancedSettings = {};
	studioAdvancedSettings.cbsize = sizeof(FMOD_STUDIO_ADVANCEDSETTINGS);
	studioAdvancedSettings.studioupdateperiod = config.GetInt("Advanced", "StudioUpdatePeriodMs");
	if (!bankKey.empty())
	{
		studioAdvancedSettings.encryptionkey = bankKey.c_str();
	}

	FMOD_ADVANCEDSETTINGS coreAdvancedSettings = {};
	coreAdvancedSettings.cbSize = sizeof(FMOD_ADVANCEDSETTINGS);
	coreAdvancedSettings.vol0virtualvol = config.GetFloat("Advanced", "Vol0VirtualLevel");
	coreAdvancedSettings.profilePort = config.GetInt("Advanced", "LiveUpdatePort");

	if (coreSystem->setAdvancedSettings(&coreAdvancedSettings) != FMOD_OK) { return false; }

	if (audioEngine.StudioSystem->initialize(maxChannelCount,studio_init_flags, init_flags,nullptr) != FMOD_OK)
	{
		return false;
	}

	// ADDITIONAL PLUGINS
	// Registering the resonance dynamic library as an additional plugin
	// Add additional third-party libraries here
	const std::vector<std::string> additionalPlugins = config.GetStringArray("Plugins", "AdditionalPlugins");
	const std::string additionalPluginsRootPath = config.GetString("Plugins", "AdditionalPluginsRootPath");
	audioEngine.RegisterAdditionalPlugins(additionalPlugins, additionalPluginsRootPath);

	// MASTER AND STRINGS BANK
	const std::string bankOutputDirectory = config.GetString("Banks", "BankOutputDirectory") + "/" + AUDIO_PLATFORM + "/";
	SetSoundBankRootDirectory(bankOutputDirectory);

	const bool bIsMainBankLoaded = LoadSoundBankFile(config.GetString("Banks", "MasterBank"));
	const bool bIsStringsBankLoaded = LoadSoundBankFile(config.GetString("Banks", "MasterStringsBank"));

	audioEngine.bMainBanksLoaded = bIsMainBankLoaded && bIsStringsBankLoaded;

	return audioEngine.StudioSystem->isValid() && audioEngine.bMainBanksLoaded;
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
	return audioEngine.StudioSystem && audioEngine.StudioSystem->isValid() && audioEngine.bMainBanksLoaded;
}

void AudioEngine::SetSoundBankRootDirectory(const std::string& directory)
{
	Get().mSoundBankRootDirectory = directory;
}

bool AudioEngine::LoadSoundBankFile(const std::string& filePath)
{
	AudioBank* bank;
	return LoadSoundBankFile(filePath, bank);
}

bool AudioEngine::LoadSoundBankFile(const std::string& filePath, AudioBank*& outBankPtr)
{
	const AudioEngine& audioEngine = Get();
	if (!audioEngine.StudioSystem->isValid()) { return false; }

	const std::string fullBankPath = audioEngine.mSoundBankRootDirectory + filePath;

	return audioEngine.StudioSystem->loadBankFile(fullBankPath.c_str(),
		FMOD_STUDIO_LOAD_BANK_NORMAL, &outBankPtr) == FMOD_OK;
}

bool AudioEngine::UnloadSoundBank(const std::string& studioPath)
{
	const AudioEngine& audioEngine = Get();
	if (!audioEngine.StudioSystem->isValid()) { return false; }

	FMOD::Studio::Bank* bank = nullptr;
	if (audioEngine.StudioSystem->getBank(studioPath.c_str(), &bank) == FMOD_OK)
	{
		return UnloadSoundBank(bank);
	}

	return false;
}

bool AudioEngine::UnloadSoundBank(AudioBank* bank)
{
	if (!(Get().StudioSystem->isValid() && bank)) { return false; }
	return bank->unload() == FMOD_OK;
}

AudioInstance* AudioEngine::PlayAudioEvent(const std::string& studioPath, const Audio3DAttributes& audio3dAttributes,
	void* userData, const AudioEventCallback callback, const AudioCallbackType callbackType, const bool autoStart, const bool autoRelease)
{
	if (!IsInitialized()) { return nullptr; }

	FMOD::Studio::EventDescription* description = nullptr;
	AudioInstance* instance = nullptr;

	if (Get().StudioSystem->getEvent(studioPath.c_str(), &description) != FMOD_OK) { return nullptr; }
	if (description->createInstance(&instance) != FMOD_OK) { return nullptr; }

	instance->set3DAttributes(&audio3dAttributes);

	if (callback)
	{
		instance->setCallback(callback, callbackType);
	}

	if (userData)
	{
		instance->setUserData(userData);
	}

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

bool AudioEngine::InstanceStart(AudioInstance* instance)
{
	if (!(IsInitialized() && instance && instance->isValid())) { return false; }
	return instance->start() == FMOD_OK;
}

bool AudioEngine::InstanceStop(AudioInstance* instance, const bool bAllowFadeOut)
{
	if (!(IsInitialized() && instance && instance->isValid())) { return false; }
	return instance->stop(bAllowFadeOut ? FMOD_STUDIO_STOP_ALLOWFADEOUT : FMOD_STUDIO_STOP_IMMEDIATE) == FMOD_OK;
}

bool AudioEngine::InstanceRelease(AudioInstance* instance)
{
	if (!(IsInitialized() && instance && instance->isValid())) { return false; }
	return instance->release() == FMOD_OK;
}

bool AudioEngine::InstanceSetPaused(AudioInstance* instance, const bool bPaused)
{
	if (!(IsInitialized() && instance && instance->isValid())) { return false; }
	return instance->setPaused(bPaused) == FMOD_OK;
}

bool AudioEngine::InstanceIsPaused(const AudioInstance* instance, bool& outPaused)
{
	if (!(IsInitialized() && instance && instance->isValid())) { return false; }
	return instance->getPaused(&outPaused) == FMOD_OK;
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

bool AudioEngine::SetParameterByNameWithLabel(AudioInstance* instance,
			const std::string& name, const std::string& label, const bool bIgnoreSeekSpeed)
{
	if (!(instance && instance->isValid() && IsInitialized())) { return false; }
	const FMOD_RESULT result = instance->setParameterByNameWithLabel(name.c_str(), label.c_str(), bIgnoreSeekSpeed);
	return result == FMOD_OK;
}

bool AudioEngine::SetParameterByName(AudioInstance* instance,
			const std::string& name, const float value, const bool bIgnoreSeekSpeed)
{
	if (!(instance && instance->isValid() && IsInitialized())) { return false; }

	const FMOD_RESULT result = instance->setParameterByName(name.c_str(), value, bIgnoreSeekSpeed);
	return result == FMOD_OK;
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

FMOD_RESULT AudioEngine::AudioEngineLogCallback(const FMOD_DEBUG_FLAGS flags,
	const char* file, int line, const char* func, const char* message)
{
	const auto now = std::chrono::system_clock::now();
	const auto time64 = std::chrono::system_clock::to_time_t(now);

	std::tm localTime {};
#ifdef WIN32
	localtime_s(&localTime, &time64);   // Windows (safe version)
#else
	localtime_r(&time64, &localTime);   // POSIX (safe version on Linux/macOS)
#endif

	std::ostringstream stringStream;
	stringStream << std::put_time(&localTime, "%d-%b-%Y %H:%M:%S");

	std::unordered_map<FMOD_DEBUG_FLAGS, std::string> loggingLevels{
			{FMOD_DEBUG_LEVEL_LOG, "Log"},
			{FMOD_DEBUG_LEVEL_WARNING, "Warning"},
			{FMOD_DEBUG_LEVEL_ERROR, "Error"}
	};

	std::string loggingLevel;
	const auto it = loggingLevels.find(flags);
	if (it != loggingLevels.end()) { loggingLevel = it->second; }

	std::cout << "FMOD " << loggingLevel << " [" + stringStream.str() + "] " + message << std::endl;
	return FMOD_OK;
}