#pragma once
#include "ObjLoader.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include "GLM/glm.hpp"
#include "GLM/vec3.hpp"
struct Ray {
    glm::vec3 origin;
    glm::vec3 dir;
    Ray() {};
    Ray(glm::vec3 origin, glm::vec3 dir) {
        this->origin = origin;
        this->dir = dir;
    }
    Ray reflect_spherical(const glm::vec3& from, const glm::vec3& normal) const {
        float e1 = Random<float>::random();
        float e2 = Random<float>::random();
        float theta = std::pow(std::cos(std::sqrt(e1)), -1.f);
        float phi = 2 * M_PI * e2;
        glm::vec3 new_dir;
        float r = glm::distance(from, normal);
        float sin_theta = sin(theta);
        new_dir.x = r * sin_theta * cos(phi);
        new_dir.y = r * sin_theta * sin(phi);
        new_dir.z = r * cos(theta);
        return { from, new_dir };
    }
    Ray reflect(glm::vec3 from, glm::vec3 normal) const {
        glm::vec3 refl_dir = 2.f * normal * glm::dot(dir, normal) - dir; // TODO или наоборот? затестить
        return { from, glm::normalize(from + refl_dir) };
    }
    bool refract(glm::vec3 from, glm::vec3 normal, float refr1, float refr2, Ray& out) const {
        float refr_index = refr1 / refr2;
        float ndd = glm::dot(dir, normal);
        auto theta = 1.f - refr_index * refr_index * (1.f - ndd * ndd); // TODO исправить? https://stackoverflow.com/questions/42218704/how-to-properly-handle-refraction-in-raytracing
        if (theta < 0.f) {
            return false;
        }
        float cos_theta = glm::sqrt(theta);
        out.origin = from;
        out.dir = glm::normalize(dir * theta - (cos_theta + refr_index * ndd) * normal);
        return true;
    }
};
float eps = 0.0001f;
class PMModel {
    ModelConstructInfo mci;
    bool traingle_intersection(const Ray& ray, const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, float& out) const {
        out = -1.f;
        glm::vec3 edge1 = p1 - p0;
        glm::vec3 edge2 = p2 - p0;
        glm::vec3 h = glm::cross(ray.dir, edge2);
        float a = glm::dot(edge1, h);

        if (a > -eps && a < eps) {
            return false;       // Этот луч параллелен этому треугольнику.
        }

        float f = 1.0f / a;
        glm::vec3 s = ray.origin - p0;
        float u = f * glm::dot(s, h);
        if (u < 0 || u > 1)
            return false;

        glm::vec3 q = glm::cross(s, edge1);
        float v = f * glm::dot(ray.dir, q);
        if (v < 0 || u + v > 1) {
            return false;
        }
        // На этом этапе мы можем вычислить t, чтобы узнать, где находится точка пересечения на линии.
        float t = f * glm::dot(edge2, q);
        if (t > eps)
        {
            out = t;
            return true;
        }
        //Это означает, что есть пересечение линий, но не пересечение лучей.
        return false;
    }
public:
    PMModel(const ModelConstructInfo& mci) {
        //this->mci = mci;
    }
   bool intersection(const Ray& ray, float& intersection, glm::vec3& normal) const {
        intersection = 0.f;
        size_t inter_ind = 0;
        if (mci.render_mode == GL_TRIANGLES) {
            for (size_t i = 0; i < mci.vertices.size(); i += 3) {
                float temp = 0.f;
                bool succ = traingle_intersection(ray, mci.vertices[i].position, mci.vertices[i + 1].position,
                    mci.vertices[i + 2].position, temp);
                if (succ && intersection == 0 || temp < intersection) {
                    intersection = temp;
                    inter_ind = i;
                }
            }
        }
        else {
            throw std::exception("Not implemented"); // TODO ?
        }
        if (intersection == 0.f) {
            return false;
        }
        normal = mci.vertices[inter_ind].normal;
        /*normal += mci.vertices[inter_ind + 1].normal;
        normal += mci.vertices[inter_ind+2].normal;
        normal /= 3;*/
        return true;
    }
    Material get_material() const {
        return mci.material;
    }
};