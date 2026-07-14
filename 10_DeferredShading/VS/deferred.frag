in vec2 texCoords;

uniform sampler2D diffuseBuffer;
uniform sampler2D specularBuffer;
uniform sampler2D normalBuffer;
uniform sampler2D positionBuffer;
uniform vec4 lightPosition;
uniform int showDebugBuffers;

out vec4 color;

vec3 shadePixel(vec2 uv) {
  // TODO Task 3:
  // Read position, diffuse color, specular color, shininess, and normal from
  // the G-buffer. Empty background pixels have position alpha 0 and should
  // return black. Reconstruct the view-space normal from the encoded normal
  // buffer and compute Phong illumination in image space.
  return vec3(uv, 0.15);
}

vec3 showBuffer(vec2 uv) {
  // TODO Task 4:
  // Split the screen into four quadrants and show the four G-buffer textures.
  // Use local coordinates inside each quadrant, for example with
  // fract(texCoords * 2.0). A useful layout is:
  //   top left:     diffuse
  //   top right:    specular
  //   bottom left:  normal
  //   bottom right: position, remapped into a visible color range
  return vec3(uv, 0.0);
}

void main() {
  if(showDebugBuffers != 0) {
    color = vec4(showBuffer(texCoords), 1.0);
  } else {
    color = vec4(shadePixel(texCoords), 1.0);
  }
}
