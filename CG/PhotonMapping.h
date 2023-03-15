#pragma once
#include "Light.h"
#include "Tools.h"
#include "PMModel.h"
#define _USE_MATH_DEFINES
#include <math.h>
class PhotonMapping {
public:
    struct Photon {
        glm::vec3 pos;
        glm::vec3 power;
        Photon(const glm::vec3& pos, const glm::vec3& power){
            this->pos = pos;
            this->power = power;
        }
    };
private:
    enum class PathType {
        dif_refl = 0, spec_refl, absorption
    };
    std::vector<Photon> stored_photons;
    std::vector<PMModel> scene;
    std::vector<LightSource> lsources;
    size_t phc; // Emitted photoms capacity
    void emit(const LightSource& ls) {
        size_t ne = 0;// Number of emitted photons
        while (ne < phc) {
            float x, y, z;
            do {
                x = Random<float>::random(-1.f, 1.f);
                y = Random<float>::random(-1.f, 0.f); // В конкретном случае свет должен светить только вниз Random<float>::random(-1.f, 1.f);
                z = Random<float>::random(-1.f, 1.f);
            } while (x * x + y * y + z * z > 1.f); // TODO normalize ?
            Ray ray(ls.position, glm::normalize(glm::vec3(x, y, z)));
            auto pp = ls.diffuse; // photon power
            trace(ray, pp);
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
    PathType destiny(const Material& mat, glm::vec3& ipp) {
        auto max_ipp = std::max(std::max(ipp.r, ipp.g), ipp.b);
        float e = Random<float>::random(0.f, 1.f); // Only for pd + ps <= 1

        auto ipp_d = mat.diffuse * ipp;
        auto max_ipp_d = std::max(std::max(ipp_d.r, ipp_d.g), ipp_d.b);
        auto pd = max_ipp_d / max_ipp;
        if (e <= pd) {  
            ipp *= mat.diffuse / pd; // diffuse reflection
            return PathType::dif_refl;
        }

        auto ipp_s = mat.specular * ipp;
        auto max_ipp_s = std::max(std::max(ipp_s.r, ipp_s.g), ipp_s.b);
        auto ps = max_ipp_s / max_ipp;
        if (e <= pd + ps) {
            ipp *= mat.specular / ps; // specular reflection
            return PathType::spec_refl;
        }

        return PathType::absorption;
    }
    /// <summary>
    /// 
    /// </summary>
    /// <param name="ray"></param>
    /// <param name="pp">- photon power</param>
    void trace(const Ray& ray, glm::vec3& pp) {
        float inter = 0.f;
        glm::vec3 normal;
        Material material;
        for (const PMModel& model : scene) {
            float temp_inter;
            glm::vec3 temp_normal;
            bool succ = model.intersection(ray, temp_inter, temp_normal);
            if (succ && (inter == 0.f || temp_inter < inter)) {
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
        Ray new_ray;
        PathType dest = destiny(material, pp);
        switch (dest) {
        case PathType::dif_refl:
            stored_photons.push_back(Photon(inter_p, pp));
            new_ray = ray.reflect_spherical(inter_p, normal);
            break;
        case PathType::spec_refl:
            new_ray = ray.reflect(inter_p, normal);
            break;
        case PathType::absorption:
            stored_photons.push_back(Photon(inter_p, pp)); // TODO проверка на зеркальность?
            return;
        default:
            break;
        }
        trace(new_ray, pp);
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
        glm::vec3 Ff = FresnelSchlick(HdotV, glm::vec3(0.f)); // материал
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
    
public:
    PhotonMapping(const std::vector<PMModel>& scene, const std::vector<LightSource>& lsources, size_t phc) {
        this->scene = scene;
        this->lsources = lsources;
        this->phc = phc;
        this->stored_photons = std::vector<Photon>();
    }
    std::vector<Photon>* build_map() {
        for (const LightSource& ls : lsources) {
            emit(ls);
        }
        return &stored_photons;
    }
};