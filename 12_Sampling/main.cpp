#include <GLApp.h>
#include <Image.h>
#include <Vec2.h>
#include <Vec3.h>

#include "SignalGenerator.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <utility>

namespace {
constexpr uint32_t imageWidth = 800;
constexpr uint32_t imageHeight = 600;
constexpr uint32_t samplingImageWidth = imageWidth / 2;
constexpr uint32_t samplingImageHeight = imageHeight / 2;
constexpr uint32_t comparisonImageCount = 4;

enum class SamplingMode {
  Point,
  Regular,
  Jittered,
  Reference
};

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

Vec2 jitteredSampleOffset(const uint32_t sampleX,
                          const uint32_t sampleY,
                          const uint32_t samplesPerAxis) {
  // TODO Task 3:
  // Use Vec2::random() to place one random sample inside the stratum
  // (sampleX, sampleY). Keep the sample inside its own stratum; only the
  // position within that stratum should be random.
  const Vec2 unusedJitterPreview = Vec2::random();
  (void)unusedJitterPreview;
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
                 const float frequencyScale) {
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
        offset = jitteredSampleOffset(sx, sy, samplesPerAxis);
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
}

Image renderSamplingImage(const SamplingMode mode,
                          const int sceneIndex,
                          const float frequencyScale) {
  Image image(samplingImageWidth, samplingImageHeight, 3);

  for (uint32_t y = 0; y < samplingImageHeight; ++y) {
    for (uint32_t x = 0; x < samplingImageWidth; ++x) {
      const Vec3 color = samplePixel(x, y, samplingImageWidth, samplingImageHeight,
                                     mode, sceneIndex, frequencyScale);
      setPixel(image, x, y, color);
    }
  }

  return image;
}

constexpr std::array<SamplingMode, comparisonImageCount> comparisonModes = {
  SamplingMode::Point,
  SamplingMode::Regular,
  SamplingMode::Jittered,
  SamplingMode::Reference
};

const std::array<std::pair<Vec2, Vec2>, comparisonImageCount> imageBounds = {{
  {Vec2{-1.0f,  0.0f}, Vec2{ 0.0f,  1.0f}},
  {Vec2{ 0.0f,  0.0f}, Vec2{ 1.0f,  1.0f}},
  {Vec2{-1.0f, -1.0f}, Vec2{ 0.0f,  0.0f}},
  {Vec2{ 0.0f, -1.0f}, Vec2{ 1.0f,  0.0f}},
}};
}

class MyGLApp : public GLApp {
public:
  std::array<Image, comparisonImageCount> samplingImages{
    Image{samplingImageWidth, samplingImageHeight, 3},
    Image{samplingImageWidth, samplingImageHeight, 3},
    Image{samplingImageWidth, samplingImageHeight, 3},
    Image{samplingImageWidth, samplingImageHeight, 3}
  };
  int sceneIndex{0};
  float frequencyScale{1.0f};
  uint32_t jitterSeed{1};

  MyGLApp() :
  GLApp(imageWidth, imageHeight, 1, "Exercise 12 - Sampling", true, true, true) {
  }

  virtual void init() override {
    renderSamplingImages();
  }

  void renderSamplingImages() {
    staticRand.seed(jitterSeed);

    for (uint32_t imageIndex = 0; imageIndex < comparisonImageCount; ++imageIndex) {
      samplingImages[imageIndex] = renderSamplingImage(comparisonModes[imageIndex],
                                                       sceneIndex,
                                                       frequencyScale);
    }
  }

  virtual void draw() override {
    for (uint32_t imageIndex = 0; imageIndex < comparisonImageCount; ++imageIndex) {
      drawImage(samplingImages[imageIndex],
                imageBounds[imageIndex].first,
                imageBounds[imageIndex].second);
    }
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
        renderSamplingImages();
        break;
      case GLENV_KEY_J:
        ++jitterSeed;
        renderSamplingImages();
        break;
      case GLENV_KEY_UP:
        frequencyScale = std::min(frequencyScale * 1.15f, 3.0f);
        renderSamplingImages();
        break;
      case GLENV_KEY_DOWN:
        frequencyScale = std::max(frequencyScale / 1.15f, 0.35f);
        renderSamplingImages();
        break;
      case GLENV_KEY_R:
        sceneIndex = 0;
        frequencyScale = 1.0f;
        jitterSeed = 1;
        renderSamplingImages();
        break;
    }
  }
};

int main(int argc, char** argv) {
  MyGLApp myApp;
  myApp.run();
  return EXIT_SUCCESS;
}
