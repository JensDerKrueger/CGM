#include "SignalGenerator.h"

#include <algorithm>
#include <cmath>

namespace {
float smoothStep(const float edge0, const float edge1, const float x) {
  const float t = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
  return t * t * (3.0f - 2.0f * t);
}

float fract(const float x) {
  return x - std::floor(x);
}

Vec3 mix(const Vec3& a, const Vec3& b, const float t) {
  return a * (1.0f - t) + b * t;
}

float thinStripe(const float coordinate, const float frequency, const float width) {
  const float phase = std::abs(fract(coordinate * frequency) - 0.5f);
  return phase < width ? 1.0f : 0.0f;
}

float checker(const Vec2& uv, const float frequency) {
  const int cx = int(std::floor(uv.x * frequency));
  const int cy = int(std::floor(uv.y * frequency));
  return ((cx + cy) & 1) == 0 ? 1.0f : 0.0f;
}
}

Vec3 evaluateContinuousSignal(const Vec2& uv, const int sceneIndex, const float frequencyScale) {
  const Vec2 centered = uv * 2.0f - Vec2{1.0f, 1.0f};
  const Vec3 paper = {0.94f, 0.92f, 0.84f};
  const Vec3 ink = {0.05f, 0.08f, 0.12f};
  const Vec3 blue = {0.10f, 0.23f, 0.72f};
  const Vec3 red = {0.78f, 0.12f, 0.08f};
  const Vec3 green = {0.05f, 0.46f, 0.22f};

  if (sceneIndex == 0) {
    const float radius = centered.length();
    const float diagonal = centered.x * 0.86f + centered.y * 0.52f;
    const float stripes = thinStripe(diagonal + 0.05f, 34.0f * frequencyScale, 0.11f);
    const float rings = thinStripe(radius, 24.0f * frequencyScale, 0.10f);
    const float circle = radius < 0.68f ? 1.0f : 0.0f;
    const float ringMask = radius > 0.22f && radius < 0.72f ? 1.0f : 0.0f;
    const float shape = std::max(stripes * circle, rings * ringMask);
    const Vec3 base = mix(paper, Vec3{0.78f, 0.82f, 0.90f}, smoothStep(-0.9f, 0.9f, centered.y));
    return mix(base, blue, shape);
  }

  if (sceneIndex == 1) {
    const float perspective = 1.0f / std::max(0.18f, 1.15f - uv.y);
    const float freq = 8.0f + 24.0f * frequencyScale * perspective;
    const float board = checker({uv.x * perspective + 0.12f * perspective, uv.y * perspective}, freq);
    const float horizon = smoothStep(0.18f, 0.28f, uv.y);
    const Vec3 nearColor = mix(Vec3{0.08f, 0.08f, 0.08f}, Vec3{0.96f, 0.96f, 0.90f}, board);
    const Vec3 farColor = mix(Vec3{0.72f, 0.77f, 0.84f}, Vec3{0.28f, 0.32f, 0.40f}, board);
    return mix(farColor, nearColor, horizon);
  }

  const float lineA = thinStripe(centered.x * 0.94f - centered.y * 0.34f, 42.0f * frequencyScale, 0.075f);
  const float lineB = thinStripe(centered.x * 0.38f + centered.y * 0.92f, 36.0f * frequencyScale, 0.070f);
  const float discA = (centered - Vec2{-0.38f, 0.18f}).length() < 0.28f ? 1.0f : 0.0f;
  const float discB = (centered - Vec2{0.42f, -0.25f}).length() < 0.22f ? 1.0f : 0.0f;
  Vec3 color = paper;
  color = mix(color, red, lineA);
  color = mix(color, green, lineB);
  color = mix(color, ink, std::max(discA, discB));
  return color;
}
