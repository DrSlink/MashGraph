#version 330

layout(location = 0) in vec3 vertex;
uniform float iTime;

const float OFFSET1 = 0.5;
const float OFFSET2 = 0.7;
const float OFFSET3 = 0.3;

void main(void)
{
    float s = sin(2.0*iTime);
    float c = cos(2.0*iTime);
    
    float x = vertex.x * c - vertex.z * s;
    float y = vertex.y;
    float z = vertex.x * s + vertex.z * c;
    
    float xo = -OFFSET2;
    float yo = OFFSET1 * s;
    float zo = OFFSET3 * c;
    
  gl_Position  = vec4(x / (z + zo - 1.1) / (z + zo - 1.1) + xo, y / (z + zo - 1.1) / (z + zo - 1.1) + yo, z + zo, 1.0);
}
