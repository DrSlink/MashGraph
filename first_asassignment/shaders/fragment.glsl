#version 330

uniform int g_screenWidth;
uniform int g_screenHeight;
layout(location = 0) out vec4 fragColor;

vec3 light1_c = vec3(0.45, 0.6, 1.5);
vec3 fourth_c = vec3(0.45, 0.3, 1.5);
vec3 first_c = vec3(0.45, 0.0, 1.5);
vec3 fifth_c = vec3(0, 0.8, 2.3);
vec3 light2_c = vec3(-0.45, 0.5, 1);
vec3 second_c = vec3(-0.45, 0.3, 2);
vec3 third_c = vec3(-0.5, -0.2, 2.5);
vec4 Colors[6];

vec2 closestDist(vec3 point) {
    float first = length(first_c - point) - 0.2;
    float second = length(second_c - point) - 0.2f;
    float third = length(max(abs(third_c - point)-vec3(0.05, 0.15, 0.1),0.0))-0.05;
    vec3 p = (fourth_c - point);
    vec3 t = vec3(0.2, 0.06, 0);
    float fourth = length(vec2(length(p.xz)-t.x,p.y)) - t.y;
    float fifth = length(max(abs(fifth_c - point)-vec3(0.2, 0.05, 0.1),0.0))-0.05;
    float floor = point.y + 0.5;
    float loc_min = min(floor, min(fifth, min(fourth, min(first, min(second, third)))));

    if (loc_min == first){
        return vec2(loc_min, 1);
    }
    if (loc_min == second){
        return vec2(loc_min, 2);
    }
    if (loc_min == third){
        return vec2(loc_min, 3);
    }
    if (loc_min == fourth){
        return vec2(loc_min, 4);
    }
    if (loc_min == fifth){
        return vec2(loc_min, 5);
    }
    else {
        return vec2(loc_min, 0);
    }
}

vec3 norm(vec3 z){
    float epps = 0.001;
    vec2 z1 = closestDist(z + vec3(epps, 0, 0));
    vec2 z2 = closestDist(z - vec3(epps, 0, 0));
    vec2 z3 = closestDist(z + vec3(0, epps, 0));
    vec2 z4 = closestDist(z - vec3(0, epps, 0));
    vec2 z5 = closestDist(z + vec3(0, 0, epps));
    vec2 z6 = closestDist(z - vec3(0, 0, epps));
    float dx = z1.x - z2.x;
    float dy = z3.x - z4.x;
    float dz = z5.x - z6.x;
    return normalize(vec3(dx, dy, dz) / (2.0*epps));
}

float seeLight(vec3 point, vec3 ray_Dir, float distToLight){
    vec2 cl_d;
    int num_steps = 200;
    vec3 first_point = point;
    while (abs(point.x) < 10 && abs(point.y) < 10  && abs(point.z) < 10 && num_steps > 0 && length(first_point - point) < distToLight) {
        cl_d = closestDist(point);
        if (cl_d.x < 0.001) {
            return 0.f;
        }
        point = point + cl_d.x * ray_Dir;
        num_steps--;
    }
    return 1.0;
}

vec4 rayMarch(vec3 point, vec3 ray_Dir) {
    vec2 cl_d;
    int num_steps = 200;
    while (abs(point.x) < 10 && abs(point.y) < 10  && abs(point.z) < 10 && num_steps > 0) {
        cl_d = closestDist(point);
        if (cl_d.x < 0.001) {
            vec3 N = norm(point);
            vec3 light1 = normalize(light1_c - point);
            vec3 light2 = normalize(light2_c - point);
            float light_intensity1 = max(0.f, dot(N, light1));
            float light_intensity2 = max(0.f, dot(N, light2));
            float see1 = seeLight(N * 0.001 + point, light1, length(light1_c - point));
            float see2 = seeLight(N * 0.001 + point, light2, length(light2_c - point));
            return (0.1 + see1 * light_intensity1 + see2 * light_intensity2) / 2 * Colors[int(cl_d.y)];
        }
        point = point + cl_d.x * ray_Dir;
        num_steps--;
    }
    return vec4(0, 0, 1, 1);
}

void main(void) {
    Colors[0] = vec4(1,1,0,1);
    Colors[1] = vec4(1,1,1,1);
    Colors[2] = vec4(1,0,1,1);
    Colors[3] = vec4(0,1,1,1);
    Colors[4] = vec4(1,0,0,1);
    Colors[5] = vec4(0,1,0,1);
    float w = float(g_screenWidth);
    float h = float(g_screenHeight);
    float m = (gl_FragCoord.x - w / 2) / w;
    float t = (gl_FragCoord.y - h / 2) / h;
    vec3 coord_point = vec3(m, t, 1);
    vec3 ray_dir = normalize(coord_point);
    fragColor = rayMarch(coord_point, ray_dir);
}