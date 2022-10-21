#pragma once
#include "GL/glew.h"
#include <vector>
#include "Primitives.h"
#include <functional>
#include "PrimitiveChanger.h"
/*
Immortal temptation
Takes over my mind, condemned
Falling weak on my knees
Summon the strength of mayhem

I am the storm that is approaching
Provoking black clouds in isolation
I am reclaimer of my name
Born in flames, I have been blessed
My family crest is a demon of death
Forsakened, I am awakened
A phoenix's ash in dark divine
Descending misery
Destiny chasing time

Inherit the nightmare, surrounded by fate
Can't run away
Keep walking the line, between the light
Led astray

Through vacant halls I won't surrender
The truth revealed in eyes of ember
We fight through fire and ice forever
Two souls once lost, and now they remember

I am the storm that is approaching
Provoking black clouds in isolation
I am reclaimer of my name
Born in flames, I have been blessed
My family crest is a demon of death
Forsakened, I am awakened
A phoenix's ash in dark divine
Descending misery
Destiny chasing time

Disappear into the night
Lost shadows left behind
Obsession's pulling me
Fading, I've come to take what's mine

Lurking in the shadows under veil of night
Constellations of blood pirouette
Dancing through the graves of those who stand at my feet
Dreams of the black throne I keep on repeat
A derelict of dark, summoned from the ashes
The puppet master congregates all the masses
Pulling strings, twisting minds as blades hit
You want this power? Then come try and take it

Beyond the tree, fire burns
Secret love, bloodline yearns
Dark minds embrace, crimson joy
Does your dim heart heal or destroy?

Bury the light deep within
Cast aside, there's no coming home
We're burning chaos in the wind
Drifting in the ocean all alone
*/


/// <summary>
/// ...� ��� ������� �����, �� �� ��������,
/// ����������, ��� ����� ������ - ��� ����
/// </summary>
class Face : public Polygon{
public:
    Face() {
        //polygon = std::vector<Polygon>();
        drawing_type = GL_TRIANGLES;
    }
    Face(glm::vec3 coords, glm::vec3 color) {
        this->points.push_back(coords);
        this->color = color;
        this->drawing_type = GL_TRIANGLES;
    }
    void push_point(glm::vec3 coords) {
        points.push_back(coords);
    }
    bool primitive_is_finished() {
        if (points.size() < 3) {
            return false;
        }
        drawing_type = GL_TRIANGLE_FAN;
        return true;

    }
    glm::vec3 center() {
        float x=0, y=0, z=0;
        auto res = glm::vec3(0.0f);
        for (auto point : points) {
            res += point;
        }
        size_t size = points.size();
        res /= size;
        return res;
    }
};
/// <summary>
/// ��, ����, ������, ��
/// </summary>
class Figure {
public:
    /// <summary>
    /// ������� �� ����, ����� �������!
    /// </summary>
    std::vector<Face> faces;

    Figure() {
        faces = std::vector<Face>();
    }

    glm::vec3 center() {
        float x = 0, y = 0, z = 0;
        auto res = glm::vec3(0.0f);
        for (auto face : faces) {
            res += face.center();
        }
        size_t size = faces.size();
        res /= size;
        return res;
    }

    /// <summary>
    /// ���������������� �����
    /// ��-��-��
    /// </summary>
    void transform(std::function<glm::vec3(glm::vec3)> transofrmator) { //TODO: ���� ������, ��� ���������� �� ������ �������� - �������, �������
        for (auto i = 0; i < faces.size(); i++) {
            for (auto j = 0; j < faces[i].points.size(); j++) {
                faces[i].points[j] = transofrmator(faces[i].points[j]);
            }
        }
    }
};


class Tetrahedron : public Figure {};

class Octahedron : public Figure {};

class Hexahedron : public Figure {};

class Icosahedron : public Figure {};

class Dodecahedron : public Figure {};

enum FigureType {
    Tetrahed, Octahed,
    Hexahed, Icosahed,
    Dodecahed
};

class FigureBuilder {
    float bound = 100;
public:
    Figure buildFigure(FigureType kind, glm::vec3 color) {
        switch (kind){
        case Tetrahed:
            return buildTetrahedron(color);
        case Hexahed:
            return buildHexahedron(color);
        case Octahed:
            return buildOctahedron(color);
        case Icosahed:
            return buildIcosahedron(color);
        case Dodecahed:
            break;
        }
    }

