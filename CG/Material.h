#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
struct Material {
    ObjTexture texture;
    ObjTexture* map_Kd;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    glm::vec3 emission;
    GLfloat opaque;
    GLfloat shininess;
    std::string diffuse_map;
    Material(){}
    Material(ObjTexture texture, glm::vec3 ambient = glm::vec3(0.2),
        glm::vec3 diffuse = glm::vec3(1.0), glm::vec3 specular = glm::vec3(1.0),
        glm::vec3 emission = glm::vec3(0.0), GLfloat shininess = 16.0f,
        GLfloat roughless = 0.3) {
        this->texture = texture;
        this->ambient = ambient;
        this->diffuse = diffuse;
        this->specular = specular;
        this->emission = emission;
        this->shininess = shininess;
    }
    // TODO очищать что-то еще?
    ~Material() {
        delete map_Kd;
    }
    std::string get_texture_name(const std::string& pref = "", const std::string& suf = "") const {
        return pref + "text" + suf;
    }
    std::string get_ambient_name(const std::string& pref = "", const std::string& suf = "") const {
        return pref + "ambient" + suf;
    }
    std::string get_dif_name(const std::string& pref = "", const std::string& suf = "") const {
        return pref + "diffuse" + suf;
    }
    std::string get_spec_name(const std::string& pref = "", const std::string& suf = "") const {
        return pref + "specular" + suf;
    }
    std::string get_emission_name(const std::string& pref = "", const std::string& suf = "") const {
        return pref + "emission" + suf;
    }
    std::string get_shininess_name(const std::string& pref = "", const std::string& suf = "") const {
        return pref + "shininess" + suf;
    }
};