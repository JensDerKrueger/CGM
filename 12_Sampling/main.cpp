#include <GLApp.h>
#include <GLTexture2D.h>
#include <Image.h>
#include <Vec2.h>
#include <Vec3.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <string>

namespace {
constexpr uint32_t imageWidth = 800;
constexpr uint32_t imageHeight = 600;
constexpr uint32_t panelWidth = imageWidth / 2;
constexpr uint32_t panelHeight = imageHeight / 2;

enum class SamplingMode {
  Point,
  Regular,
  Jittered,
  Reference
};

uint32_t hashBits(uint32_t value) {
  value ^= value >> 16;
  value *= 0x7feb352dU;
  value ^= value >> 15;
  value *= 0x846ca68bU;
  value ^= value >> 16;
  return value;
}

float hash01(const uint32_t x, const uint32_t y, const uint32_t sample, const uint32_t seed) {
  const uint32_t h = hashBits(x * 73856093U ^ y * 19349663U ^ sample * 83492791U ^ seed);
  return float(h & 0x00ffffffU) / float(0x01000000U);
}

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

Vec2 pixelToUV(const uint32_t x,
               const uint32_t y,
               const Vec2& subPixelOffset,
               const uint32_t width,
               const uint32_t height) {
  const float u = (float(x) + subPixelOffset.x) / float(width);
  const float v = (float(y) + subPixelOffset.y) / float(height);
  return {u, v};
}

Vec2 regularSampleOffset(const uint32_t sampleX,
                         const uint32_t sampleY,
                         const uint32_t samplesPerAxis) {
  // TODO Task 1:
  // Return the center of the sub-cell (sampleX, sampleY) inside the current
  // pixel. The returned coordinates are sub-pixel offsets in [0, 1]^2.
  // For example, for a 4x4 grid, sample (0, 0) should be at (0.125, 0.125).
  (void)sampleX;
  (void)sampleY;
  (void)samplesPerAxis;
  return {0.5f, 0.5f};
}

Vec2 jitteredSampleOffset(const uint32_t pixelX,
                          const uint32_t pixelY,
                          const uint32_t sampleX,
                          const uint32_t sampleY,
                          const uint32_t samplesPerAxis,
                          const uint32_t seed) {
  // TODO Task 3:
  // Use hash01 to place one random sample inside the stratum
  // (sampleX, sampleY). Keep the sample inside its own stratum; only the
  // position within that stratum should be random.
  const float unusedJitterPreview = hash01(pixelX, pixelY, sampleX + sampleY, seed);
  (void)unusedJitterPreview;
  (void)seed;
  return regularSampleOffset(sampleX, sampleY, samplesPerAxis);
}

uint32_t samplesPerAxisForMode(const SamplingMode mode) {
  // TODO Task 2:
  // Return the number of samples per axis for each supersampling mode.
  // Regular sampling should use 4x4 samples. Jittered sampling should use
  // 8x8 samples, so the quality increases in reading order. The reference
  // image should use 16x16 samples.
  (void)mode;
  return 4U;
}

Vec3 samplePixel(const uint32_t x,
                 const uint32_t y,
                 const uint32_t width,
                 const uint32_t height,
                 const SamplingMode mode,
                 const int sceneIndex,
                 const float frequencyScale,
                 const uint32_t seed) {
  if (mode == SamplingMode::Point) {
    const Vec2 uv = pixelToUV(x, y, {0.5f, 0.5f}, width, height);
    return evaluateContinuousSignal(uv, sceneIndex, frequencyScale);
  }

  const uint32_t samplesPerAxis = samplesPerAxisForMode(mode);
  Vec3 color = {0.0f, 0.0f, 0.0f};

  for (uint32_t sy = 0; sy < samplesPerAxis; ++sy) {
    for (uint32_t sx = 0; sx < samplesPerAxis; ++sx) {
      Vec2 offset = regularSampleOffset(sx, sy, samplesPerAxis);

      if (mode == SamplingMode::Jittered || mode == SamplingMode::Reference) {
        // TODO Task 3:
        // Use a jittered stratified sample instead of the regular grid sample.
        offset = jitteredSampleOffset(x, y, sx, sy, samplesPerAxis, seed);
      }

      const Vec2 uv = pixelToUV(x, y, offset, width, height);
      color = color + evaluateContinuousSignal(uv, sceneIndex, frequencyScale);
    }
  }

  const float sampleCount = float(samplesPerAxis * samplesPerAxis);
  return color / sampleCount;
}

void setPixel(Image& image, const uint32_t x, const uint32_t y, const Vec3& color) {
  image.setNormalizedValue(x, y, 0, std::clamp(color.r, 0.0f, 1.0f));
  image.setNormalizedValue(x, y, 1, std::clamp(color.g, 0.0f, 1.0f));
  image.setNormalizedValue(x, y, 2, std::clamp(color.b, 0.0f, 1.0f));
  image.setNormalizedValue(x, y, 3, 1.0f);
}

Vec3 panelBorderColor(const uint32_t x, const uint32_t y) {
  const bool border = x == 0 || y == 0 || x == panelWidth - 1 || y == panelHeight - 1;
  return border ? Vec3{0.85f, 0.85f, 0.85f} : Vec3{-1.0f, -1.0f, -1.0f};
}

Image renderSamplingComparison(const int sceneIndex,
                               const float frequencyScale,
                               const uint32_t seed) {
  Image image(imageWidth, imageHeight, 4);
  const std::array<SamplingMode, 4> modes = {
    SamplingMode::Point,
    SamplingMode::Regular,
    SamplingMode::Jittered,
    SamplingMode::Reference
  };

  for (uint32_t y = 0; y < imageHeight; ++y) {
    for (uint32_t x = 0; x < imageWidth; ++x) {
      const uint32_t panelX = x / panelWidth;
      const uint32_t panelY = y / panelHeight;
      const uint32_t localX = x % panelWidth;
      const uint32_t localY = y % panelHeight;
      const uint32_t modeIndex = panelY * 2U + panelX;

      const Vec3 borderColor = panelBorderColor(localX, localY);
      if (borderColor.r >= 0.0f) {
        setPixel(image, x, y, borderColor);
      } else {
        const Vec3 color = samplePixel(localX, localY, panelWidth, panelHeight,
                                       modes[modeIndex], sceneIndex,
                                       frequencyScale, seed);
        setPixel(image, x, y, color);
      }
    }
  }

  return image;
}
}

