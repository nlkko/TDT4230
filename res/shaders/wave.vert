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

vec3 wave_normal(GerstnerWave w, vec3 p) {
    float k = (2*PI) / w.wavelength;
    float c = sqrt(G / k);
    vec2 d = normalize(w.direction);
    float f = k * (dot(d, p.xz) - (time * c));
    float a = w.steepness / k;

    vec3 tangent = vec3(
        1 - d.x * d.x * (w.steepness * sin(f)),
        d.x * (w.steepness * cos(f)),
        -d.x * d.y * (w.steepness * sin(f))
    );

    vec3 binormal = vec3(
        -d.x * d.y * ( w.steepness * sin(f) ),
        d.y * ( w.steepness * cos(f) ),
        1 - d.y * d.y * (w.steepness * sin(f))
    );

    vec3 normal = normalize(cross(binormal, tangent));

    return normal;
}

void main()
{
    GerstnerWave wave1 = GerstnerWave(vec2(1), 0.5, 10);
    GerstnerWave wave2 = GerstnerWave(vec2(0, 1), 0.25, 20);
    GerstnerWave wave3 = GerstnerWave(vec2(-0.25, -0.5), 0.7, 60);

    vec3 wave_position = position;

    wave_position = wave_function(wave1, position);
    wave_position += wave_function(wave2, position);
    wave_position += wave_function(wave3, position);
    wave_position -= (W_N - 1) * position;


    vec3 wave_normal = normal_in;
    wave_normal += wave_normal(wave1, normal_in);
    wave_normal += wave_normal(wave2, normal_in);
    wave_normal += wave_normal(wave3, normal_in);
    wave_normal -= (W_N - 1) * normal_in;

    height_out = wave_position.y;
    a_out = max( max(wave1.steepness, wave2.steepness), wave3.steepness );

    // OUT
    normal_out = wave_normal;
    textureCoordinates_out = textureCoordinates_in;
    position_out = wave_position;

    gl_Position = MVP * vec4(wave_position, 1.0);
}