    Tetrahedron buildTetrahedron(glm::vec3 color) {
        /*float from = 0.5f;
        float to = 0.5f;
        auto tetr = Tetrahedron();
        glm::vec3 a = glm::vec3(from, from, from);
        glm::vec3 c = glm::vec3(to, from, -to);
        glm::vec3 f = glm::vec3(to, to, from);
        glm::vec3 h = glm::vec3(from, to, -to);*/
        auto tetr = Tetrahedron();
        glm::vec3 a = glm::vec3(0, 0, 0);
        glm::vec3 c = glm::vec3(bound / 2, 0, -bound / 2);
        glm::vec3 f = glm::vec3(bound / 2, bound / 2, 0);
        glm::vec3 h = glm::vec3(0, bound / 2, -bound / 2);
        a /= bound;
        c /= bound;
        f /= bound;
        h /= bound;
        //TODO: ��������� ����� buildTriangle

        Face face1 = Face(a, glm::vec3(1.f, 0.f, 0.f));
        face1.push_point(f);
        face1.push_point(c);
        face1.primitive_is_finished();

        Face face2 = Face(f, glm::vec3(0.f, 1.f, 0.f));
        face2.push_point(c);
        face2.push_point(h);
        face2.primitive_is_finished();

        Face face3 = Face(c, glm::vec3(0.f, 0.f, 1.f));
        face3.push_point(h);
        face3.push_point(a);
        face3.primitive_is_finished();

        Face face4 = Face(f, color);
        face4.push_point(h);
        face4.push_point(a);
        face4.primitive_is_finished();

        tetr.faces.push_back(face1);
        tetr.faces.push_back(face2);
        tetr.faces.push_back(face3);
        tetr.faces.push_back(face4);
        return tetr;

    }

    Hexahedron buildHexahedron(glm::vec3 color) {
        auto hexa = Hexahedron();
        glm::vec3 a = glm::vec3(0, 0, 0);
        glm::vec3 b = glm::vec3(bound / 2, 0, 0);
        glm::vec3 c = glm::vec3(bound / 2, 0, -bound / 2);
        glm::vec3 d = glm::vec3(0, 0, -bound / 2);
        glm::vec3 e = glm::vec3(0, bound / 2, 0);
        glm::vec3 f = glm::vec3(bound / 2, bound / 2, 0);
        glm::vec3 g = glm::vec3(bound / 2, bound / 2, -bound / 2);
        glm::vec3 h = glm::vec3(0, bound / 2, -bound / 2);
        a /= bound;
        b /= bound;
        c /= bound;
        d /= bound;
        e /= bound;
        f /= bound;
        g /= bound;
        h /= bound;

        Face face1 = Face(a, glm::vec3(1.f, 0.f, 0.f));
        face1.push_point(b);
        face1.push_point(c);
        face1.push_point(d);
        face1.primitive_is_finished();
            
        Face face2 = Face(b, glm::vec3(0.f, 1.f, 0.f));
        face2.push_point(c);
        face2.push_point(g);
        face2.push_point(f);
        face2.primitive_is_finished();
        
        Face face3 = Face(f, glm::vec3(0.f, 0.f, 1.f));
        face3.push_point(g);
        face3.push_point(h);
        face3.push_point(e);
        face3.primitive_is_finished();

        Face face4 = Face(h, glm::vec3(1.f, 1.f, 0.f));
        face4.push_point(e);
        face4.push_point(a);
        face4.push_point(d);
        face4.primitive_is_finished();

        Face face5 = Face(a, glm::vec3(1.f, 0.f, 1.f));
        face5.push_point(b);
        face5.push_point(f);
        face5.push_point(e);
        face5.primitive_is_finished();
            
        
        Face face6 = Face(d, glm::vec3(0.f, 1.f, 1.f));
        face6.push_point(c);
        face6.push_point(g);
        face6.push_point(h);
        face6.primitive_is_finished();

        hexa.faces.push_back(face1);
        hexa.faces.push_back(face2);
        hexa.faces.push_back(face3);
        hexa.faces.push_back(face4);
        hexa.faces.push_back(face5);
        hexa.faces.push_back(face6);

        return hexa;
    }

