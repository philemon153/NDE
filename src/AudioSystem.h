#pragma once

#include "minaudio.h"
#include <vector>
#include <string>
#include <atomic>

class AudioSystem {
public:
  AudioSystem();
  ~AudioSystem();

  bool init(const std::string& filePath);
  void cleanup();

  void update();
  float getTime() const;
  void getFFT(std::vector<float>& outBins);

private:
  ma_decoder decoder{};
  ma_device device{};
  bool initialized = false;

  std::vector<float> sampleBuffer;
  std::vector<float> fftBins;

  double currentTime = 0.0;
  std::atomic<bool> isPlaying{false};

  static void dataCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
  void computeLowResFFT(const float* samples, ma_uint32 frameCount);
};