#pragma once
#include "Photon.h"
#include "Tools.h"
#include <iostream>
#include "PMModel.h"
#include "GLM/glm.hpp"
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
    /// ph � photon map,
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
            && diffuse_surfs < 1) {
            return true;
        } 
        if (lastPathType == PathType::dif_refl) {
            return false;
        }
        return false; // �� �������� �� ����� � ������ ������ �������
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
    struct StackContent {
        DeepLookStack<std::pair<float, size_t>> mediums;
        bool exiting;
        StackContent() {}
        StackContent(const DeepLookStack<std::pair<float, size_t>>& mediums, bool exiting) {
            this->mediums = mediums;
            this->exiting = exiting;
        }
    };
    //DeepLookStack<std::pair<float, size_t>> mediums;
    std::stack<StackContent> st_mediums;
    float default_refr;
    /// <summary>
    /// ca_table � critical angle table.
    /// Map with critical angles for each medium pair in the scene.
    /// <param name="Key"> is the division of the refractive coefficients of the two media through which light passes</param>
    /// <param name="Value"> is a critical angle for this mediums in radians</param>
    /// </summary>
    std::map<std::pair<float,float>, float> ca_table;
public:
    MediumManager(float def_refr = 1.f) : default_refr(def_refr) {
        clear();
    }
    void compute_critical_angles(const PMScene& scene)
    {
        float eta1, eta2, eta, ca;
        for (int i = 0; i < (int)scene.objects.size() - 1; i++) {
            for (int j = i + 1; j < (int)scene.objects.size(); j++) {
                eta1 = scene.objects[i].get_material()->refr_index;
                eta2 = scene.objects[j].get_material()->refr_index;
                eta = eta2 / eta1; // from eta1 medium to eta2 medium
                if (eta <= 1.f && ca_table.find({ eta1, eta2 }) == ca_table.end()) {
                    ca = std::cos(std::asin(eta));
                    ca_table[{eta1, eta2}] = ca;
                }
                eta = eta1 / eta2; // from eta2 medium to eta1 medium
                if (eta <= 1.f && ca_table.find({ eta2, eta1 }) == ca_table.end()) {
                    ca = std::cos(std::asin(eta));
                    ca_table[{eta2, eta1}] = ca;
                }
            }
        }
        for (int i = 0; i < scene.objects.size(); i++) {
            eta1 = scene.objects[i].get_material()->refr_index;
            eta = default_refr / eta1; // from eta1 medium to eta2 medium
            if (eta <= 1.f && ca_table.find({ eta1, default_refr }) == ca_table.end()) {
                ca = std::cos(std::asin(eta));
                ca_table[{eta1, eta2}] = ca;
            }
            eta = eta1 / default_refr; // from eta2 medium to eta1 medium
            if (eta <= 1.f && ca_table.find({ default_refr , eta1 }) == ca_table.end()) {
                ca = std::cos(std::asin(eta));
                ca_table[{eta2, eta1}] = ca;
            }
        }
    };
    bool can_refract(const std::pair<float, float>& cn, float cosNL) {
        if (ca_table.find(cn) == ca_table.end()) {
            return true;
        }
        return cosNL > ca_table[cn]; // <= ?
    }
    std::pair<float, float> get_cur_new(const PMModel* model) {
        std::pair<float, float> res;
        auto& mediums = st_mediums.top().mediums;
        auto& exiting = st_mediums.top().exiting;
        if (model->equal(mediums.peek().second)) {
            // ���� ��� ���������� � ��������, � ������� �� ���������, � ���������� �������
            // ���� ������� ������� �� ��������� � ������� �����. �� ������� ����� ����� ������, 
            // � ������� ��������� ���
            // ������� ����� - ����� �������, �� �������� ������ ���, �.�. �� ������
            res.first = model->get_material()->refr_index;
            res.second = mediums.peek(1).first;
            exiting = true;
            return res;
        }
        // ���� ��� ������ � ����� ������
        res.first = mediums.peek().first;
        res.second = model->get_material()->refr_index;
        exiting = false;
        return res;
    }
    void inform(bool refract_suc, const PMModel* model) {
        if (!refract_suc) {
            return;
        }
        auto& mediums = st_mediums.top().mediums;
        auto& exiting = st_mediums.top().exiting;
        if (exiting && model->equal(mediums.peek().second)) {
            // ���� ��� ����� �� �������, �� ������� �� ����� ������� �����
            mediums.pop();
            exiting = false;
        }
        else { // ����� ��� ������� ��� ���� ������, ��������� ������� �����
            mediums.push({ model->get_material()->refr_index, model->get_id() });
        }
    }
    void increase_depth() {
        st_mediums.push(st_mediums.top());
    }
    void reduce_depth() {
        st_mediums.pop();
    }
    void clear() {
        auto default_m = DeepLookStack<std::pair<float, size_t>>();
        default_m.push({ default_refr, 0 });
        st_mediums = std::stack<StackContent>();
        st_mediums.push(StackContent(default_m, false));
    }
};