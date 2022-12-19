#pragma once
#include <glm/glm.hpp>
struct Ray {
    glm::vec3 start, direction;
    float distance;

    Ray(glm::vec3 st, glm::vec3 end)
    {
        start = st;
        direction = glm::normalize(end - st);
        distance = 0;
    }
    Ray() { }
    Ray(const Ray& r)
    {
        start = r.start;
        direction = r.direction;
    }
    glm::vec3 move() {
        distance += 0.05;
        return start + direction * distance;
    }
};