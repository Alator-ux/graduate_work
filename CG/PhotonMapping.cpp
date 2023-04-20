#include "PhotonMapping.h"
/* ========== PhotonMapping class begin ========== */
PhotonMapping::PhotonMapping(CImgTexture* canvas, const std::vector<PMModel>& objects, 
    const std::vector<LightSource>& lsources, size_t phc) {
    this->scene = PMScene(objects);
    this->lsources = lsources;
    this->canvas = canvas;
    this->phc = phc;
    this->stored_photons = std::vector<Photon>();
    this->ca_table = std::map<float, float>();
    this->default_medium = Material();
    this->default_medium.refr_index = 1.f;
    this->mediums.push({ &default_medium, 0 });
    compute_critical_angles();
}
void PhotonMapping::clear_mediums() {
    this->mediums = {};
    this->mediums.push({ &default_medium, 0 });
}
void PhotonMapping::emit(const LightSource& ls) {
    size_t ne = 0;// Number of emitted photons
    while (ne < phc) {
        clear_mediums();
        float x, y, z;
        do {
            x = Random<float>::random(-1.f, 1.f);
            y = Random<float>::random(-1.f, 0.f); // В конкретном случае свет должен светить только вниз Random<float>::random(-1.f, 1.f);
            z = Random<float>::random(-1.f, 1.f);
        } while (x * x + y * y + z * z > 1.f); // TODO normalize ?
        Ray ray(ls.position, { x,y,z });
        auto pp = ls.diffuse; // photon power
        trace(ray, false, pp);
        ne++;
        if (ne % 500 == 0) {
            std::cout << "Photons emited: " << ne << std::endl;
        }
    }
}
void PhotonMapping::compute_critical_angles()
{
    for (int i = 0; i < (int)scene.objects.size() - 1; i++) {
        for (int j = i + 1; j < (int)scene.objects.size(); j++) {
            float eta1 = scene.objects[i].get_material()->refr_index;
            float eta2 = scene.objects[j].get_material()->refr_index;
            float eta = eta2 / eta1; // from eta1 medium to eta2 medium
            if (eta <= 1.f && ca_table.find(eta) == ca_table.end()) {
                float ca = std::asin(eta);
                ca_table[eta] = ca;
            }
            eta = eta1 / eta2; // from eta2 medium to eta1 medium
            if (eta <= 1.f && ca_table.find(eta) == ca_table.end()) {
                float ca = std::asin(eta);
                ca_table[eta] = ca;
            }
        }
    }
}
bool PhotonMapping::refract(float cosNL, const PMModel* ipmm) {
    const Material* im = ipmm->get_material();
    auto mat_ind = mediums.peek();
    /*
    * Поверхность, на которую попал фотон, должна иметь не единичный показатель преломления, иначе считаем,
    * что фотон будет просто отражен. Если же показатели преломления равны, то это означает,
    * что преломления так же не должно быть. Например, фотон попал из воды в воду.
    */
    if (im->refr_index != 1.f) {
        // Если луч столкнулся с объектом, в котором он находится, с внутренней стороны
        if (ipmm->equal(mat_ind.second)) {
            // Надо достать внешнюю по отношению к объекту среду. На вершине стека лежит объект, 
            // в котором находится луч 
            mat_ind = mediums.peek(1);
        }
        float refr_probability;
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
        }

        float e = Random<float>::random(0.f, 1.f);
        if (e > refr_probability) { // если выпала участь преломиться
            return false;
        }

        if (ipmm->equal(mat_ind.second)) { // если луч вышел из объекта, то убираем со стека текущую среду
            mediums.pop();
        }
        else { // иначе луч пересек еще один объект, добавляем текущую среду
            mediums.push({ im, ipmm->get_id() });
        }
        return true;
    }
    return false;
}
PathType PhotonMapping::destiny(float cosNL, const PMModel* ipmm, const glm::vec3& lphoton, glm::vec3& ipp) {
    if (refract(cosNL, ipmm)) {
        return PathType::refr;
    }
    float e;
    const Material* im = ipmm->get_material();

    auto max_ipp = std::max(std::max(ipp.r, ipp.g), ipp.b);
    e = Random<float>::random(0.f, 2.f); // upper bound = 2.f because max|d + s| = 2

    auto ipp_d = im->diffuse * ipp;
    auto max_ipp_d = std::max(std::max(ipp_d.r, ipp_d.g), ipp_d.b);
    auto pd = max_ipp_d / max_ipp;
    if (e <= pd) {
        ipp = lphoton * im->diffuse; //im->diffuse / pd; // diffuse reflection
        if (ipp.x == ipp.y && ipp.z == ipp.y) {
            auto a = 1;
        }
        return PathType::dif_refl;
    }

    auto ipp_s = im->specular * ipp;
    auto max_ipp_s = std::max(std::max(ipp_s.r, ipp_s.g), ipp_s.b);
    auto ps = max_ipp_s / max_ipp;
    if (e <= pd + ps) {
        //ipp *= im->specular / ps; // specular reflection
        return PathType::spec_refl;
    }

    ipp = lphoton * im->diffuse;
    return PathType::absorption;
}
bool PhotonMapping::find_intersection(const Ray& ray, PMModel*& imodel, glm::vec3& normal, glm::vec3& inter_p) {
    float inter = 0.f;
    imodel = nullptr;
    for (PMModel& model : scene.objects) {
        float temp_inter;
        glm::vec3 temp_normal;
        bool succ = model.intersection(ray, false, temp_inter, temp_normal); // TODO in_object скорее всего дропнуть
        if (succ && (inter == 0.f || temp_inter < inter)) {
            inter = temp_inter;
            normal = temp_normal;
            imodel = &model;
        }
    }
    if (imodel == nullptr) {
        return false;
    }
    inter_p = ray.origin + ray.dir * inter;
    return true;
}
void PhotonMapping::trace(const Ray& ray, bool in_object, const glm::vec3& pp) {
    glm::vec3 normal, inter_p;
    // Incident model
    PMModel* imodel;
    if (!find_intersection(ray, imodel, normal, inter_p)) {
        return;
    }
    if (glm::dot(ray.dir, normal) > 0) {
        normal *= -1.f;
    }
    Ray new_ray;
    float cosNL = glm::dot(-ray.dir, normal);
    auto cur_mi = mediums.peek(); // current material and id
    glm::vec3 photon_to_store;
    PathType dest = destiny(cosNL, imodel, pp, photon_to_store);
    switch (dest) {
    case PathType::refr:
    {
        bool succ = ray.refract(inter_p, normal,
            cur_mi.first->refr_index, imodel->get_material()->refr_index, new_ray);
        if (!succ) {
            throw std::exception("ЧЗХ?");
        }
        break;
    }
    case PathType::dif_refl:
        if (photon_to_store.x == photon_to_store.y && photon_to_store.z == photon_to_store.y) {
            auto a = 1;
        }
        stored_photons.push_back(Photon(inter_p, photon_to_store, ray.dir));
        new_ray = ray.reflect_spherical(inter_p, normal);
        break;
    case PathType::spec_refl:
        new_ray = ray.reflect(inter_p, normal);
        break;
    case PathType::absorption:
        if (imodel->get_material()->specular == glm::vec3(0.f)) {
            if (photon_to_store.x == photon_to_store.y && photon_to_store.z == photon_to_store.y) {
                auto a = 1;
            }
            stored_photons.push_back(Photon(inter_p, photon_to_store, ray.dir));
        }
        return;
    default:
        break;
    }
    trace(new_ray, false, pp);
}
float PhotonMapping::FresnelSchlick(float cosNL, float n1, float n2) {
    float f0 = std::pow((n1 - n2) / (n1 + n2), 2);
    return f0 + (1.f - f0) * pow(1.f - cosNL, 5.f);
}
void PhotonMapping::build_map() {
    for (const LightSource& ls : lsources) {
        emit(ls);
    }
    this->photon_map.fill_balanced(stored_photons);
    stored_photons.clear();
    return ;
}
glm::vec3 PhotonMapping::render_trace(const Ray& ray) {
    glm::vec3 normal, inter_p;
    PMModel* imodel;
    glm::vec3 res(0.f);
    if (!find_intersection(ray, imodel, normal, inter_p)) {
        return res;
    }
    if (glm::dot(ray.dir, normal) > 0) {
        normal *= -1.f;
    }
    const Material* mat = imodel->get_material();
    //return mat->ambient;

    if (mat->diffuse != glm::vec3(0.f)) {
        glm::vec3 re;
        if (!photon_map.radiance_estimate(ray.dir, inter_p, normal, PhotonMap::Type::def, re)) { // diffuse
            throw std::exception("someting goes wrong...");
        }
        res = re;
    }

    float cosNL = glm::dot(-ray.dir, normal);
    auto cur_mi = mediums.peek(); // current material and id
    if (refract(cosNL, imodel)) {
        Ray nray;
        bool succ = ray.refract(inter_p, normal,
                cur_mi.first->refr_index, imodel->get_material()->refr_index, nray);
        if (!succ) {
            throw std::exception("ЧЗХ?");
        }
        auto t = render_trace(nray);
        res += t;
    }
    else if (mat->specular != glm::vec3(0.f)) {
        Ray nray = ray.reflect(inter_p, normal);
        auto t = render_trace(nray);
        res += t; // TODO пока так, но мб все же домножать?
    }
    return res;
}
void PhotonMapping::render() {
    canvas->clear();
    float width = canvas->get_width();
    float height = canvas->get_height();
    std::vector<glm::vec3> framebuffer(width * height);
    float fov = glm::radians(60.f);
    scene.camera = { 0,1,1 };
    scene.normal = { 0,0,-1 };
    for (size_t j = 0; j < height; j++) {
        for (size_t i = 0; i < width; i++) {
            float dir_x = i + 0.5f - width / 2.f;
            float dir_y = -(j + 0.5f) + height / 2.f;
            float dir_z = -height / (2.f * tan(fov / 2.f));
            //float x = (2 * (i + 0.5) / (float)width - 1) * tan(fov / 2.) * width / (float)height;
            //float y = -(2 * (j + 0.5) / (float)height - 1) * tan(fov / 2.);
            glm::vec3 dir = glm::normalize(glm::vec3(dir_x, dir_y, dir_z));
            Ray ray(scene.camera - scene.normal * 1.5f, dir);
            //Ray ray(scene.camera, dir);
            glm::vec3 color = glm::normalize(render_trace(ray));
            canvas->set_rgb(i, j, color * 255.f);
        }
    }
}
/*void PhotonMapping::render() {
    canvas->clear();
    float w = canvas->get_width();
    float h = canvas->get_height();
    glm::vec3 step_up = glm::vec3(scene.right_upper.x - scene.left_lower.x, 0.f, 0.f) / 
        (w - 1.f); //отношение ширины комнаты к ширине экрана
    glm::vec3 step_down = glm::vec3(0.f, scene.right_upper.y - scene.left_lower.y, 0.f) /
        (h - 1.f); //отношение высоты комнаты к высоте экрана
    glm::vec3 up = glm::vec3(scene.left_lower.x, scene.right_upper.y, scene.right_upper.z);
    glm::vec3 down = glm::vec3(scene.left_lower.x, scene.left_lower.y, scene.right_upper.z);

    for (int i = 0; i < w; ++i)
    {
        glm::vec3 step_y = (up - down) / (h - 1.f);
        //glm::vec3 d = glm::vec3(down);
        glm::vec3 dir = glm::vec3(down);
        for (int j = 0; j < h; ++j)
        {
            //pixels[i, j] = d;
            //Ray ray = Ray(scene.camera, pixels[i, j]);
            clear_mediums();
            Ray ray = Ray(scene.camera, glm::normalize(dir)); // мб что-то другое
            //r.origin = glm::vec3(pixels[i, j]);
            glm::vec3 color = glm::normalize(render_trace(ray));
            canvas->set_rgb(i, j, color * 255.f);
            //d += step_y;
            dir += step_y;
        }
        up += step_up;
        down += step_down;
    }
}*/
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