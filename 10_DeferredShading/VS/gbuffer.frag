in vec3 positionViewSpace;
in vec3 normalViewSpace;
in vec3 tangentViewSpace;
in vec3 binormalViewSpace;
in vec2 texCoords;

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D normalTexture;

uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float shininess;
uniform int useDiffuseTexture;
uniform int useSpecularTexture;
uniform int useNormalTexture;

layout(location = 0) out vec4 outDiffuse;
layout(location = 1) out vec4 outSpecular;
layout(location = 2) out vec4 outNormal;
layout(location = 3) out vec4 outPosition;

void main() {
  // TODO Task 2:
  // Compute the diffuse color from materialDiffuse and, if requested, the
  // diffuse texture. Do the same for the specular color. Compute a view-space
  // normal from normalViewSpace and optionally perturb it with the normal map
  // using the tangent, binormal, normal basis.
  //
  // Write the four G-buffer outputs:
  //   outDiffuse  = diffuse color
  //   outSpecular = specular color, shininess in alpha
  //   outNormal   = view-space normal mapped from [-1, 1] to [0, 1]
  //   outPosition = view-space position, alpha 1 for covered pixels

  vec3 diffuse = materialDiffuse;
  vec3 specular = materialSpecular;
  vec3 N = normalize(normalViewSpace);

  outDiffuse = vec4(diffuse, 1.0);
  outSpecular = vec4(specular, clamp(shininess / 128.0, 0.0, 1.0));
  outNormal = vec4(N * 0.5 + vec3(0.5), 1.0);
  outPosition = vec4(positionViewSpace, 1.0);
}
