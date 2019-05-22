#version 330

#define float2 vec2
#define float3 vec3
#define float4 vec4
#define float4x4 mat4
#define float3x3 mat3

in float2 fragmentTexCoord;

layout(location = 0) out vec4 fragColor;

uniform int g_screenWidth;
uniform int g_screenHeight;

uniform float3 g_bBoxMin   = float3(-1, -1, -1);
uniform float3 g_bBoxMax   = float3(1, 1, 1);

uniform float4x4 g_rayMatrix;

uniform float4   g_bgColor = float4(0, 0, 1, 1);

uniform float time;
float sine = sin(time);
float cosine = cos(time);
uniform sampler2D texture_sampler;

float3 EyeRayDirection(float x, float y, float w, float h)
{
    float field_of_view = 3.141592654f / 2.0f;
    float3 ray_direction;

    ray_direction.x = x + 0.5f - (w / 2.0f);
    ray_direction.y = y + 0.5f - (h / 2.0f);
    ray_direction.z = -w / tan(field_of_view / 2.0f);

    return normalize(ray_direction);
}
 
float obj1( vec3 p ) // sphere
{
    return length(p)-0.2;
}

float obj2( vec3 p ) // hex prism
{
    vec2 h = vec2(0.2, 0.3);
    vec3 q = abs(p);

    const vec3 k = vec3(-0.8660254, 0.5, 0.57735);
    p = abs(p);
    p.xy -= 2.0*min(dot(k.xy, p.xy), 0.0)*k.xy;
    vec2 d = vec2(
       length(p.xy - vec2(clamp(p.x, -k.z*h.x, k.z*h.x), h.x))*sign(p.y - h.x),
       p.z-h.y );
    return min(max(d.x,d.y),0.0) + length(max(d,0.0));
}

float obj3( vec3 p) // cylinder
{
    vec2 h = vec2(0.2, 0.3);
    vec2 d = abs(vec2(length(p.xz),p.y)) - h;
    return min(max(d.x,d.y),0.0) + length(max(d,0.0));
}

float obj4( in vec3 p) // ellipsoid
{
    vec3 r = vec3(0.1, 0.2, 0.3);
    float k0 = length(p/r);
    float k1 = length(p/(r*r));
    return k0*(k0-1.0)/k1;
    
}

float obj5( vec3 p) // torus
{
    vec2 t = vec2(0.3, 0.12);
    return length( vec2(length(p.xz)-t.x,p.y) )-t.y;
}

float obj6( vec3 p) // cube
{
    vec3 d = abs(p) - 0.2;
    return min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0));
}

vec2 sdAll( in vec3 pos) // result: (dist, id)
{
    const float ORBIT_RADIUS1 = 0.6;
    const float ORBIT_RADIUS2 = 1.1;
    const float ORBIT_RADIUS3 = 1.5;
    float res = 1e10;
    float t = 0;
    int id = 0;
    
    if ((t = obj1(pos - ORBIT_RADIUS1*vec3(sine, cosine, -0.6))) < res)
    {
        res = t;
        id = 1;
    }
    
    if ((t = obj2(pos - ORBIT_RADIUS1*vec3(-sine, -cosine, -0.6))) < res)
    {
        res = t;
        id = 2;
    }
    
    if ((t = obj3(pos - ORBIT_RADIUS2*vec3(sine, 0.0, cosine -0.6))) < res)
    {
        res = t;
        id = 3;
    }
    
    if ((t = obj4(pos - ORBIT_RADIUS2*vec3(-sine, 0.0, -cosine -0.6))) < res)
    {
        res = t;
        id = 4;
    }
    
    if ((t = obj5(pos - ORBIT_RADIUS3*vec3(0.0, sine, cosine -0.6))) < res)
    {
        res = t;
        id = 5;
    }
    
    if ((t = obj6(pos - ORBIT_RADIUS3*vec3(0.0, -sine, -cosine -0.6))) < res)
    {
        res = t;
        id = 6;
    }
    
    return vec2(res, id);
}

vec3 CalcNormal( in vec3 pos ) // fast centered normal
{
    const float EPS = 1e-5;
    vec2 e = vec2(1.0,-1.0)*EPS;
    return normalize( e.xyy*sdAll( pos + e.xyy ).x + 
					  e.yyx*sdAll( pos + e.yyx ).x + 
					  e.yxy*sdAll( pos + e.yxy ).x + 
					  e.xxx*sdAll( pos + e.xxx ).x );
}

vec4 ray_marching( in vec3 ray_pos, in vec3 ray_dir) // result: (pos, id)
{
    float dist = 1.0;
    float EPS = 1e-3;
    int id = 0;
    while(dist < 1e3)
    {
        vec2 res = sdAll( ray_pos+ray_dir*dist);
        float shift = res.x;
        id = int(res.y+0.0001);
        if( abs(shift) < EPS )
        { 
            return vec4(ray_pos+ray_dir*dist, id);
        }
        dist += shift;
    }
        
    
    id = 0;
    return vec4(ray_pos+ray_dir*dist, id);
}

const vec4 colors[6] = vec4[6](vec4(1,0,0,1), vec4(0.5,0.5,0,1), vec4(0,1,0,1), vec4(0,0.5,0.5,1), vec4(0,0,1,1), vec4(0.5,0,0.5,1));

float func(float x)
{
    x *= 10;
    return exp(-x*x/2.0);
}

vec4 light_color(vec3 pos, vec3 V, vec4 color)
{
    const float c_att = 1.0; // constant factor
    const float l_att = 0.01; // linear factor
    const float q_att = 0.0001; // quadratic factor

    float kd = 0.8; // diffuse
    float ks = 0.8; // specular
    float n = 100; // shininess
    
    vec3 L = -normalize(pos);
    vec3 N = CalcNormal(pos);
    
    float dist = length(pos);
    float LdotN = max(0, dot(L, N));
    float diffuse = kd * LdotN;
    float specular = 0;
    
    if (LdotN > 0) {
        vec3 H = normalize(L + V);
        specular = ks * pow(max(0, dot(H, N)), n);
    }
        
    float att = 1.0 / (c_att + l_att * dist + q_att * dist * dist);
    float ambient = 0.25;
    
    return att * (diffuse * color + specular) + ambient * color;
}

void main(void)
{
    float w = float(g_screenWidth);
    float h = float(g_screenHeight);

    // get curr pixelcoordinates
    //
    float x = fragmentTexCoord.x * w;
    float y = fragmentTexCoord.y * h;

    // generate initial ray
    //
    float3 ray_pos = float3(0, 0, 0);
    float3 ray_dir = EyeRayDirection(x, y, w, h);

    // transorm ray with matrix
    //
    ray_pos = (g_rayMatrix * float4(ray_pos, 1)).xyz;
    ray_dir = float3x3(g_rayMatrix)* ray_dir;
    
    vec4 march_result = ray_marching(ray_pos, ray_dir);
    vec3 res_pos = march_result.xyz;
    int id = int(march_result.w+0.0001);
    
    if (id == 0) {
        fragColor = vec4(0,0,0,1);
        return;
    }
    fragColor = colors[id-1];
    if (id == 2) {
        fragColor = texture(texture_sampler, (res_pos.xy - 0.6*vec2(-sine, -cosine))*2);
    }
    
    
    fragColor = light_color(res_pos, -ray_dir, fragColor);
    
}


