#pragma once
#include "Light.h"
#include "Tools.h"
#include "PMModel.h"
#define _USE_MATH_DEFINES
#include <math.h>
class PhotonMapping {
    struct Photon {
        glm::vec3 pos;
        glm::vec3 power;
        char pi, theta;
        short flag;
    };
    
    std::vector<PMModel> scene;
    size_t phc; // Emitted photoms capacity
    void emit(const LightSource& ls) {
        size_t ne = 0;// Number of emitted photons
        while (ne < phc) {
            int x, y, z;
            do {
                x = Random<float>::random(-1.f, 1.f);
                y = Random<float>::random(-1.f, 1.f);
                z = Random<float>::random(-1.f, 1.f);
            } while (x * x + y * y + z * z > 1.f); // TODO normalize ?
            Ray ray(ls.pos, { x, y, z });
            trace(ray);
            // TODO trace photon from ls pos in dir d
            ne++;
        }
        //scale power of stored photons with 1/ne
    }
    /// <summary>
    /// Возвращает значение, будет ли фотон отражен диффузно, зеркально или вовсе рассеян
    /// </summary>
    /// <param name="mat">Материал поверхности, на которую попал фотон</param>
    /// <param name="ipp">Incident photon power — мощность попавшего фотона в RGB </param>
    /// <returns></returns>
    bool destiny(const Material& mat, glm::vec3& ipp) {
        auto max_ipp = std::max(std::max(ipp.r, ipp.g), ipp.b);
        float e = Random<float>::random(0.f, 1.f); // Only for pd + ps <= 1

        auto ipp_d = mat.diffuse * ipp;
        auto max_ipp_d = std::max(std::max(ipp_d.r, ipp_d.g), ipp_d.b);
        auto pd = max_ipp_d / max_ipp;
        if (e <= pd) {  
            ipp *= mat.diffuse / pd; // diffuse reflection
            return true;
        }

        auto ipp_s = mat.specular * ipp;
        auto max_ipp_s = std::max(std::max(ipp_s.r, ipp_s.g), ipp_s.b);
        auto ps = max_ipp_s / max_ipp;
        if (e <= pd + ps) {
            ipp *= mat.specular / ps; // specular reflection
            return true;
        }

        return false;
    }
    void trace(const Ray& ray) {
        float inter = 0.f;
        glm::vec3 normal;
        Material material;
        for (const PMModel& model : scene) {
            float temp_inter;
            glm::vec3 temp_normal;
            bool succ = model.intersection(ray, temp_inter, temp_normal);
            if (succ && inter == 0.f || temp_inter < inter) {
                inter = temp_inter;
                normal = temp_normal;
                material = model.get_material();
            }
        }
        if (inter == 0.f) {
            return;
        }
        if (glm::dot(ray.dir, normal) > 0) {
            normal *= -1.f;
        }
        glm::vec3 inter_p = ray.origin + ray.dir * inter;
        int dest = 0;// destiny(material);
        switch (dest)
        {
        case 0:
            break; // diffuse reflection
        case 1:
            break; // specular reflection
        case 2:
            return; // absorption
        default:
            break;
        }
    }
    float GGX_GFunction(float cosNX, float sqRoughness) // Потерянный свет
    {
        float alpha = sqRoughness * sqRoughness;
        float sqCosNX = cosNX * cosNX;
        float sqTan = 1 / sqCosNX - 1; // 1+ tan^2 = 1 / cos^2
        float res = 2 / (1 + sqrt(1 + alpha * sqTan));
        return res;
    }

    float GGX_DFunction(float cosNX, float sqRoughness)
    {
        float alpha = sqRoughness * sqRoughness;
        float sqCosNX = cosNX * cosNX;
        float fourthCosNX = sqCosNX * sqCosNX;
        float sqTan = 1 / sqCosNX - 1; // 1+ tan^2 = 1 / cos^2
        float part = (alpha + sqTan);
        float res = alpha / (M_PI * fourthCosNX * part * part);
        return res;
    }

    glm::vec3 FresnelSchlick(float cosNL, glm::vec3 F0)
    {
        return F0 + (glm::vec3(1.f) - F0) * pow(1.f - cosNL, 5.f);
    }
    glm::vec3 brdf(float HdotV, float NdotL, float NdotV, float NdotH, glm::vec3 F, float roughness) {
        glm::vec3 F = FresnelSchlick(HdotV, glm::vec3(0.f)); // материал
        auto ck = CookTorrance_GGX(NdotL, NdotV, NdotH, F, roughness);
        // TODO доделать
    }
    glm::vec3 CookTorrance_GGX(float NdotL, float NdotV, float NdotH, glm::vec3 F, float roughness)
    {
        if (NdotL <= 0.0 || NdotV <= 0.0) return glm::vec3(0.0);

        float sqRoughness = roughness * roughness;
        float GCoeff = GGX_GFunction(NdotL, sqRoughness) * GGX_GFunction(NdotV, sqRoughness);
        float DCoeff = GGX_DFunction(NdotH, sqRoughness);

        return glm::max(GCoeff * DCoeff * F * 0.25f / NdotV, 0.f);
    }
    void build_map() {

    }
};