class MyGLApp : public GLApp {
public:
  GLTexture2D samplingTexture{GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE};
  int sceneIndex{0};
  float frequencyScale{1.0f};
  uint32_t seed{1};
  bool imageDirty{true};

  MyGLApp() :
  GLApp(imageWidth, imageHeight, 1, "Exercise 12 - Sampling", true, true, true) {
  }

  virtual void init() override {
    setBackground(0.02f, 0.02f, 0.025f, 1.0f);
  }

  void updateImageIfNeeded() {
    if (!imageDirty) {
      return;
    }

    Image image = renderSamplingComparison(sceneIndex, frequencyScale, seed);
    samplingTexture.setData(image);
    imageDirty = false;
  }

  virtual void draw() override {
    updateImageIfNeeded();
    GL(glDisable(GL_DEPTH_TEST));
    drawImage(samplingTexture);
  }

  virtual void keyboard(int key, int scancode, int action, int mods) override {
    if (action != GLENV_PRESS) {
      return;
    }

    switch (key) {
      case GLENV_KEY_ESCAPE:
        closeWindow();
        break;
      case GLENV_KEY_S:
        sceneIndex = (sceneIndex + 1) % 3;
        imageDirty = true;
        break;
      case GLENV_KEY_J:
        ++seed;
        imageDirty = true;
        break;
      case GLENV_KEY_UP:
        frequencyScale = std::min(frequencyScale * 1.15f, 3.0f);
        imageDirty = true;
        break;
      case GLENV_KEY_DOWN:
        frequencyScale = std::max(frequencyScale / 1.15f, 0.35f);
        imageDirty = true;
        break;
      case GLENV_KEY_R:
        sceneIndex = 0;
        frequencyScale = 1.0f;
        seed = 1;
        imageDirty = true;
        break;
    }
  }
};

int main(int argc, char** argv) {
  MyGLApp myApp;
  myApp.run();
  return EXIT_SUCCESS;
}
