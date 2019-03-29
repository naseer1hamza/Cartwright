uniform vec3 u_LightPos;
uniform vec3 u_EyePosition;
uniform float u_Shininess;
uniform sampler2D textureSampler;
varying vec2 v_TextureCoords;
varying vec3 v_Position;
varying vec3 v_Normal;

void main() {
   float distance = length(u_LightPos - v_Position);

   vec3 lightVector_viewspace = normalize(u_LightPos - v_Position);
   vec3 normal = normalize(v_Normal);
   vec4 baseColour = vec4(texture(textureSampler, v_TextureCoords).rgb, 1.0);

   // ambient
   vec4 ambientColour = vec4(0.1, 0.1, 0.1, 1.0) * baseColour;

   // diffuse
   float diffuse = clamp(dot(normal, lightVector_viewspace), 0, 1);

   // attenuate
   diffuse = diffuse * (1.0 / (1.0 + (0.00025 * distance * distance)));

   // specular
   vec3 incidenceVector = -lightVector_viewspace;
   vec3 reflectionVector = reflect(incidenceVector, normal);
   vec3 eyeVector = normalize(u_EyePosition - v_Position);
   float cosAngle = clamp(dot(eyeVector, reflectionVector), 0, 1);
   float specular = pow(cosAngle, u_Shininess);

   gl_FragColor = baseColour * specular +
                  baseColour * diffuse +
                  ambientColour;
}
