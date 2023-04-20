#include "PMModel.h"

// ========== Ray section start ==========
Ray::Ray(glm::vec3 origin, glm::vec3 dir) {
     this->origin = origin;
     this->dir = glm::normalize(dir);
}
Ray Ray::reflect_spherical(const glm::vec3& from, const glm::vec3& normal) const {
    /*float e1 = Random<float>::random();
    float e2 = Random<float>::random();
    float theta = std::pow(std::cos(std::sqrt(e1)), -1.f);
    float phi = M_PI * e2;
    glm::vec3 new_dir;
    float r = glm::length(normal);
    float sin_theta = sin(theta);
    new_dir.x = r * sin_theta * cos(phi);
    new_dir.y = r * sin_theta * sin(phi);
    new_dir.z = r * cos(theta);
    Ray res(from, new_dir);*/
    /*float u = Random<float>::random(-1.f, 1.f);
    float theta = Random<float>::random(0.f, 2.f * M_PI);
    float uc = std::sqrt(1.f - u * u);
    glm::vec3 new_dir(
        uc * std::cos(theta),
        uc * std::sin(theta),
        u
    );*/
    glm::vec3 new_dir;
    do {
        float x1, x2;
        float sqrx1, sqrx2;
        do {
            x1 = Random<float>::random(-1.f, 1.f);
            x2 = Random<float>::random(-1.f, 1.f);
            sqrx1 = x1 * x1;
            sqrx2 = x2 * x2;
        } while (sqrx1 + sqrx2 >= 1);
        float fx1x2 = std::sqrt(1.f - sqrx1 - sqrx2);
        new_dir.x = 2.f * x1 * fx1x2;
        new_dir.y = 2.f * x2 * fx1x2;
        new_dir.z = 1.f - 2.f * (sqrx1 + sqrx2);
    } while (glm::dot(new_dir, normal) < 0);
    return Ray(from, new_dir);
}
Ray Ray::reflect(const glm::vec3& from, const glm::vec3& normal) const {
    glm::vec3 refl_dir = dir - 2.f * normal * glm::dot(dir, normal); 
    return { from, glm::normalize(from + refl_dir) };
}
bool Ray::refract(const glm::vec3& from, const glm::vec3& normal, float refr1, float refr2, Ray& out) const {
    float eta = refr1 / refr2; // relative index of refraction
    float ndd = glm::dot(dir, normal); // n dot dir, cos(theta1)
    auto theta = 1.f - eta * eta * (1.f - ndd * ndd);
    if (theta < 0.f) {
        return false;
    }
    float theta_sqrt = glm::sqrt(theta);
    out.origin = from;
    out.dir = glm::normalize(eta * (dir - ndd * normal) - theta_sqrt * normal);

    //out.dir = glm::normalize(dir * theta - (cos_theta + refr_index * ndd) * normal);
    return true;
}
// ========== Ray section end ==========

// ========== PMModel section start ==========
size_t PMModel::id_gen = 1;
float PMModel::eps = 0.0001f;
PMModel::PMModel(const PMModel& other) : name(other.name) {
    this->id = other.id;
    this->mci = other.mci;
}
PMModel::PMModel(const ModelConstructInfo& mci) : name(mci.name) {
    this->mci = mci;
    this->id = id_gen;
    id_gen++;
}
bool PMModel::traingle_intersection(const Ray& ray, bool in_object, const glm::vec3& p0,
    const glm::vec3& p1, const glm::vec3& p2, float& out) const {
    out = 0.f;
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
bool PMModel::intersection(const Ray& ray, bool in_object, float& intersection, glm::vec3& normal) const {
    intersection = 0.f;
    size_t inter_ind = 0;
    if (mci.render_mode == GL_TRIANGLES) {
        for (size_t i = 0; i < mci.vertices.size(); i += 3) {
            float temp = 0.f;
            bool succ = traingle_intersection(ray, in_object, mci.vertices[i].position,
                mci.vertices[i + 1].position, mci.vertices[i + 2].position, temp);
            if (succ && (intersection == 0 || temp < intersection)) {
                intersection = temp;
                inter_ind = i;
            }
        }
    }
    else if (mci.render_mode == GL_QUADS) {
        for (size_t i = 0; i < mci.vertices.size(); i += 4) {
            float temp = 0.f;
            bool succ = traingle_intersection(ray, in_object, mci.vertices[i].position,
                mci.vertices[i + 1].position, mci.vertices[i + 3].position, temp);
            if (succ && (intersection == 0 || temp < intersection)) {
                intersection = temp;
                inter_ind = i;
                continue;
            }

            temp = 0.f;
            succ = traingle_intersection(ray, in_object, mci.vertices[i + 1].position,
                mci.vertices[i + 2].position, mci.vertices[i + 3].position, temp);
            if (succ && (intersection == 0 || temp < intersection)) {
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
const Material* PMModel::get_material() const {
    return &mci.material;
}
bool PMModel::equal(const PMModel& other) const {
    return this->id == other.id;
}
bool PMModel::equal(size_t other_id) const {
    return this->id == other_id;
}
size_t PMModel::get_id() const {
    return id;
}
glm::vec3* PMModel::get_wn() const
{
    glm::vec3 right_upper(mci.vertices[0].position), left_lower(mci.vertices[0].position), normal(0.f);
    for (auto& v : mci.vertices) {
        for (size_t point_i = 0; point_i < 3; point_i++) {
            if (v.position[point_i] > right_upper[point_i]) {
                right_upper[point_i] = v.position[point_i];
            }
            if (v.position[point_i] < left_lower[point_i]) {
                left_lower[point_i] = v.position[point_i];
            }
        }
        normal += v.normal;
    }
    normal /= mci.vertices.size();
    glm::vec3 res[3];
    res[0] = left_lower;
    res[1] = right_upper;
    res[2] = glm::normalize(normal);
    return res;
}

PMScene::PMScene() {
    std::vector<PMModel> objects;
    glm::vec3 left_lower, right_upper, camera, normal;
}
PMScene::PMScene(const std::vector<PMModel>& objects)
{
    this->objects = std::vector<PMModel>(objects.size());
    for (size_t i = 0; i < objects.size(); i++) {
        this->objects[i] = objects[i];
        if(objects[i].name == "frontWall") {
            glm::vec3* wn = objects[i].get_wn();
            left_lower = *wn;
            right_upper = *(wn + 1);
            normal = *(wn + 2);
        }
    }
    this->camera = (left_lower + right_upper) / 2.f;
    //this->camera = glm::vec3(0.f, 1.f, 1.f);
    //this->normal = glm::vec3(0.f, 0.f, - 1.f);
}
