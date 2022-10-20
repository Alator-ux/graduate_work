#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>

struct Color {
    glm::vec4 rgba;
    Color() {
        rgba = glm::vec4(0.0f);
    }
    Color(glm::vec4 rgba) {
        this->rgba = rgba;
    }
    Color(glm::vec3 rgb) {
        this->rgba = glm::vec4(rgb, 1.0f);
    }
    glm::vec3 to_vec3() {
        return glm::vec3(rgba.r, rgba.g, rgba.b);
    }
    static Color red() {
        return Color(glm::vec3(1.0f, 0.0f, 0.0f));
    }
    static Color green() {
        return Color(glm::vec3(0.0f, 1.0f, 0.0f));
    }
    static Color blue() {
        return Color(glm::vec3(0.0f, 0.0f, 1.0f));
    }
    static Color white() {
        return Color(glm::vec3(1.0f, 1.0f, 1.0f));
    }
    static Color copper() {
        return Color(glm::vec3(1.0f, 0.5f, 0.31f));
    }
    static Color dark_olive_green() {
        return Color(glm::vec3(0.33f, 0.42f, 0.18f));
    }
};

enum Axis{ox, oy, oz};

struct Primitive {
public:
    int type;
    std::vector<glm::vec3> points;
    int drawing_type;
    glm::vec3 color;
    float width = 1;
    GLint get_points_count() {
        return (GLint)points.size();
    }
    void push_point(glm::vec2 coords) {

    }
    bool primitive_is_finished() {
        return true;
    }
    static std::string get_string_name() {
        return "Primitive";
    }
};

struct Point : public Primitive {
public:
    Point(glm::vec3 coord, glm::vec3 color) {
        points.push_back(coord);
        this->color = color;
        drawing_type = GL_POINTS;
        this->type = 0;
    }
    static std::string get_string_name() {
        return "Point";
    }
};

struct BezierPoint : public Primitive {
    BezierPoint(glm::vec3 coord, glm::vec3 color, int offset = 50) {
        this->points.push_back(coord);
        auto help_p = glm::vec3(coord.x - offset, coord.y, 1);
        this->points.push_back(help_p);
        help_p = glm::vec3(coord.x + offset, coord.y, 1);
        this->points.push_back(help_p);

        this->drawing_type = GL_POINTS;
        this->color = color;
    }
    glm::vec3& get_main() {
        return points[0];
    }
    glm::vec3& get_fhelp() {
        return points[1];
    }
    glm::vec3& get_shelp() {
        return points[2];
    }
    void set_main(glm::vec3 coords) {
        auto offset = coords - points[0];
        points[0] = coords;
        points[1] += offset;
        points[2] += offset;
    }
    void set_fhelp(glm::vec3 coords) {
        points[2] -= coords - points[1];
        points[1] = coords;
    }
    void set_shelp(glm::vec3 coords) {
        points[1] -= coords - points[2];
        points[2] = coords;
    }
};

struct Edge : public Primitive {
public:
    Edge(glm::vec3 coords, glm::vec3 color) {
        points.push_back(coords);
        this->color = color;
        drawing_type = GL_POINTS;
        this->type = 1;
    }
    Edge(glm::vec3 first, glm::vec3 second, glm::vec3 color, float width = 1) {
        points.push_back(first);
        push_point(second);
        this->color = color;
        this->type = 1;
        this->width = width;
    }
    void push_point(glm::vec3 coords) {
        if (points.size() == 1) {
            points.push_back(coords);
            drawing_type = GL_LINES;
        }
    }
    bool primitive_is_finished() {
        return points.size() == 2;
    }
    static std::string get_string_name() {
        return "Edge";
    }
};

struct Segment : public Primitive {
public:
    Segment(glm::vec3 left_point, glm::vec3 right_point, glm::vec3 color) {
        points.push_back(left_point);
        points.push_back(right_point);
        drawing_type = GL_LINES;
        this->color = color;
    }
    glm::vec3 left() {
        return points[0];
    }
    
    glm::vec3 right() {
        return points[1];
    }
    bool primitive_is_finished() {
        return points.size() == 2;
    }
    static std::string get_string_name() {
        return "Segment";
    }
};


struct Polygon : public Primitive {
public:
    Polygon(){}
    Polygon(glm::vec3 coords, glm::vec3 color) {
        points.push_back(coords);
        this->color = color;
        drawing_type = GL_POINTS;
        this->type = 2;
    }
    void push_point(glm::vec3 coords) {
        points.push_back(coords);
        if (points.size() > 1) {
            drawing_type = GL_LINE_STRIP;
        }
    }
    bool primitive_is_finished() {
        if (points.size() < 3) {
            return false;
        }
        drawing_type = GL_LINE_LOOP;
        return true;

    }
    static std::string get_string_name() {
        return "Polygon";
    }
};

class PrimitiveFabric {
    int code = 0;
    glm::vec3 color;
    std::vector<Primitive> primitives;
    bool prim_finished = true;
    size_t max_size;
public:
    PrimitiveFabric() {
        color = glm::vec3(1.0f);
        max_size = 10;
    }
    void update_code(int code) {
        if (this->code == code) {
            return;
        }
        this->code = code;
        if (!prim_finished) {
            primitives.erase(primitives.end() - 1);
        }
        prim_finished = true;
    }
    void update_color(glm::vec3 color) {
        this->color = color;
    }
    void build(glm::vec3 coords) {
        switch (code)
        {
        case 0:
            create_point(coords);
            break;
        case 1:
            if (prim_finished) {
                create_edge(coords);
            }
            else {
                Edge* edge = reinterpret_cast<Edge*>(&primitives[primitives.size() - 1]);
                edge->push_point(coords);
                prim_finished = edge->primitive_is_finished();
            }
            break;
        case 2:
            if (prim_finished) {
                create_polygon(coords);
            }
            else {
                Polygon* polygon = reinterpret_cast<Polygon*>(&primitives[primitives.size() - 1]);
                polygon->push_point(coords);
            }
        default:
            break;
        }
    }
    void finish_primitive() {
        switch (code)
        {
        case 0:
            prim_finished = true;
            break;
        case 1:
        {
            Edge* edge = reinterpret_cast<Edge*>(&primitives[primitives.size() - 1]);
            prim_finished = edge->primitive_is_finished();
            break;
        }
        case 2: 
        {
            Polygon* polygon = reinterpret_cast<Polygon*>(&primitives[primitives.size() - 1]);
            prim_finished = polygon->primitive_is_finished();
            break;
        }
        default:
            break;
        }
    }
    void create_point(glm::vec3 coord) {
        if (primitives.size() == max_size) {
            return;
        }
        primitives.push_back(Point(coord, color));
        prim_finished = true;
    }
    void create_edge(glm::vec3 coords) {
        if (primitives.size() == max_size) {
            return;
        }
        primitives.push_back(Edge(coords, color));
        prim_finished = false;
    }
    void create_polygon(glm::vec3 coords) {
        if (primitives.size() == max_size) {
            return;
        }
        primitives.push_back(Polygon(coords, color));
        prim_finished = false;
    }
    void clear() {
        primitives.clear();
        prim_finished = true;
    }
    std::vector<Primitive>& get_items() {
        return primitives;
    }
    size_t size() {
        return primitives.size();
    }
    size_t get_max_size() {
        return max_size;
    }
};