    Octahedron buildOctahedron(glm::vec3 color) {
        auto res = Octahedron();
        auto core = buildHexahedron(color);
        auto a = core.faces[0].center();
        auto b = core.faces[1].center();
        auto c = core.faces[2].center();
        auto d = core.faces[3].center();
        auto e = core.faces[4].center();
        auto f = core.faces[5].center();

        Face face1 = Face(a, glm::vec3(1.f, 0.f, 0.f));
        face1.push_point(f);
        face1.push_point(b);
        face1.primitive_is_finished();

        Face face2 = Face(b, glm::vec3(0.f, 1.f, 0.f));
        face2.push_point(c);
        face2.push_point(f);
        face2.primitive_is_finished();

        Face face3 = Face(c, glm::vec3(0.f, 0.f, 1.f));
        face3.push_point(d);
        face3.push_point(f);
        face3.primitive_is_finished();

        Face face4 = Face(d, glm::vec3(1.f, 1.f, 0.f));
        face4.push_point(a);
        face4.push_point(f);
        face4.primitive_is_finished();

        Face face5 = Face(a, glm::vec3(1.f, 0.f, 1.f));
        face5.push_point(e);
        face5.push_point(b);
        face5.primitive_is_finished();

        Face face6 = Face(b, glm::vec3(0.f, 1.f, 1.f));
        face6.push_point(e);
        face6.push_point(c);
        face6.primitive_is_finished();

        Face face7 = Face(c, glm::vec3(1.f, 1.f, 1.f));
        face7.push_point(e);
        face7.push_point(d);
        face7.primitive_is_finished();

        Face face8 = Face(d, glm::vec3(0.5f, 0.5f, 0.5f));
        face8.push_point(e);
        face8.push_point(a);
        face8.primitive_is_finished();

        res.faces.push_back(face1);
        res.faces.push_back(face2);
        res.faces.push_back(face3);
        res.faces.push_back(face4);
        res.faces.push_back(face5);
        res.faces.push_back(face6);
        res.faces.push_back(face7);
        res.faces.push_back(face8);
        return res;
    }

    /*Icosahedron buildIcosahedron(glm::vec3 color) {
        auto res = Icosahedron();
        glm::vec3 center = glm::vec3(0.f);
        std::vector<glm::vec3> circle;
        for (size_t angle = 0; angle < 360; angle += 36) {
            if (angle % 72 == 0) {
                circle.push_back(glm::vec3(center.x + (cos(toRadians(angle))), center.y + 0.1, 
                    center.z + (sin(toRadians(angle)))));
                continue;
            }
            circle.push_back(glm::vec3(center.x + (cos(toRadians(angle))), center.y,
                center.z + (sin(toRadians(angle)))));
        }
        glm::vec3 a = glm::vec3(0, -0.1, 0);
        glm::vec3 b = glm::vec3(0, 0.1, 0);

        Face face;
        for (int i = 0; i < 10; i++) {
            face = Face(circle[i], color);
            face.push_point(circle[(i + 1) % 10]);
            face.push_point(circle[(i + 2) % 10]);
            face.primitive_is_finished();
            res.faces.push_back(face);
        }

        auto top_color = glm::vec3(39.f / 255.f, 36.f / 255.f, 37.f/255.f);

        face = Face(circle[1], top_color);
        face.push_point(a);
        face.push_point(circle[3]);
        face.primitive_is_finished();
        res.faces.push_back(face);
        
        face = Face(circle[3], top_color);
        face.push_point(a);
        face.push_point(circle[5]);
        face.primitive_is_finished();
        res.faces.push_back(face);

        face = Face(circle[5], top_color);
        face.push_point(a);
        face.push_point(circle[7]);
        face.primitive_is_finished();
        res.faces.push_back(face);

        face = Face(circle[7], top_color);
        face.push_point(a);
        face.push_point(circle[9]);
        face.primitive_is_finished();
        res.faces.push_back(face);

        face = Face(circle[9], top_color);
        face.push_point(a);
        face.push_point(circle[1]);
        face.primitive_is_finished();
        res.faces.push_back(face);


        face = Face(circle[0], top_color);
        face.push_point(b);
        face.push_point(circle[2]);
        face.primitive_is_finished();
        res.faces.push_back(face);

        face = Face(circle[2], top_color);
        face.push_point(b);
        face.push_point(circle[4]);
        face.primitive_is_finished();
        res.faces.push_back(face);

        face = Face(circle[4], top_color);
        face.push_point(b);
        face.push_point(circle[6]);
        face.primitive_is_finished();
        res.faces.push_back(face);

        face = Face(circle[6], top_color);
        face.push_point(b);
        face.push_point(circle[8]);
        face.primitive_is_finished();
        res.faces.push_back(face);

        face = Face(circle[8], top_color);
        face.push_point(b);
        face.push_point(circle[0]);
        face.primitive_is_finished();
        res.faces.push_back(face);
       
        return res;
    }*/

