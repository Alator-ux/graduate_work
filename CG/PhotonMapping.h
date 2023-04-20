#pragma once
#include "Light.h"
#include "Tools.h"
#include "PMModel.h"
#include <stack>
#define _USE_MATH_DEFINES
#include <math.h>
#include "PhotonMap.h"
#include "Photon.h"
#include "PathOperator.h"
#include "Texture.h"
class PhotonMapping {
public:
    
private:
    std::vector<Photon> stored_photons;
    PhotonMap photon_map;
    DeepLookStack<std::pair<const Material*, size_t>> mediums;
    PMScene scene;
    std::vector<LightSource> lsources;
    CImgTexture* canvas;
    Material default_medium;
    /// <summary>
    /// ca_table � critical angle table.
    /// Map with critical angles for each medium pair in the scene.
    /// <param name="Key"> is the division of the refractive coefficients of the two media through which light passes</param>
    /// <param name="Value"> is a critical angle for this mediums in radians</param>
    /// </summary>
    std::map<float, float> ca_table; 
    size_t phc; // Emitted photoms capacity
    void emit(const LightSource& ls);
    void compute_critical_angles();
    bool refract(float cosNL, const PMModel* ipmm);
    bool find_intersection(const Ray& ray, PMModel*& imodel, glm::vec3& normal, glm::vec3& inter_p);
    /// <summary>
    /// ���������� ��������, ����� �� ����� ������� ��������, ��������� ��� ����� �������
    /// </summary>
    /// <param name="om">Origin material - �������� �����������, ����������� �����������</param>
    /// <param name="ipmm">Incident PMModel - �������� �����������, �� ������� ����� �����</param>
    /// <param name="ipp">Light photon power � �������� ����������� �� ��������� ����� ������ � RGB </param>
    /// <param name="ipp">Incident photon power � �������� ��������� ������ � RGB </param>
    /// <returns></returns>
    PathType destiny(float cosNL, const PMModel* ipmm, const glm::vec3& lphoton, glm::vec3& ipp);
    /// <summary>
    /// ������� ����������� ���� ��� ����������� �������� ����
    /// </summary>
    /// <param name="ray">- ray...</param>
    /// <param name="pp">- photon power</param>
    void trace(const Ray& ray, bool in_object, const glm::vec3& pp);
    /// <summary>
    /// ������� ����������� ���� ��� ����������������� ����������
    /// </summary>
    /// <param name="ray">- ray...</param>
    glm::vec3 render_trace(const Ray& ray);
    float BRDF(glm::vec3 direction, glm::vec3 location, glm::vec3 normal, const Material* mat);
    float GGX_GFunction(float cosNX, float sqRoughness); // ���������� ����
    float GGX_DFunction(float cosNX, float sqRoughness);
    /// <summary>
    /// Returns amount of reflected light
    /// </summary>
    /// <param name="cosNL"> - angle between the light direction and the normal</param>
    /// <param name="n1"> - index of refreaction "from" medium</param>
    /// <param name="n2"> - index of refreaction "to" medium</param>
    /// <returns></returns>
    float FresnelSchlick(float cosNL, float n1, float n2);
    glm::vec3 FresnelSchlick(float cosNL, glm::vec3 F0);
    glm::vec3 CookTorrance_GGX(float NdotL, float NdotV, float NdotH, glm::vec3 F, float roughness);
    void clear_mediums();
    
public:
    PhotonMapping(CImgTexture* canvas, const std::vector<PMModel>& objects, 
        const std::vector<LightSource>& lsources, size_t phc);
    void build_map();
    void render();
};