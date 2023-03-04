#pragma once
#include "Primitives.h"
#include "Figure.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include <map>

struct ObjVertex {
    glm::vec3 position;
    glm::vec2 texcoord;
    glm::vec3 normal;
};
// Нужна для конструкции класса модели
struct ModelConstructInfo {
    Material material;
    std::vector<ObjVertex> vertices;
    unsigned char render_mode;
};
Material loadMaterial(const std::string& mtl_path, const std::string& material) {
    std::stringstream ss;
    std::string line = "";
    std::string prefix = "";
    glm::vec3 temp_vec3;
    GLfloat temp_float;
    std::ifstream mtl_file(mtl_path);
    Material res;
    if (!mtl_file.is_open())
    {
        throw "ERROR::OBJLOADER::Could not open mtl file.";
    }
    bool checked = false;
    while (checked || std::getline(mtl_file, line))
    {
        ss.clear();
        ss.str(line);
        ss >> prefix;
        if (prefix == "newmtl") {
            std::string mat_name;
            ss >> mat_name;
            if(mat_name != material){
                continue;
            }
            checked = true;
        }
        else if (prefix == "Ka") {
            ss >> temp_vec3.r >> temp_vec3.g >> temp_vec3.b;
            res.ambient = temp_vec3;
        }
        else if (prefix == "Kd") {
            ss >> temp_vec3.r >> temp_vec3.g >> temp_vec3.b;
            res.diffuse = temp_vec3;
        }
        else if (prefix == "Ks") {
            ss >> temp_vec3.r >> temp_vec3.g >> temp_vec3.b;
            res.specular = temp_vec3;
        }
        else if (prefix == "d") {
            ss >> temp_float;
            res.opaque = temp_float;
        }
        else if (prefix == "Tr") {
            ss >> temp_float;
            res.opaque = 1.f - temp_float;
        }
        else if (prefix == "Ns") {
            ss >> temp_float;
            res.shininess = temp_float;
        }
        else if (prefix == "map_Kd") {
            std::string map_path;
            ss >> map_path;
            ObjTexture* tex = new ObjTexture(map_path.c_str(), 'n');
            res.map_Kd = tex;
        }
    }
    return res;
}
std::vector<ObjVertex> build_vertices(std::vector<ObjVertex>& vertices, std::vector<glm::fvec3>& vertex_positions,
    std::vector<glm::fvec2>& vertex_texcoords, std::vector<glm::fvec3>& vertex_normals,
    std::vector<GLint>& vertex_position_indicies, std::vector<GLint>& vertex_texcoord_indicies,
    std::vector<GLint>& vertex_normal_indicies) {
    vertices.resize(vertex_position_indicies.size(), ObjVertex());
    for (size_t i = 0; i < vertices.size(); ++i)
    {
        vertices[i].position = vertex_positions[vertex_position_indicies[i] - 1];
        if (vertex_texcoords.size() != 0)
        {
            vertices[i].texcoord = vertex_texcoords[vertex_texcoord_indicies[i] - 1];
        }
        if (vertex_normals.size() != 0) {
            vertices[i].normal = vertex_normals[vertex_normal_indicies[i] - 1];
        }
    }

    vertex_positions = std::vector<glm::fvec3>();
    vertex_texcoords = std::vector<glm::fvec2>();
    vertex_normals = std::vector<glm::fvec3>();
    vertex_position_indicies = std::vector<GLint>();
    vertex_texcoord_indicies = std::vector<GLint>();
    vertex_normal_indicies = std::vector<GLint>();
}
std::map<std::string, ModelConstructInfo> loadOBJ(const std::string& path, const std::string fname)
{
    //Vertex portions
    std::vector<glm::fvec3> vertex_positions;
    std::vector<glm::fvec2> vertex_texcoords;
    std::vector<glm::fvec3> vertex_normals;

    //Face vectors
    std::vector<GLint> vertex_position_indicies;
    std::vector<GLint> vertex_texcoord_indicies;
    std::vector<GLint> vertex_normal_indicies;

    std::map<std::string, ModelConstructInfo> res;
    ModelConstructInfo cur;
    std::string cur_name;

    std::stringstream ss;
    std::ifstream obj_file(path + "/" + fname);
    std::string mtl_path;
    std::string line = "";
    std::string prefix = "";
    glm::vec3 temp_vec3;
    glm::vec2 temp_vec2;
    GLint temp_glint = 0;
    bool desc_finished = false;

    if (!obj_file.is_open())
    {
        throw "ERROR::OBJLOADER::Could not open file.";
    }

    while (std::getline(obj_file, line))
    {
        //Get the prefix of the line
        ss.clear();
        ss.str(line);
        ss >> prefix;

        if (prefix == "#")
        {
            std::string comment;
            ss >> comment;
            
        }
        else if (prefix == "mtllib") {
            ss >> mtl_path;
            mtl_path = path + "/" + mtl_path;
        }
        else if (prefix == "g") {
            ss >> cur_name;
        }
        else if (prefix == "o")
        {

        }
        else if (prefix == "s")
        {

        }
        else if (prefix == "use_mtl")
        {
            std::string mat_name;
            ss >> mat_name;
            Material mat = loadMaterial(mtl_path, mat_name);
            cur.material = mat;
        }
        else if (prefix == "v") //Vertex position
        {
            ss >> temp_vec3.x >> temp_vec3.y >> temp_vec3.z;
            vertex_positions.push_back(temp_vec3);
        }
        else if (prefix == "vt")
        {
            ss >> temp_vec2.x >> temp_vec2.y;
            vertex_texcoords.push_back(glm::vec2(temp_vec2.x, 1 - temp_vec2.y));
        }
        else if (prefix == "vn")
        {
            ss >> temp_vec3.x >> temp_vec3.y >> temp_vec3.z;
            vertex_normals.push_back(temp_vec3);
        }
        else if (prefix == "f")
        {
            desc_finished = true; // После всех "f" либо конец файла, либо следующая модель
            int counter = 0;
            while (ss >> temp_glint)
            {
                //Pushing indices into correct arrays
                if (counter == 0)
                    vertex_position_indicies.push_back(temp_glint);
                else if (counter == 1)
                    vertex_texcoord_indicies.push_back(temp_glint);
                else if (counter == 2)
                    vertex_normal_indicies.push_back(temp_glint);

                //Handling characters
                if (ss.peek() == '/')
                {
                    ++counter;
                    ss.ignore(1, '/');
                    if (ss.peek() == '/')
                    {
                        ++counter;
                        ss.ignore(1, '/');
                    }
                }
                else if (ss.peek() == ' ')
                {
                    ++counter;
                    ss.ignore(1, ' ');
                }

                //Reset the counter
                if (counter > 2)
                    counter = 0;
            }
        }
        else if (desc_finished) {
            build_vertices(cur.vertices, vertex_positions, vertex_texcoords, vertex_normals, vertex_position_indicies,
                vertex_texcoord_indicies, vertex_normal_indicies);
            //DEBUG
            std::cout << "Nr of vertices: " << cur.vertices.size() << "\n";
            res[cur_name] = cur;
            cur = ModelConstructInfo();
        }
        else
        {

        }
    }


    //Loaded success
    std::cout << "OBJ file loaded!" << "\n";
    return vertices;
}