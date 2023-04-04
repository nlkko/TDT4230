#version 430 core

in layout(location = 0) vec3 position;
in layout(location = 1) vec3 normal_in;
in layout(location = 2) vec2 textureCoordinates_in;

uniform layout(location = 4) mat4 MVP;
uniform layout(location = 5) float time;

out layout(location = 0) vec3 normal_out;
out layout(location = 1) vec2 textureCoordinates_out;
out layout(location = 2) vec3 position_out;

out layout(location = 3) float height_out;
out layout(location = 4) float a_out;

#define PI 3.1415926538
#define G 9.81 // earth's gravity
#define W_N 3 // number of waves

uniform struct GerstnerWave {
    vec2 direction;
    float steepness; // range from 0 to 1
    float wavelength;
};

vec3 wave_function(GerstnerWave w, vec3 p) {
    float k = (2*PI) / w.wavelength;
    float c = sqrt(G / k);
    vec2 d = normalize(w.direction);
    float f = k * (dot(d, p.xz) - (time * c));
    float a = w.steepness / k;

    p.x += d.x * (a * cos(f));
    p.y = a * sin(f);
    p.z += d.y * (a * cos(f));

    return p;
}

vec3 wave_tangent(GerstnerWave w, vec3 p) {
    float k = (2*PI) / w.wavelength;
    float c = sqrt(G / k);
    vec2 d = normalize(w.direction);
    float f = k * (dot(d, p.xz) - (time * c));
    float a = w.steepness / k;

    vec3 tangent = vec3(
        -d.x * d.x * (w.steepness * sin(f)),
        d.x * (w.steepness * cos(f)),
        -d.x * d.y * (w.steepness * sin(f))
    );

    return tangent;
}

vec3 wave_binormal(GerstnerWave w, vec3 p) {
    float k = (2*PI) / w.wavelength;
    float c = sqrt(G / k);
    vec2 d = normalize(w.direction);
    float f = k * (dot(d, p.xz) - (time * c));
    float a = w.steepness / k;

    vec3 binormal = vec3(
    -d.x * d.y * ( w.steepness * sin(f) ),
    d.y * ( w.steepness * cos(f) ),
    -d.y * d.y * (w.steepness * sin(f))
    );

    return binormal;
}

void main()
{
    GerstnerWave waves[W_N];

    waves[0] = GerstnerWave(vec2(1, 1), 0.25, 30);
    waves[1] = GerstnerWave(vec2(1, 0.6), 0.25, 15);
    waves[2] = GerstnerWave(vec2(1, 1.3), 0.25, 12);

    vec3 wave_position = position;
    vec3 w_tangent = vec3(0);
    vec3 w_binormal = vec3(0);

    for(int i=0; i < W_N; ++i) {
        wave_position += wave_function(waves[i], position);
        w_tangent += wave_tangent(waves[i], position);
        w_binormal += wave_binormal(waves[i], position);
    }

    wave_position -= (W_N) * position;
    w_tangent -= (W_N) * position;
    w_binormal -= (W_N) * position;


    // OUT
    height_out = wave_position.y;
    a_out = max( max(waves[0].steepness, waves[1].steepness), waves[2].steepness );
    normal_out = normalize(cross(w_binormal, w_tangent));
    textureCoordinates_out = textureCoordinates_in;
    position_out = wave_position;

    gl_Position = MVP * vec4(wave_position, 1.0);
}