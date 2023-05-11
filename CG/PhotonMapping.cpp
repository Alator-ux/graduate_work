#include "PhotonMapping.h"
/* ========== PhotonMapping class begin ========== */
PhotonMapping::PhotonMapping() :
    global_map(PhotonMap::Type::def), caustic_map(PhotonMap::Type::caustic) {}
void PhotonMapping::init(CImgTexture* canvas, const std::vector<PMModel>& objects,
    const std::vector<LightSource>& lsources) {
    this->scene = std::move(PMScene(objects));
    this->lsources = lsources;
    this->canvas = canvas;
    this->medium_manager.compute_critical_angles(this->scene);
}
void PhotonMapping::emit(const PMModel& ls) {
    size_t ne = 0;// Number of emitted photons
    Ray ray;
    glm::vec3 normal;
    while (photon_collector.unfilled()) {
        medium_manager.clear();
        path_operator.clear();
        auto p = ls.get_radiation_info();
        ray.origin = p.first + p.second * 0.0001f;
        float prob;
        do{
            ray.dir.x = Random<float>::random(-1.f, 1.f);
            ray.dir.y = Random<float>::random(-1.f, 1.f);
            ray.dir.z = Random<float>::random(-1.f, 1.f);
            ray.dir = glm::normalize(ray.dir);
            prob = Random<float>::random(0.f, 1.f);
        } while (prob > glm::dot(ray.dir, p.second));
        auto pp = ls.get_ls()->intensity / (float)photon_collector.gsize; // photon power
        trace(ray, false, pp);
        ne++;
        if (ne % (photon_collector.gsize / 10) == 0) {
            std::cout << "\tPhotons emited: " << ne << std::endl;
            photon_collector.pring_logs();
        }
    }
}
bool PhotonMapping::refract(float cosNL, const PMModel* ipmm) {
    const Material* im = ipmm->get_material();
    /*
    * Поверхность, на которую попал фотон, должна иметь не единичный показатель преломления, иначе считаем,
    * что фотон будет просто отражен. Если же показатели преломления равны, то это означает,
    * что преломления так же не должно быть. Например, фотон попал из воды в воду.
    */
    if (im->opaque == 1.f) {
        return false;
    }

    auto cn = medium_manager.get_cur_new(ipmm);
    if (!medium_manager.can_refract(cn, cosNL)) {
        return false;
    }
    float refr_probability;
    refr_probability = 1.f - im->opaque;
    /* ЗАКОНСЕРВИРОВАННО ДО ТЕСТА И ПОНИМАНИЯ
    float divn2n1 = im->refr_index / mat_ind.first->refr_index;
    if (ca_table.find(divn2n1) != ca_table.end()) {
        float critical_angle = ca_table[divn2n1];
        float angle = std::acos(cosNL);
        // Результат деления может быть > 1, в таком случае вероятность преломления должна быть равна 0.
        // Чем меньше угол падения, тем больше вероятность отражения
        refr_probability = 1.f - angle / critical_angle;
    }
    else {
        // Количество преломленного света описывается этим выражением.
        refr_probability = 1.f - FresnelSchlick(cosNL, mat_ind.first->refr_index, im->refr_index);
    }*/

    float e = Random<float>::random(0.f, 1.f);
    if (e > refr_probability) { // если выпала участь преломиться
        return false;
    }
    return true;
}
PathType PhotonMapping::destiny(float cosNL, const PMModel* ipmm, const glm::vec3& lphoton) {
    if (refract(cosNL, ipmm)) {
        return PathType::refr;
    }
    float e;
    const Material* im = ipmm->get_material();

    auto max_lp = std::max(std::max(lphoton.r, lphoton.g), lphoton.b);
    e = Random<float>::random(0.f, 2.f); // upper bound = 2.f because max|d + s| = 2

    auto lp_d = im->diffuse * lphoton;
    auto max_lp_d = std::max(std::max(lp_d.r, lp_d.g), lp_d.b);
    auto pd = max_lp_d / max_lp;
    if (e <= pd) {
        return PathType::dif_refl;
    }

    auto lp_s = im->specular * lphoton;
    auto max_lp_s = std::max(std::max(lp_s.r, lp_s.g), lp_s.b);
    auto ps = max_lp_s / max_lp;
    if (e <= pd + ps) {
        return PathType::spec_refl;
    }

    return PathType::absorption;
}
bool PhotonMapping::find_intersection(const Ray& ray, bool reverse_normal,
    PMModel*& imodel, glm::vec3& normal, glm::vec3& inter_p) {
    float inter = 0.f;
    imodel = nullptr;
    glm::vec3 inter_ind;
    for (PMModel& model : scene.objects) {
        float temp_inter;
        glm::vec3 temp_ind;
        bool succ = model.intersection(ray, false, temp_inter, temp_ind); // TODO in_object скорее всего дропнуть
        if (succ && (inter == 0.f || temp_inter < inter)) {
            inter = temp_inter;
            inter_ind = temp_ind;
            imodel = &model;
        }
    }
    if (imodel == nullptr) {
        return false;
    }
    inter_p = ray.origin + ray.dir * inter;
    imodel->get_normal(inter_ind, inter_p, normal);
    if (glm::dot(ray.dir, normal) > 0) { // reverse_normal &&
        normal *= -1.f;
    }
    return true;
}
void PhotonMapping::trace(const Ray& ray, bool in_object, const glm::vec3& pp) {
    glm::vec3 normal, inter_p;
    // Incident model
    PMModel* imodel;
    if (!find_intersection(ray, in_object, imodel, normal, inter_p)) {
        return;
    }
    Ray new_ray;
    float cosNL = glm::dot(-ray.dir, normal);
    PathType dest = destiny(cosNL, imodel, pp);
    switch (dest) {
    case PathType::refr:
    {
        float refr1, refr2;
        auto cn = medium_manager.get_cur_new(imodel);
        bool succ = ray.refract(inter_p, normal, cn.first, cn.second, new_ray);
        if (!succ) {
            throw std::exception("ЧЗХ?");
        }
        medium_manager.inform(succ, imodel);
        in_object = !in_object;
        break;
    }
    case PathType::dif_refl:
        photon_collector.push(Photon(inter_p, pp, ray.dir), path_operator);
        new_ray = ray.reflect_spherical(inter_p, normal);
        break;
    case PathType::spec_refl:
        new_ray = ray.reflect(inter_p, normal);
        break;
    case PathType::absorption:
        if (imodel->get_material()->specular != glm::vec3(1.f)) {
            photon_collector.push(Photon(inter_p, pp, ray.dir), path_operator);
        }
        return;
    default:
        break;
    }
    path_operator.inform(dest);
    trace(new_ray, in_object, pp);
}
float PhotonMapping::FresnelSchlick(float cosNL, float n1, float n2) {
    float f0 = std::pow((n1 - n2) / (n1 + n2), 2);
    return f0 + (1.f - f0) * pow(1.f - cosNL, 5.f);
}
void PhotonMapping::hdr(glm::vec3& dest) {
    dest = glm::vec3(1.f) - glm::exp(-dest * settings.exposure);
    dest = glm::pow(dest, glm::vec3(1.f / settings.gamma));
}
void PhotonMapping::build_map() {
    std::cout << "Photon emission started" << std::endl;
    size_t count = 0;
    for (size_t i = 0; i < scene.objects.size(); i++) {
        if (scene.objects[i].get_ls() != nullptr) {
            std::cout << "Light source " << count << std::endl;
            emit(scene.objects[i]);
            count++;
        }
    }
    std::cout << "Photon emission ended" << std::endl;
    std::cout << "Global map:" << std::endl;
    global_map.clear();
    global_map.fill_balanced(photon_collector.global);
    std::cout << "Caustic map:" << std::endl;
    caustic_map.clear();
    caustic_map.fill_balanced(photon_collector.caustic);
    photon_collector.clear();
    return ;
}
glm::vec3 PhotonMapping::render_trace(const Ray& ray, bool in_object, int depth) {
    glm::vec3 res(0.f);
    if (depth > settings.max_rt_depth) {
        return res;
    }
    glm::vec3 normal, inter_p;
    PMModel* imodel;
    if (!find_intersection(ray, in_object, imodel, normal, inter_p)) {
        return res;
    }
    if (depth == 0 && imodel->name != "rightSphere") {
     //   return res;
    }
    const Material* mat = imodel->get_material();

    if (false && mat->diffuse != glm::vec3(0.f)) {
        glm::vec3 re(0.f);
        int lcount = 0;
        glm::vec3 tnormal, tinter_p;
        PMModel* timodel;
        Ray tray;
        for (auto& ls : lsources) {
            tray.origin = ls.position;
            tray.dir = glm::normalize(inter_p - ls.position); // point <- ls
            if (find_intersection(tray, in_object, timodel, tnormal, tinter_p) && vec3_equal(inter_p, tinter_p)) {
                lcount++;
                re += glm::max(glm::dot(normal, -tray.dir), 0.f);
            }
        }
        re /= lcount == 0 ? 1 : lcount;
        if (settings.dpmdi) {
            if (lcount == 0) {
                global_map.radiance_estimate(ray.dir, inter_p, normal, re);
            }
        }
        else {
            res += re;
            global_map.radiance_estimate(ray.dir, inter_p, normal, re);
        }
        res += re;
        res *= mat->diffuse;
    }
    if (false && !in_object && mat->specular != glm::vec3(0.f)) {
        Ray nray = ray.reflect(inter_p, normal);
        auto t = render_trace(nray, in_object, depth+1);
        //glm::vec3 halfway = glm::normalize(nray.dir - ray.dir); // nray.dir + (-ray.dir)
        //float coef = glm::pow(glm::max(glm::dot(normal, halfway), 0.f), mat->shininess);
        //res += t * mat->specular * coef;
        res += t * mat->specular;
    }
    if(mat->opaque != 1.f) {
        auto cn = medium_manager.get_cur_new(imodel);
        Ray nray;
        bool succ = ray.refract(inter_p, normal,
                cn.first, cn.second, nray);
        if (succ) {
            medium_manager.inform(true, imodel);
            auto t = render_trace(nray, !in_object, depth+1);
            res = glm::mix(t, res, mat->opaque); // t * (1 - opaque) + res * opaque
        }
    }
    res += mat->emission;
    glm::vec3 caustic(0.f);
    if (mat->opaque == 1.f && caustic_map.radiance_estimate(ray.dir, inter_p, normal, caustic)) {
        res += caustic;
    }
    return res;
}
void PhotonMapping::render() {
    canvas->clear();
    std::cout << "Rendering has started" << std::endl;
    float width = canvas->get_width();
    float height = canvas->get_height();
    constexpr float fov = glm::radians(60.f);
    float step_x = 0.5f - width / 2.f;
    float step_y = -0.5f + height / 2.f;
    float dir_z = -height / (2.f * tan(fov / 2.f));

    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            medium_manager.clear();
            float dir_x = i + step_x;
            float dir_y = -j + step_y;
            glm::vec3 dir = glm::normalize(glm::vec3(dir_x, dir_y, dir_z));
            Ray ray(scene.camera - scene.normal * 1.5f, dir);
            glm::vec3 color = render_trace(ray, false, 0);
            if (color == glm::vec3(-1.f)) {
                color = render_trace(ray, false, 0);
            }
            hdr(color);
            color *= settings.brightness;
            canvas->set_rgb(i, j, color * 255.f);
        }
        if (j % ((size_t)height / 50) == 0) {
            std::cout << "\tPixels filled: " << (j + 1) * width << " of " << width * height << std::endl;        }
    }
    global_map.total_locate_time();
    std::cout << "Rendering has ended" << std::endl;
}
float PhotonMapping::BRDF(glm::vec3 direction, glm::vec3 location, glm::vec3 normal, const Material* mat) {
    glm::vec3 halfVector = glm::normalize(direction + glm::normalize(location));
    float NdotH = glm::max(glm::dot(normal, halfVector), 0.0f);
    float NdotL = glm::max(glm::dot(normal, direction), 0.0f);
    float HdotL = glm::max(glm::dot(halfVector, direction), 0.0f);
    float shininess = mat->shininess;
    float specular = glm::pow(NdotH, shininess);
    float diffuse = NdotL;
    return (diffuse / glm::pi<float>() + specular) * HdotL;
}
// Пока сомнительно
float PhotonMapping::GGX_GFunction(float cosNX, float sqRoughness) // Потерянный свет
{
    float alpha = sqRoughness * sqRoughness;
    float sqCosNX = cosNX * cosNX;
    float sqTan = 1 / sqCosNX - 1; // 1+ tan^2 = 1 / cos^2
    float res = 2 / (1 + sqrt(1 + alpha * sqTan));
    return res;
}
float PhotonMapping::GGX_DFunction(float cosNX, float sqRoughness)
{
    float alpha = sqRoughness * sqRoughness;
    float sqCosNX = cosNX * cosNX;
    float fourthCosNX = sqCosNX * sqCosNX;
    float sqTan = 1 / sqCosNX - 1; // 1+ tan^2 = 1 / cos^2
    float part = (alpha + sqTan);
    float res = alpha / (M_PI * fourthCosNX * part * part);
    return res;
}
glm::vec3 PhotonMapping::FresnelSchlick(float cosNL, glm::vec3 F0)
{
    return F0 + (glm::vec3(1.f) - F0) * pow(1.f - cosNL, 5.f);
}
glm::vec3 PhotonMapping::CookTorrance_GGX(float NdotL, float NdotV, float NdotH, glm::vec3 F, float roughness)
{
    if (NdotL <= 0.0 || NdotV <= 0.0) return glm::vec3(0.0);

    float sqRoughness = roughness * roughness;
    float GCoeff = GGX_GFunction(NdotL, sqRoughness) * GGX_GFunction(NdotV, sqRoughness);
    float DCoeff = GGX_DFunction(NdotH, sqRoughness);

    return glm::max(GCoeff * DCoeff * F * 0.25f / NdotV, 0.f);
}
void PhotonMapping::update_exposure(float exposure) {
    settings.exposure = exposure;
}
void PhotonMapping::update_brightness(float brightness) {
    settings.brightness = brightness;
}
void PhotonMapping::update_ls_intensity(const glm::vec3& intensity) {
    for(auto& m : scene.objects){
        if (m.get_ls() != nullptr) {
            m.set_light_intensity(intensity);
        }
    }
}
void PhotonMapping::update_dpmdi(bool value) {
    settings.dpmdi = value;
}
void PhotonMapping::update_gphc(size_t gphc) {
    photon_collector.update_gsize(gphc);
}
void PhotonMapping::update_cphc(size_t cphc) {
    photon_collector.update_csize(cphc);
}
void PhotonMapping::update_gnp_count(size_t count) {
    global_map.update_np_size(count);
}
void PhotonMapping::update_cnp_count(size_t count) {
    caustic_map.update_np_size(count);
}
void PhotonMapping::update_disc_compression(float coef) {
    global_map.update_disc_compression(coef);
    caustic_map.update_disc_compression(coef);
}