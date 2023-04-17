#pragma once
#define _USE_MATH_DEFINES
#include <math.h>
#include "GLM/glm.hpp"
#include "GLM/vec3.hpp"
#include "Tools.h"
#include "ObjLoader.h"
struct Ray {
    glm::vec3 origin;
    glm::vec3 dir;
    Ray() {}
    Ray(glm::vec3 origin, glm::vec3 dir);
    Ray reflect_spherical(const glm::vec3& from, const glm::vec3& normal) const;
    Ray reflect(const glm::vec3& from, const glm::vec3& normal) const;
    bool refract(const glm::vec3& from, const glm::vec3& normal, float refr1, float refr2, Ray& out) const;
};
class PMModel {
    static size_t id_gen;
    static float eps;
    ModelConstructInfo mci;
    size_t id;
    bool traingle_intersection(const Ray& ray, bool in_object, const glm::vec3& p0,
        const glm::vec3& p1, const glm::vec3& p2, float& out) const;
public:
    std::string name; // TODO разобраться как const
    PMModel() {}
    PMModel(const PMModel& other);
    PMModel(const ModelConstructInfo& mci);
    bool intersection(const Ray& ray, bool in_object, float& intersection, glm::vec3& normal) const;
    bool equal(const PMModel& other) const;
    bool equal(size_t other_id) const;
    const Material* get_material() const;
    size_t get_id() const;
    glm::vec3* get_wn() const;
};
struct PMScene {
    std::vector<PMModel> objects;
    /*
    * 1    2
    * |    ^
    * v    |
    * 4 -> 3
    */
    glm::vec3 right_upper, left_lower, camera, normal;
    PMScene(const std::vector<PMModel>& objects);
    PMScene();
};