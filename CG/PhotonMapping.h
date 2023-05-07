#pragma once
#include "Light.h"
#include "Tools.h"
#include "PMModel.h"
#include <stack>
#define _USE_MATH_DEFINES
#include <math.h>
#include "PhotonMap.h"
#include "Photon.h"
#include "PMTools.h"
#include "Texture.h"
class PhotonMapping {
    struct Settings {
        // disable photon mapping for direct illumination
        bool dpmdi = false;
        float exposure = 1.f;
        float brightness = 1.f;
        float gamma = 2.2f;
        int max_rt_depth = 2;
        size_t phc; // Emitted photoms capacity
    };
private:
    // Stored photons for global illumination map
    std::vector<Photon> global_sp;
    // Stored photons for caustic illumination map
    std::vector<Photon> caustic_sp;
    PhotonMap global_map;
    PhotonMap caustic_map;
    DeepLookStack<std::pair<const Material*, size_t>> mediums;
    Material default_medium;
    PMScene scene;
    std::vector<LightSource> lsources;
    PathOperator path_operator;
    Settings settings;
    CImgTexture* canvas;
    /// <summary>
    /// ca_table — critical angle table.
    /// Map with critical angles for each medium pair in the scene.
    /// <param name="Key"> is the division of the refractive coefficients of the two media through which light passes</param>
    /// <param name="Value"> is a critical angle for this mediums in radians</param>
    /// </summary>
    std::map<float, float> ca_table; 
    void emit(const LightSource& ls);
    void compute_critical_angles();
    bool refract(float cosNL, const PMModel* ipmm);
    bool find_intersection(const Ray& ray, bool reverse_normal, 
        PMModel*& imodel, glm::vec3& normal, glm::vec3& inter_p);
    /// <summary>
    /// Возвращает значение, будет ли фотон отражен диффузно, зеркально или вовсе рассеян
    /// </summary>
    /// <param name="om">Origin material - Материал поверхности, изначальной поверхности</param>
    /// <param name="ipmm">Incident PMModel - Материал поверхности, на которую попал фотон</param>
    /// <param name="ipp">Light photon power — параметр испущенного из источника света фотона в RGB </param>
    /// <returns></returns>
    PathType destiny(float cosNL, const PMModel* ipmm, const glm::vec3& lphoton);
    /// <summary>
    /// Функция трассировки луча для составления фотонных карт
    /// </summary>
    /// <param name="ray">- ray...</param>
    /// <param name="pp">- photon power</param>
    void trace(const Ray& ray, bool in_object, const glm::vec3& pp);
    /// <summary>
    /// Функция трассировки луча для непосредственного рендеринга
    /// </summary>
    /// <param name="ray">- ray...</param>
    glm::vec3 render_trace(const Ray& ray, bool in_object, int depth);
    float BRDF(glm::vec3 direction, glm::vec3 location, glm::vec3 normal, const Material* mat);
    float GGX_GFunction(float cosNX, float sqRoughness); // Потерянный свет
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
    void hdr(glm::vec3& dest);
public:
    PhotonMapping();
    void init(CImgTexture* canvas, const std::vector<PMModel>& objects,
        const std::vector<LightSource>& lsources);
    void build_map();
    void render();
    void update_exposure(float exposure);
    void update_brightness(float brightness);
    void update_ls_intensity(const glm::vec3& intensity);
    /// <summary>
    /// Update disabling photon mapping for direct illumination parameter
    /// </summary>
    /// <param name="value"> - if true — disable. if false - enable</param>
    /// <returns></returns>
    void update_dpmdi(bool value);
    /// <summary>
    /// Update the number of photons to be emitted
    /// </summary>
    /// <param name="phc"> - photons count</param>
    void update_phc(size_t phc);
    void update_gnp_count(size_t count);
    void update_cnp_count(size_t count);
};