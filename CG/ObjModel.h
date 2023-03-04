#pragma once
#include "Texture.h"
#include "ObjMesh.h"
#include "ObjLoader.h"
#include <map>

class Model {
private:
    ObjTexture* overrideTextureDiffuse;
    ObjTexture* overrideTextureSpecular;
    OpenGLManager* manager;
    std::vector<Mesh> meshes;
    bool hasTexture = false;
public:
    Material material;
    Model() {

    }
    Model(const Model& other) {
        this->overrideTextureDiffuse = other.overrideTextureDiffuse;
        this->overrideTextureSpecular = other.overrideTextureSpecular;
        this->manager = other.manager;
        this->material = other.material;
        std::copy(other.meshes.begin(), other.meshes.end(), this->meshes.begin());
        this->hasTexture = other.hasTexture;

    }
    Model(ModelConstructInfo& mci) {
        this->manager = OpenGLManager::get_instance();
        this->material = mci.material;
        this->meshes.push_back(Mesh(mci.vertices.data(), mci.vertices.size(), NULL, 0));
    }
    /*Model(const char* objFile) {
        std::vector<ObjVertex> mesh = loadOBJ(objFile);
        meshes.push_back(Mesh(mesh.data(), mesh.size(), NULL, 0));
    }
    Model(
        const char* objFile,
        Material mat
    ) {
        manager = OpenGLManager::get_instance();
        material = mat;
        manager->checkOpenGLerror();
        hasTexture = true;
        std::vector<ObjVertex> mesh = loadOBJ(objFile);
        meshes.push_back(Mesh(mesh.data(), mesh.size(), NULL, 0));
    }
    Model(
        const char* objFile,
        ObjTexture tex
    ) {
        manager = OpenGLManager::get_instance();
        material = Material(tex);
        manager->checkOpenGLerror();
        hasTexture = true;
        std::vector<ObjVertex> mesh = loadOBJ(objFile);
        meshes.push_back(Mesh(mesh.data(), mesh.size(), NULL, 0));
    }*/

    void render(size_t count = 1, unsigned char mode = GL_TRIANGLES)
    {
        manager->checkOpenGLerror();
        //Draw
        for (auto& i : this->meshes) {
            if (material.map_Kd.initialized) {
                material.map_Kd.bind(0);
            }
            manager->checkOpenGLerror();
            i.render(count, mode);
            Texture::unbind();
        }
    }
};

std::map<std::string, Model*> loadObjModel(const std::string& path, const std::string& fname) {
    std::map<std::string, Model*> res;
    auto infos = loadOBJ(path, fname);
    for (auto it = infos.begin(); it != infos.end(); it++) {
        auto model = new Model(it->second);
        res[it->first] = model;
    }
    return res;
}