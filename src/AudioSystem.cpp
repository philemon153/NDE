#define MINIAUDIO_IMPLEMENTATION
#include "AudioSystem.h"
#include <iostream>
#include <algorithm>
#include <cmath>

AudioSystem::AudioSystem() = default;

AudioSystem::~AudioSystem()
{
    cleanup();
}

bool AudioSystem::init(const std::string& filePath)
{
    cleanup();

    ma_decoder_config decoderConfig = ma_decoder_config_init(ma_format_f32, 2, 44100);
    if (ma_decoder_init_file(filePath.c_str(), &decoderConfig, &decoder) != MA_SUCCESS) {
        std::cerr << "AudioSystem: Failed to load " << filePath << std::endl;
        return false;
    }

    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format = ma_format_f32;
    deviceConfig.playback.channels = 2;
    deviceConfig.sampleRate = 44100;
    deviceConfig.dataCallback = dataCallback;
    deviceConfig.pUserData = this;

    if (ma_device_init(nullptr, &deviceConfig, &device) != MA_SUCCESS) {
        std::cerr << "AudioSystem: Failed to init audio device\n";
        ma_decoder_uninit(&decoder);
        return false;
    }

    sampleBuffer.resize(4096);
    fftBins.resize(64, 0.0f);

    if (ma_device_start(&device) != MA_SUCCESS) {
        std::cerr << "AudioSystem: Failed to start playback\n";
        return false;
    }

    initialized = true;
    std::cout << "AudioSystem: Now playing " << filePath << std::endl;
    return true;
}

void AudioSystem::cleanup()
{
    if (initialized) {
        ma_device_stop(&device);
        ma_device_uninit(&device);
        ma_decoder_uninit(&decoder);
        initialized = false;
    }
}

void AudioSystem::dataCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    AudioSystem* self = static_cast<AudioSystem*>(pDevice->pUserData);
    if (!self) return;

    ma_uint64 framesRead = 0;
    ma_decoder_read_pcm_frames(&self->decoder, pOutput, frameCount, &framesRead);

    if (framesRead < frameCount) {
        ma_decoder_seek_to_pcm_frame(&self->decoder, 0);
    }

    self->computeLowResFFT(static_cast<const float*>(pOutput), framesRead);
}

void AudioSystem::update()
{
    if (initialized) {
        ma_uint64 cursor = 0;
        ma_decoder_get_cursor_in_pcm_frames(&decoder, &cursor);
        currentTime = static_cast<double>(cursor) / decoder.outputSampleRate;
    }
}

float AudioSystem::getTime() const
{
    return static_cast<float>(currentTime);
}

void AudioSystem::getFFT(std::vector<float>& outBins)
{
    outBins = fftBins;
    if (outBins.size() != 64) outBins.resize(64, 0.0f);
}

void AudioSystem::computeLowResFFT(const float* samples, ma_uint32 frameCount)
{
    const int fftSize = 512;
    const int numBins = 64;

    if (frameCount < 64) return;

    std::vector<float> real(fftSize, 0.0f);

    for (int i = 0; i < fftSize && i < (int)frameCount; ++i) {
        float window = 0.5f * (1.0f - std::cos(2.0f * 3.14159265359f * i / (fftSize - 1)));
        real[i] = samples[i * 2] * window;   // left channel
    }

    for (int k = 0; k < numBins; ++k) {
        float sumReal = 0.0f, sumImag = 0.0f;
        for (int i = 0; i < fftSize; ++i) {
            float angle = 2.0f * 3.14159265359f * k * i / fftSize;
            sumReal += real[i] * std::cos(angle);
            sumImag += real[i] * std::sin(angle);
        }
        fftBins[k] = std::sqrt(sumReal*sumReal + sumImag*sumImag) / fftSize * 3.0f;
    }

    float maxVal = 0.0f;
    for (float v : fftBins) maxVal = std::max(maxVal, v);
    if (maxVal > 0.001f) {
        for (float& v : fftBins) v /= maxVal;
    }
    if (!fftBins.empty()) fftBins[0] *= 1.8f;
}