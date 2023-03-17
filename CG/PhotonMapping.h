#pragma once
#include "Light.h"
#include "Tools.h"
#include "PMModel.h"
#include <stack>
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
        dif_refl = 0, spec_refl, absorption, refr
    };
    std::vector<Photon> stored_photons;
    Stack<const Material&> mediums;
    std::vector<PMModel> scene;
    std::vector<LightSource> lsources;
    size_t phc; // Emitted photoms capacity
    void emit(const LightSource& ls) {
        Material ls_medium;
        ls_medium.refr_index = 1.f;
        mediums.push(ls_medium);
        size_t ne = 0;// Number of emitted photons
        while (ne < phc) {
            float x, y, z;
            do {
                x = Random<float>::random(-1.f, 1.f);
                y = Random<float>::random(-1.f, 0.f); // ¬ конкретном случае свет должен светить только вниз Random<float>::random(-1.f, 1.f);
                z = Random<float>::random(-1.f, 1.f);
            } while (x * x + y * y + z * z > 1.f); // TODO normalize ?
            Ray ray(ls.position, glm::normalize(glm::vec3(x, y, z)));
            auto pp = ls.diffuse; // photon power
            trace(ray, false, pp);
            // TODO trace photon from ls pos in dir d
            ne++;
        }
        //scale power of stored photons with 1/ne
    }
    /// <summary>
    /// 
    /// </summary>
    /// <param name="refr_index1"> - refractive index of "from" medium</param>
    /// <param name="refr_index2"> - refractive index of "to" medium</param>
    /// <param name="cosLN"> - cos between surface normal and reverse light direction</param>
    /// <returns></returns>
    float refraction_angle(float refr_index1, float refr_index2, float cosLN) { // uses n1 sin(theta1) = n2 sin(theta2) formula
        float sin_theta1 = glm::sin(glm::acos(cosLN)); // aka sinLN
        float sin_theta2 = refr_index1 / refr_index2 * sin_theta1;
        //float  37

    }
    /// <summary>
    /// ¬озвращает значение, будет ли фотон отражен диффузно, зеркально или вовсе рассе€н
    /// </summary>
    /// <param name="om">Origin material - ћатериал поверхности, изначальной поверхности</param>
    /// <param name="im">Incident material - ћатериал поверхности, на которую попал фотон</param>
    /// <param name="ipp">Incident photon power Ч мощность попавшего фотона в RGB </param>
    /// <returns></returns>
    PathType destiny(float cosNL, const Material& om, const Material& im, glm::vec3& ipp) {
        float e;
        /*
        * ѕоверхность, на которую попал фотон, должна иметь не единичный показатель преломлени€, иначе считаем,
        * что фотон будет просто отражен. ≈сли же показатели преломлени€ равны, то это означает, 
        * что преломлени€ так же не должно быть. Ќапример, фотон попал из воды в воду.
        * 
        */
        if (im.refr_index != 1.f && om.refr_index != im.refr_index) {
            float refr_probability = 1.f - FresnelSchlick(cosNL, om.refr_index, im.refr_index);
            e = Random<float>::random(0.f, 1.f);
            if (e <= refr_probability) {
                return PathType::refr;
            }
        }

        auto max_ipp = std::max(std::max(ipp.r, ipp.g), ipp.b);
        e = Random<float>::random(0.f, 2.f); // upper bound = 2.f because max|d + s| = 2

        auto ipp_d = im.diffuse * ipp;
        auto max_ipp_d = std::max(std::max(ipp_d.r, ipp_d.g), ipp_d.b);
        auto pd = max_ipp_d / max_ipp;
        if (e <= pd) {  
            ipp *= im.diffuse / pd; // diffuse reflection
            return PathType::dif_refl;
        }

        auto ipp_s = im.specular * ipp;
        auto max_ipp_s = std::max(std::max(ipp_s.r, ipp_s.g), ipp_s.b);
        auto ps = max_ipp_s / max_ipp;
        if (e <= pd + ps) {
            ipp *= im.specular / ps; // specular reflection
            return PathType::spec_refl;
        }

        return PathType::absorption;
    }
    /// <summary>
    /// 
    /// </summary>
    /// <param name="ray"></param>
    /// <param name="pp">- photon power</param>
    void trace(const Ray& ray, bool in_object, glm::vec3& pp) {
        float inter = 0.f;
        glm::vec3 normal;
        Material material;
        for (const PMModel& model : scene) {
            float temp_inter;
            glm::vec3 temp_normal;
            bool succ = model.intersection(ray, in_object, temp_inter, temp_normal);
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
        float cosNL = glm::dot(-ray.dir, normal);
        PathType dest = destiny(cosNL, cur_mat, material, pp);
        switch (dest) {
        case PathType::refr:
        {
            bool succ = ray.refract(inter_p, normal, cur_mat.refr_index, material.refr_index, new_ray);
            if (!succ) {
                throw std::exception("„«’?");
            }
            break;
        }
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
        trace(new_ray, material, false, pp);
    }
    float GGX_GFunction(float cosNX, float sqRoughness) // ѕотер€нный свет
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
    /// <summary>
    /// 
    /// </summary>
    /// <param name="cosNL"> - angle between the light direction and the normal</param>
    /// <param name="n1"> - index of refreaction "from" medium</param>
    /// <param name="n2"> - index of refreaction "to" medium</param>
    /// <returns></returns>
    float FresnelSchlick(float cosNL, float n1, float n2) {
        float f0 = std::pow((n1 - n2) / (n1 + n2), 2);
        return f0 + (1.f - f0) * pow(1.f - cosNL, 5.f);
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