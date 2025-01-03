#ifndef POINTLIGHT_H
#define POINTLIGHT_H

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include <vector>
#include <string>

using namespace glm;
using namespace std;


class PointLight {
public:
    // Data members
    vec3 position;       // Position of the light
    float constant;      // Constant attenuation factor
    float linear;        // Linear attenuation factor
    float quadratic;     // Quadratic attenuation factor
    vec3 ambient;        // Ambient color of the light
    vec3 diffuse;        // Diffuse color of the light
    vec3 specular;       // Specular color of the light

    // Constructor to initialize all members
    PointLight(const vec3& position, float constant, float linear, float quadratic,
        const vec3& ambient, const vec3& diffuse, const vec3& specular)
        : position(position),
        constant(constant),
        linear(linear),
        quadratic(quadratic),
        ambient(ambient),
        diffuse(diffuse),
        specular(specular)
    {
    }
};

#endif

