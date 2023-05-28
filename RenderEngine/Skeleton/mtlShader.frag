#version 330 core

// Interpolated values from the vertex shaders
// e.g.
in vec2 UV;
in vec3 pos_worldspace;
in vec3 normal_cameraspace; //normal in camera space
in vec3 eyeDirection_cameraspace; // eye direction in camera space
in vec3 lightDirection_cameraspace; // light direction in camera space

// Ouput data
out vec4 color;

// Values that stay constant for the whole mesh.
uniform sampler2D myTextureSampler;
uniform float opacity;  // HOW TO USE?
uniform vec4 ambientColor;
uniform vec4 diffuseColor;
uniform vec4 specularColor;

uniform mat4 MV;
uniform vec3 lightPos_worldspace;

const float ns = 6.0; //specular exponent

const vec4 ambientLightColor = vec4(0.4,0.4,0.4,1.0);
const vec4 diffuseLightColor = vec4(1.0,1.0,1.0,1.0);
const vec4 specularLightColor = vec4(1.0,1.0,1.0,1.0);

void main(){
		
  // Material properties
  vec4 textureVal = texture( myTextureSampler, UV ).rgba;
  
  // Normal of the computed fragment, in camera space
  vec3 N = normalize( normal_cameraspace );
  // Direction of the light (from the fragment to the light) in camera space
  vec3 L = normalize( lightDirection_cameraspace );

  float cosTheta = clamp( dot( N,L), 0,1 );

  // Eye vector (towards the camera)
  vec3 E = normalize(eyeDirection_cameraspace);
  // Direction in which the triangle reflects the light

  // blinn-phong reflection
  vec3 H = normalize(L + N);  
  float cosAlpha = clamp(dot(N, H), 0, 1);

  vec4 diffuseComponent = diffuseLightColor* diffuseColor * textureVal * cosTheta;
  vec4 ambientComponent = ambientLightColor * ambientColor * textureVal; //for simplification we reuse the diffuse texture map for the ambient texture map
  vec4 specularComponent = specularLightColor * specularColor * pow(cosAlpha,ns);
  
  color =
  // Ambient : simulates indirect lighting
  (ambientComponent +
  // Diffuse : "color" of the object
  diffuseComponent +
  // Specular : reflective highlight, like a mirror
  specularComponent) *  opacity;
}
