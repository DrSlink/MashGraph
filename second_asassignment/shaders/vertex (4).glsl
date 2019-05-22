#version 330

layout(location = 0) in vec3 vertex;
uniform float iTime;

const float OFFSET = 0.5;
const float PARTPI = 2.0 * 3.14159265358979 / 5.0;

void main(void)
{
    float s = sin(iTime + PARTPI * 3.0);
    float c = cos(iTime + PARTPI * 3.0);
    
    float x = vertex.x * c - vertex.y * s;
    float y = vertex.x * s + vertex.y * c;
    float z = vertex.z;
    
    float xo = OFFSET * s;
    float yo = OFFSET * c;
    float zo = 0.0;
    
  gl_Position  = vec4(x / (z + zo - 1.1) / (z + zo - 1.1) + xo, y / (z + zo - 1.1) / (z + zo - 1.1) + yo, z + zo, 1.0);
}