    Icosahedron buildIcosahedron(glm::vec3 color) {
        auto res = Icosahedron();

        auto an_color = glm::vec3(1.f) - color;
        auto half_bound = bound / 2;
        float r = half_bound;
        auto p1 = glm::vec3(0, -half_bound, -r) / bound;
        auto p2 = glm::vec3(0, half_bound, -r) / bound;
        auto p3 = glm::vec3(half_bound, r, 0) / bound;
        auto p4 = glm::vec3(r, 0, -half_bound) / bound;
        auto p5 = glm::vec3(half_bound, -r, 0) / bound;
        auto p6 = glm::vec3(-half_bound, -r, 0) / bound;
        auto p7 = glm::vec3(-r, 0, -half_bound) / bound;
        auto p8 = glm::vec3(-half_bound, r, 0) / bound;
        auto p9 = glm::vec3(r, 0, half_bound) / bound;
        auto p10 = glm::vec3(-r, 0, half_bound) / bound;
        auto p11 = glm::vec3(0, -half_bound, r) / bound;
        auto p12 = glm::vec3(0, half_bound, r) / bound;

        Face face = Face(p1, glm::vec3(1.f, 0.f, 0.f));
        face.push_point(p2);
        face.push_point(p4);
        face.primitive_is_finished();
        res.faces.push_back(face);

        face = Face(p1, glm::vec3(0.f, 1.f, 0.f));
        face.push_point(p2);
        face.push_point(p7);
        face.primitive_is_finished();
        res.faces.push_back(face);

        face = Face(p7, glm::vec3(0.f, 0.f, 1.f));
        face.push_point(p2);
        face.push_point(p8);
        face.primitive_is_finished();
        res.faces.push_back(face);

        face = Face(p8, glm::vec3(1.f, 1.f, 0.f));
        face.push_point(p2);
        face.push_point(p3);
        face.primitive_is_finished();
        res.faces.push_back(face);

        face = Face(p4, glm::vec3(1.f, 0.f, 1.f));
        face.push_point(p2);
        face.push_point(p3);
        face.primitive_is_finished();
        res.faces.push_back(face);

        face = Face(p6, glm::vec3(0.f, 1.f, 1.f));
        face.push_point(p1);
        face.push_point(p5);
        face.primitive_is_finished();
        res.faces.push_back(face);

        face = Face(p6, glm::vec3(1.f, 1.f, 1.f));
        face.push_point(p7);
        face.push_point(p10);
        face.primitive_is_finished();
        res.faces.push_back(face);

        face = Face(p10, glm::vec3(0.f, 0.f, 0.f));
        face.push_point(p7);
        face.push_point(p8);
        face.primitive_is_finished();
        res.faces.push_back(face);

        face = Face(p10, glm::vec3(0.5f, 0.f, 0.f));
        face.push_point(p8);
        face.push_point(p12);
        face.primitive_is_finished();
        res.faces.push_back(face);

        face = Face(p12, glm::vec3(0.5f, 0.5f, 0.f));
        face.push_point(p8);
        face.push_point(p3);
        face.primitive_is_finished();
        res.faces.push_back(face);

        face = Face(p9, glm::vec3(0.5f, 0.0f, 0.5f));
        face.push_point(p4);
        face.push_point(p3);
        face.primitive_is_finished();
        res.faces.push_back(face);

        face = Face(p5, glm::vec3(0.0f, 0.5f, 0.5f));
        face.push_point(p4);
        face.push_point(p9);
        face.primitive_is_finished();
        res.faces.push_back(face);

        face = Face(p12, glm::vec3(0.5f, 0.5f, 0.5f));
        face.push_point(p3);
        face.push_point(p9);
        face.primitive_is_finished();
        res.faces.push_back(face);

        face = Face(p5, glm::vec3(1.0f, 0.5f, 0.5f));
        face.push_point(p1);
        face.push_point(p4);
        face.primitive_is_finished();
        res.faces.push_back(face);

        face = Face(p7, glm::vec3(1.0f, 0.5f, 0.5f));
        face.push_point(p1);
        face.push_point(p6);
        face.primitive_is_finished();
        res.faces.push_back(face);

        face = Face(p11, glm::vec3(1.0f, 1.0f, 0.5f));
        face.push_point(p5);
        face.push_point(p6);
        face.primitive_is_finished();
        res.faces.push_back(face);

        face = Face(p11, glm::vec3(1.0f, 0.5f, 1.0f));
        face.push_point(p6);
        face.push_point(p10);
        face.primitive_is_finished();
        res.faces.push_back(face);

        face = Face(p11, glm::vec3(0.5f, 1.0f, 1.0f));
        face.push_point(p10);
        face.push_point(p12);
        face.primitive_is_finished();
        res.faces.push_back(face);

        face = Face(p11, glm::vec3(0.0f, 0.5f, 0.5f));
        face.push_point(p12);
        face.push_point(p9);
        face.primitive_is_finished();
        res.faces.push_back(face);

        face = Face(p11, glm::vec3(0.5f, 0.0f, 0.5f));
        face.push_point(p5);
        face.push_point(p9);
        face.primitive_is_finished();
        res.faces.push_back(face);

        return res;
    }
};
