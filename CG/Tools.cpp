#include "Tools.h"

bool vec3_equal(const glm::vec3& f, const glm::vec3& s) {
    constexpr float eps = 0.0001f; // TODO потом как-нибудь на нормальное заменить
    bool res = true;
    res = res && glm::abs(f.x - s.x) < eps;
    res = res && glm::abs(f.y - s.y) < eps;
    res = res && glm::abs(f.z - s.z) < eps;
    return res;
}

std::vector<std::string> split(const std::string& str, char delim) {
    std::stringstream ss(str);
    std::vector<std::string> res;
    std::string temp;
    while (std::getline(ss, temp, delim)) {
        res.push_back(temp);
    }
    return res;
}