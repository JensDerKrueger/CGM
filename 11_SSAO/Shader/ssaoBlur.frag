in vec2 texCoords;

uniform sampler2D sourceAmbientOcclusionBuffer;
uniform sampler2D positionBuffer;
uniform vec2 direction;
uniform float depthFalloff;

out vec4 color;

float gaussianWeight(vec2 offset) {
  float distanceSquared = dot(offset, offset);
  return exp(-0.5 * distanceSquared);
}

float smoothAmbientOcclusion(vec2 uv) {
  vec4 centerPosition = texture(positionBuffer, uv);
  if(centerPosition.a <= 0.0) return 1.0;

  // TODO Task 3:
  // Smooth the ambient occlusion texture with one pass of a separable,
  // edge-aware blur.
  //
  // 1. Compute the texture-space size of one texel with textureSize.
  // 2. Loop over five samples from -2 to 2 along direction.
  // 3. For each neighbor, read its position. Skip background pixels.
  // 4. Compute a spatial Gaussian weight from the image-space offset.
  // 5. Compute a depth weight from the absolute difference between the center
  //    view-space z value and the neighbor z value. Use depthFalloff to control
  //    how quickly this weight decreases.
  // 6. Accumulate weighted AO values from sourceAmbientOcclusionBuffer.
  // 7. Divide by the total weight and return the smoothed AO value.
  //
  // The Gaussian part is separable: running this shader once horizontally and
  // once vertically gives a 5 x 5 blur-like result with 10 samples instead of 25.
  // The depth weighting keeps the blur from smearing across depth edges.
  return texture(sourceAmbientOcclusionBuffer, uv).r;
}

void main() {
  color = vec4(vec3(smoothAmbientOcclusion(texCoords)), 1.0);
}
