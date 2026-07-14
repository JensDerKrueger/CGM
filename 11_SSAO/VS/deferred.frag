in vec2 texCoords;

uniform sampler2D diffuseBuffer;
uniform sampler2D specularBuffer;
uniform sampler2D normalBuffer;
uniform sampler2D positionBuffer;
uniform sampler2D ambientOcclusionBuffer;
uniform vec4 lightPosition;
uniform int showDebugBuffers;
uniform int showAmbientOcclusionBuffer;

out vec4 color;

vec3 shadePixel(vec2 uv) {
  vec4 positionSample = texture(positionBuffer, uv);
  if(positionSample.a <= 0.0) return vec3(0.0);

  vec3 position = positionSample.xyz;
  vec3 diffuse = texture(diffuseBuffer, uv).rgb;
  // TODO Task 4:
  // Read the SSAO value from ambientOcclusionBuffer. The SSAO pass writes 1 for
  // open areas and lower values for locally occluded regions. Use a small power
  // curve, for example pow(ao, 2.2), to make contact shadows more visible.
  float ambientOcclusion = 1.0;
  vec4 specularSample = texture(specularBuffer, uv);
  vec3 specular = specularSample.rgb;
  float shininess = max(1.0, specularSample.a * 128.0);
  vec3 N = normalize(texture(normalBuffer, uv).rgb * 2.0 - vec3(1.0));

  vec3 L = normalize(lightPosition.xyz - position);
  vec3 V = normalize(-position);
  vec3 R = reflect(-L, N);

  float diffuseWeight = max(dot(N, L), 0.0);
  float specularWeight = 0.0;
  if(diffuseWeight > 0.0) {
    specularWeight = 0.35 * pow(max(dot(V, R), 0.0), shininess);
  }

  // TODO Task 4:
  // Use ambientOcclusion to attenuate the ambient term. You can also use it as a
  // soft contact term for direct lighting by mapping it with mix(0.25, 1.0, ao).
  // This is not physically exact, but it makes the local SSAO effect visible.
  vec3 ambient = 0.14 * diffuse;
  float contactOcclusion = 1.0;
  vec3 directLight = contactOcclusion * (diffuseWeight * diffuse + specularWeight * specular);
  return ambient + directLight;
}

vec3 showBuffer(vec2 uv) {
  bool right = texCoords.x >= 0.5;
  bool top = texCoords.y >= 0.5;
  vec2 localUV = fract(texCoords * 2.0);

  if(!right && top) {
    return texture(diffuseBuffer, localUV).rgb;
  }

  if(right && top) {
    return texture(specularBuffer, localUV).rgb;
  }

  if(!right && !top) {
    return texture(normalBuffer, localUV).rgb;
  }

  vec3 position = texture(positionBuffer, localUV).xyz;
  return vec3(position.xy * 0.01 + vec2(0.5), clamp(-position.z / 160.0, 0.0, 1.0));
}

void main() {
  if(showAmbientOcclusionBuffer != 0) {
    color = vec4(vec3(texture(ambientOcclusionBuffer, texCoords).r), 1.0);
  } else if(showDebugBuffers != 0) {
    color = vec4(showBuffer(texCoords), 1.0);
  } else {
    color = vec4(shadePixel(texCoords), 1.0);
  }
}
