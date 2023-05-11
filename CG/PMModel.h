#pragma once
#define _USE_MATH_DEFINES
#include <math.h>
#include "GLM/glm.hpp"
#include "GLM/vec3.hpp"
#include "Tools.h"
#include "ObjLoader.h"
#include <unordered_map>
#include "Light.h"
#include "Light.h"
struct Ray {
    glm::vec3 origin;
    glm::vec3 dir;
    Ray() {}
    Ray(const glm::vec3& origin, const glm::vec3& dir);
    Ray reflect_spherical(const glm::vec3& from, const glm::vec3& normal) const;
    Ray reflect(const glm::vec3& from, const glm::vec3& normal) const;
    bool refract(const glm::vec3& from, const glm::vec3& normal, float refr1, float refr2, Ray& out) const;
};
class PMModel {
    struct BarycentricCache
    {
        glm::vec3 v0, v1;
        float d00, d01, d11, denom;
        BarycentricCache() {}
        BarycentricCache(const glm::vec3& v0, const glm::vec3& v1, float d00, float d01,
            float d11, float denom) : v0(v0), v1(v1), d00(d00), d01(d01), d11(d11), denom(denom) {}
    };
    static size_t id_gen;
    static float eps;
    std::unordered_map<glm::vec3, BarycentricCache, Vec3Hash> bcache;
    ModelConstructInfo mci;
    size_t id;
    glm::vec3 barycentric_coords(const glm::vec3& point, const glm::vec3& tr_ind);
    bool traingle_intersection(const Ray& ray, bool in_object, const glm::vec3& p0,
        const glm::vec3& p1, const glm::vec3& p2, float& out) const;
    LightSource* ls;
public:
    std::string name;
    PMModel() {}
    PMModel(const PMModel& other);
    PMModel(const ModelConstructInfo& mci, LightSource* ls= nullptr);
    bool intersection(const Ray& ray, bool in_object, float& intersection, glm::vec3& inter_ind) const;
    bool equal(const PMModel& other) const;
    bool equal(size_t other_id) const;
    void set_light_intensity(const glm::vec3& value);
    const Material* get_material() const;
    size_t get_id() const;
    glm::vec3* get_wn() const;
    glm::vec3 get_normal(size_t i) const;
    void get_normal(const glm::vec3& inter_ind, const glm::vec3& point, glm::vec3& normal);
    const LightSource* get_ls() const;
    std::pair<glm::vec3, glm::vec3> get_radiation_info() const;
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