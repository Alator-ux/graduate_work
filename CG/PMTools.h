#pragma once
#include "Photon.h"
#include "Tools.h"
#include "PMModel.h"
#include <iostream>
/* Photon Book page 85
Is this simple visualization a full solution to the rendering equation? To
answer this question we can look at the paths traced by the photons and
the rays and see if they cover the space of all paths.
L(S|D)*D are all the paths represented by the photon map.
(LS*E)|(DS*E) are all the paths traced by the ray tracer.*/
/* o - empty node
*      
*      ^ -> s
*      |    |
*      |    v
* L -> o <- o
*      |    ^
*      |    |
*      v -> d -> pm
* 
* 
*      ^ -> d
*      |    | <- ^
*      |    v    |
* L -> o -> o -> s
*           |
*           v -> e -> rt
* 
*/
class PathOperator
{
    int diffuse_surfs;
    PathType lastPathType;
public:
    /// <summary>
    /// ph — photon map,
    /// rt - ray tracer
    /// </summary>
    PathOperator() {
        clear();
    }
    void inform(PathType pt) {
        if (pt == PathType::dif_refl) {
            diffuse_surfs++;
        }
        lastPathType = pt;
    }
    /// <summary>
    /// Returns true if caustic map needs to be filled in
    /// </summary>
    bool response() const {
        if ((lastPathType == PathType::spec_refl || lastPathType == PathType::refr)
            && diffuse_surfs < 2) {
            return true;
        } 
        if (lastPathType == PathType::dif_refl) {
            return false;
        }
        return false; // мб заменить на енамы и другой ретерн сделать
        //return ;
    }
    void clear() {
        diffuse_surfs = 0;
        lastPathType = PathType::none;
    }
};
struct PhotonCollector {
    size_t gsize, csize;
    // Stored photons for global illumination map
    std::vector<Photon> global;
    // Stored photons for caustic illumination map
    std::vector<Photon> caustic;
    PhotonCollector() : global(), caustic(), gsize(0), csize(0) {}
    PhotonCollector(size_t gsize, size_t csize) : global(), caustic(), gsize(gsize), csize(csize) { }
    void push(const Photon& photon, const PathOperator& po) {
        if (global.size() < gsize) {
            global.push_back(photon);
        }
        if (po.response() && (caustic.size() < csize)) {
            caustic.push_back(photon);
        }
    }
    bool unfilled() {
        return global.size() < gsize || caustic.size() < csize;
    }
    void clear(){
        global.clear();
        caustic.clear();
    }
    void update_gsize(size_t size) {
        gsize = size;
        global = std::vector<Photon>();

    }
    void update_csize(size_t size) {
        csize = size;
        caustic = std::vector<Photon>();
    }
    void pring_logs() {
        std::cout << "Collected photons for global map: " << global.size() << " out of " << gsize << std::endl;
        std::cout << "Collected photons for caustic map: " << caustic.size() << " out of " << csize << std::endl;
    }
};
class MediumManager {
    DeepLookStack<std::pair<float, size_t>> mediums;
    float default_refr;
    bool exiting;
    /// <summary>
    /// ca_table — critical angle table.
    /// Map with critical angles for each medium pair in the scene.
    /// <param name="Key"> is the division of the refractive coefficients of the two media through which light passes</param>
    /// <param name="Value"> is a critical angle for this mediums in radians</param>
    /// </summary>
    std::map<std::pair<float,float>, float> ca_table;

public:
    MediumManager(float def_refr = 1.f) : mediums(), exiting(false), default_refr(def_refr) {}
    void compute_critical_angles(const PMScene& scene)
    {
        float eta1, eta2, eta, ca;
        for (int i = 0; i < (int)scene.objects.size() - 1; i++) {
            for (int j = i + 1; j < (int)scene.objects.size(); j++) {
                eta1 = scene.objects[i].get_material()->refr_index;
                eta2 = scene.objects[j].get_material()->refr_index;
                eta = eta2 / eta1; // from eta1 medium to eta2 medium
                if (eta <= 1.f && ca_table.find({ eta1, eta2 }) == ca_table.end()) {
                    ca = std::asin(eta);
                    ca_table[{eta1, eta2}] = ca;
                }
                eta = eta1 / eta2; // from eta2 medium to eta1 medium
                if (eta <= 1.f && ca_table.find({ eta2, eta1 }) == ca_table.end()) {
                    ca = std::asin(eta);
                    ca_table[{eta2, eta1}] = ca;
                }
            }
        }
        for (int i = 0; i < scene.objects.size(); i++) {
            eta1 = scene.objects[i].get_material()->refr_index;
            eta = default_refr / eta1; // from eta1 medium to eta2 medium
            if (eta <= 1.f && ca_table.find({ eta1, default_refr }) == ca_table.end()) {
                ca = std::asin(eta);
                ca_table[{eta1, eta2}] = ca;
            }
            eta = eta1 / default_refr; // from eta2 medium to eta1 medium
            if (eta <= 1.f && ca_table.find({ default_refr , eta1 }) == ca_table.end()) {
                ca = std::asin(eta);
                ca_table[{eta2, eta1}] = ca;
            }
        }
    };
    bool can_refract(const std::pair<float, float>& cn, float cosNL) {
        if (ca_table.find(cn) == ca_table.end()) {
            return true;
        }
        float angle = std::acos(cosNL);
        return angle < ca_table[cn]; // <= ?
    }
    std::pair<float, float> get_cur_new(const PMModel* model) {
        std::pair<float, float> res;
        if (model->equal(mediums.peek().second)) {
            // Если луч столкнулся с объектом, в котором он находится, с внутренней стороны
            // Надо достать внешнюю по отношению к объекту среду. На вершине стека лежит объект, 
            // в котором находится луч
            // текущая среда - среда объекта, по которому ударил луч, т.к. мы внутри
            res.first = model->get_material()->refr_index;
            res.second = mediums.peek(1).first;
            exiting = true;
            return res;
        }
        // Если луч входит в новый объект
        res.first = mediums.peek().first;
        res.second = model->get_material()->refr_index;
        exiting = false;
        return res;
    }
    void inform(bool refract_suc, const PMModel* model) {
        if (refract_suc) {
            if (exiting) { // если луч вышел из объекта, то убираем со стека текущую среду
                mediums.pop();
                exiting = false;
            }
            else { // иначе луч пересек еще один объект, добавляем текущую среду
                mediums.push({ model->get_material()->refr_index, model->get_id() });
            }
        }
    }
    void clear() {
        mediums = DeepLookStack<std::pair<float, size_t>>();
        mediums.push({ default_refr, 0 });
    }
};