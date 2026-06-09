#include "Effect.h"

void Effect::update(float time, const std::vector<float>& fft)
{
  float rawBass = fft.empty() ? 0.0f : fft[0];

  smoothedBass = smoothedBass * 0.85f + rawBass * 0.15f;
  float delta = rawBass - smoothedBass;

  if (delta > 0.4f) bassTrigger = 1.0f;           // Strong kick detected
  else bassTrigger *= 0.85f;                      // Decay

  uniforms.bass = bassTrigger;
  uniforms.intensity = 0.6f + bassTrigger * 2.8f;
  uniforms.time = time;
}
