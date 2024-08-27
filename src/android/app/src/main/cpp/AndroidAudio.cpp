#include "AndroidAudio.h"

#include "Cafe/OS/libs/snd_core/ax.h"
#include "audio/IAudioAPI.h"

#if HAS_CUBEB
#include "audio/CubebAPI.h"
#endif // HAS_CUBEB

namespace AndroidAudio
{
    // 假设这些组件已经在其他地方定义和初始化
    extern std::mutex g_audioMutex;
    extern std::shared_ptr<IAudioDevice> g_tvAudio;
    extern std::shared_ptr<IAudioDevice> g_padAudio;

    void createAudioDevice(IAudioAPI::AudioAPI audioApi, sint32 channels, sint32 volume, bool isTV)
    {
        static constexpr int AX_FRAMES_PER_GROUP = 4;
        std::unique_lock<std::mutex> lock(g_audioMutex);

        auto& audioDevice = isTV ? g_tvAudio : g_padAudio;

        // 处理通道数映射
        switch (channels)
        {
        case 0:
            channels = 1;
            break;
        case 2:
            channels = 6;
            break;
        default:
            if (channels < 1 || channels > 6)
            {
                cemuLog_log(LogType::Warning, "Unsupported number of channels: {}, defaulting to 2", channels);
                channels = 2;
            }
            break;
        }

        // 创建音频设备
        switch (audioApi)
        {
#if HAS_CUBEB
        case IAudioAPI::AudioAPI::Cubeb:
        {
            audioDevice.reset();
            auto deviceDescriptionPtr = std::make_shared<CubebAPI::CubebDeviceDescription>(nullptr, std::string(), std::wstring());
            audioDevice = IAudioAPI::CreateDevice(IAudioAPI::AudioAPI::Cubeb, deviceDescriptionPtr, 48000, channels, snd_core::AX_SAMPLES_PER_3MS_48KHZ * AX_FRAMES_PER_GROUP, 16);
            if (audioDevice)
            {
                audioDevice->SetVolume(volume);
            }
            else
            {
                cemuLog_log(LogType::Error, "Failed to create Cubeb audio device.");
            }
            break;
        }
#endif // HAS_CUBEB
        default:
            cemuLog_log(LogType::Error, "Invalid or unsupported audio API: {}", audioApi);
            break;
        }
    }

    void setAudioVolume(sint32 volume, bool isTV)
    {
        std::shared_lock<std::mutex> lock(g_audioMutex);
        auto& audioDevice = isTV ? g_tvAudio : g_padAudio;
        if (audioDevice)
        {
            audioDevice->SetVolume(volume);
        }
        else
        {
            cemuLog_log(LogType::Warning, "Audio device is not initialized.");
        }
    }
}; // namespace AndroidAudio
