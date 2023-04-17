#include "ObjLoader.h"

void loadMaterial(const std::string& mtl_path, const std::string mtl_fname,
    const std::string& material, Material& dest) {
    std::stringstream ss;
    std::string line = "";
    std::string prefix = "";
    glm::vec3 temp_vec3;
    GLfloat temp_float;
    std::ifstream mtl_file(mtl_path + "/" + mtl_fname);
    if (!mtl_file.is_open())
    {
        throw "ERROR::OBJLOADER::Could not open mtl file.";
    }
    bool processing = false;
    while (std::getline(mtl_file, line))
    {
        ss.clear();
        if (line.size() == 0) {
            prefix.clear();
        }
        else {
            ss.str(line);
            ss >> prefix;
        }
        if (prefix == "#") {

        }
        else if (prefix == "newmtl") {
            if (processing) { // ≈сли наткнулись на другой материал во врем€ обработки, то прошлый "закончилс€"
                break;
            }
            std::string mat_name;
            ss >> mat_name;
            if (mat_name != material) {
                continue;
            }
            processing = true; // Ќашли нужный материал и начали его считывать
        }
        else if (prefix == "Ka") {
            ss >> temp_vec3.r >> temp_vec3.g >> temp_vec3.b;
            dest.ambient = temp_vec3;
        }
        else if (prefix == "Kd") {
            ss >> temp_vec3.r >> temp_vec3.g >> temp_vec3.b;
            dest.diffuse = temp_vec3;
        }
        else if (prefix == "Ks") {
            ss >> temp_vec3.r >> temp_vec3.g >> temp_vec3.b;
            dest.specular = temp_vec3;
        }
        else if (prefix == "Ke") {
            ss >> temp_vec3.r >> temp_vec3.g >> temp_vec3.b;
            dest.emission = temp_vec3;
        }
        else if (prefix == "d") {
            ss >> temp_float;
            dest.opaque = temp_float;
        }
        else if (prefix == "Tr") {
            ss >> temp_float;
            dest.opaque = 1.f - temp_float;
        }
        else if (prefix == "Ns") {
            ss >> temp_float;
            dest.shininess = temp_float;
        }
        else if (prefix == "Ni") {
            ss >> temp_float;
            dest.refr_index = temp_float;
        }
        else if (prefix == "map_Kd") {
            std::string map_path;
            ss >> map_path;
            map_path = mtl_path + "/" + map_path;
            dest.map_Kd = ObjTexture(map_path.c_str(), 'n');
        }
    }
}

void build_vertices(std::vector<ObjVertex>& vertices, std::vector<glm::fvec3>& vertex_positions,
    std::vector<glm::fvec2>& vertex_texcoords, std::vector<glm::fvec3>& vertex_normals,
    std::vector<GLint>& vertex_position_indicies, std::vector<GLint>& vertex_texcoord_indicies,
    std::vector<GLint>& vertex_normal_indicies) {
    vertices.resize(vertex_position_indicies.size(), ObjVertex());
    for (size_t i = 0; i < vertices.size(); ++i)
    {
        GLint pos_ind = vertex_position_indicies[i] - 1;
        if (pos_ind < 0) {
            pos_ind = vertex_positions.size() + pos_ind + 1;
        }
        vertices[i].position = vertex_positions[pos_ind];
        if (vertex_texcoord_indicies.size() != 0)
        {
            vertices[i].texcoord = vertex_texcoords[vertex_texcoord_indicies[i] - 1];
        }
        if (vertex_normal_indicies.size() != 0) {
            vertices[i].normal = vertex_normals[vertex_normal_indicies[i] - 1];
        }
    }

    /*vertex_positions = std::vector<glm::fvec3>();
    vertex_texcoords = std::vector<glm::fvec2>();
    vertex_normals = std::vector<glm::fvec3>();*/
    vertex_position_indicies = std::vector<GLint>();
    vertex_texcoord_indicies = std::vector<GLint>();
    vertex_normal_indicies = std::vector<GLint>();
}

std::map<std::string, ModelConstructInfo> loadOBJ(const std::string& path, const std::string& fname)
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
    std::string cur_name = "n1";

    std::stringstream ss;
    std::ifstream obj_file(path + "/" + fname);
    std::string mtl_fname;
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
        if (line.size() == 0) {
            prefix.clear();
        }
        else {
            ss.str(line);
            ss >> prefix;
        }
        if (desc_finished && prefix != "f") {
            if (cur_name == "backWall") {
                auto a = 1;
            }
            build_vertices(cur.vertices, vertex_positions, vertex_texcoords, vertex_normals, vertex_position_indicies,
                vertex_texcoord_indicies, vertex_normal_indicies);
            //DEBUG
            std::cout << "Nr of vertices: " << cur.vertices.size() << "\n";
            if (res.find(cur_name) != res.end()) {
                cur_name += '1';
            }
            res[cur_name] = cur;
            cur = ModelConstructInfo();
            desc_finished = false;
            //std::cout << "OBJ file loaded!" << "\n";
            //return res;
        }
        if (prefix == "#")
        {
            std::string comment;
            ss >> comment;

        }
        else if (prefix == "mtllib") {
            ss >> mtl_fname;
        }
        else if (prefix == "g") {
            ss >> cur_name;
        }
        else if (prefix == "o")
        {
            ss >> cur_name;
        }
        else if (prefix == "s")
        {

        }
        else if (prefix == "usemtl")
        {
            std::string mat_name;
            ss >> mat_name;
            loadMaterial(path, mtl_fname, mat_name, cur.material);
            // позорно проиграл плюсам и сдалс€. TODO: разобратьс€ и исправить
            //Material mat(loadMaterial(path, mtl_fname, mat_name));
            //cur.material = Material(mat);
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
            desc_finished = true; // ѕосле всех "f" либо конец файла, либо следующа€ модель
            int counter = 0;
            size_t ps = 1; // poly size
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
                    if (counter == 0) {
                        ps++; // ≈сли модель содержит описание вида f v v v
                    }
                    else {
                        counter++; // ≈сли модель содержит описание вида f v/vt/vn v/vt/vn v/vt/vn
                    }
                    ss.ignore(1, ' ');
                }

                //Reset the counter
                if (counter > 2) {
                    counter = 0;
                    ps++;
                }
            }
            cur.lengths.push_back(ps);
            if (ps == 3) {
                cur.render_mode = GL_TRIANGLES;
            }
            else if (ps == 4) {
                cur.render_mode = GL_QUADS;
            }
            else if (ps > 4) {
                cur.render_mode = GL_POLYGON;
            }
        }
        else
        {

        }
    }
    if (desc_finished) {
        build_vertices(cur.vertices, vertex_positions, vertex_texcoords, vertex_normals, vertex_position_indicies,
            vertex_texcoord_indicies, vertex_normal_indicies);
        //DEBUG
        std::cout << "Nr of vertices: " << cur.vertices.size() << "\n";
        if (res.find(cur_name) != res.end()) {
            cur_name += '1';
        }
        res[cur_name] = cur;
    }

    //Loaded success
    std::cout << "OBJ file loaded!" << "\n";
    return res;
}
