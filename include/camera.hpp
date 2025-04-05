#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct MVP
{
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
    glm::mat4 inverseModel = glm::mat4(1.0f);
};

struct LightUBO
{
    alignas(16) glm::vec3 lightColor;
    alignas(16) glm::vec3 lightPos;
    alignas(16) glm::vec3 lightDir;
    alignas(16) glm::vec3 viewPos;
    alignas(4) float constant;
    alignas(4) float linear;
    alignas(4) float quadratic;
    alignas(4) float cutoff;
    alignas(4) float outerCutoff;
};