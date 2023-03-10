#pragma once
#include "Primitives.h"
#include "GLM/mat3x3.hpp"
#include "GLM/vec3.hpp"
#include "GLM/glm.hpp"

class PrimitiveChanger {
    glm::vec3 matrix_mult(glm::mat3x3 afin_matrix, glm::vec3 point)
    {
        glm::vec3 res;
        for (int i = 0; i < 3; ++i)
        {
            res[i] = 0;
            for (int k = 0; k < 3; ++k)
                res[i] += afin_matrix[k][i] * point[k];
        }
        return res;
    }

    // Returns matrix which needed for point rotation.
    // x and y are the coordinates of point, which we are roating around
    glm::mat3x3 build_rotation_matrix(double x, double y, double angle) {
        auto rad_angle = toRadians(angle);
        auto cos_r_a = cos(rad_angle);
        auto sin_r_a = sin(rad_angle);
        glm::mat3x3 shift_matrix1 = glm::mat3x3(
            1, 0, 0,
            0, 1, 0,
            -x, -y, 1);
        glm::mat3x3 shift_matrix2 = glm::mat3x3(
            1, 0, 0,
            0, 1, 0,
            x, y, 1);
        glm::mat3x3 rotation_matrix = glm::mat3x3(
            cos_r_a, -sin_r_a, 0,
            -sin_r_a, cos_r_a, 0,
            0, 0, 1);

       glm::mat3x3 temp = glm::mat3x3(
            cos_r_a, sin_r_a, 0,
            -sin_r_a, cos_r_a, 0,
            -x * cos_r_a + y * sin_r_a + x, -x * sin_r_a - y * cos_r_a + y, 1);
        /*glm::mat3x3 temp = glm::mat3x3(
            cos_r_a, -sin_r_a, -x * cos_r_a + y * sin_r_a + x,
            sin_r_a, cos_r_a, -x * sin_r_a - y * cos_r_a + y,
            0, 0, 1
        );*/

        
        glm::mat3x3 res = (shift_matrix1 * rotation_matrix * shift_matrix2);
        return temp;
    }

    glm::mat3x3 build_scale_matrix(double x, double y, double x_factor, double y_factor) {
        glm::mat3x3 shift_matrix1 = glm::mat3x3(
            1, 0, 0,
            0, 1, 0,
            -x, -y, 1);
        glm::mat3x3 shift_matrix2 = glm::mat3x3(
            1, 0, 0,
            0, 1, 0,
            x, y, 1);
        glm::mat3x3 scaling_matrix = glm::mat3x3(
            x_factor, 0, 0,
            0, y_factor, 0,
            0, 0, 1);

        glm::mat3x3 temp = glm::mat3x3(
            x_factor, 0, 0,
            0, y_factor, 0,
            (1 - x_factor) * x, (1 - y_factor) * y, 1);

        auto res = (shift_matrix1 * scaling_matrix * shift_matrix2);
        return temp;
    }
    glm::vec3 get_center(std::vector<glm::vec3>& points) {
        GLfloat midx = 0;
        GLfloat midy = 0;
        GLfloat midz = 0;
        for (int i = 0; i < points.size(); i++) {
            midx += points[i].x;
            midy += points[i].y;
            midz += points[i].z;
        }
        midx /= points.size();
        midy /= points.size();
        midz /= points.size();
        return glm::vec3(midx, midy, midz);
    }
public:
    PrimitiveChanger(){}

    // Converts provided angle to it's radian presentation
    double toRadians(double angle) {
        return PI * angle / 180.0;
    }

    void shift(Primitive* item, glm::vec3 coords) {
        if (item == nullptr) {
            return;
        }
        
        std::vector<glm::vec3> points = item->points;
        GLfloat dx = coords.x - points[0].x;
        GLfloat dy = coords.y - points[0].y;
        
        shift(item, dx, dy);
    }
    void shift(Primitive* item, float dx, float dy) {
        if (item == nullptr) {
            return;
        }

        std::vector<glm::vec3> points = item->points;

        glm::mat3x3 shift_matrix = glm::mat3x3(1, 0, 0,
            0, 1, 0,
            dx, dy, 1);
        //shift_matrix = glm::transpose(shift_matrix);

        for (int i = 0; i < points.size(); i++) {
            glm::vec3 values = glm::vec3(points[i][0], points[i][1], 1); //TODO: remove it m8
            auto shifted_point = shift_matrix * values; // ????????? ? ???????? ???????, ?.?.
            // glm ??????? ?????? ????????
            item->points[i] = shifted_point;
        }
    }
    void rotate_around_point(Primitive* item, glm::vec3 point, double angle) {
        if (item == nullptr) {
            return;
        }

        std::vector<glm::vec3> points = item->points;

        glm::mat3x3 rotation_matrix = this->build_rotation_matrix(point[0], point[1], angle);

        auto e = glm::mat3x3(
            1, 0, 0,
            0, 1, 0,
            0, 0, 1
        );
        auto v = glm::vec3(1, 2, 3);
        auto res = matrix_mult(e, v);
        
        for (int i = 0; i < points.size(); i++) {
            glm::vec3 values = glm::vec3(points[i][0], points[i][1], 1);
            values = matrix_mult(rotation_matrix, values);
            item->points[i] = glm::vec3(values[0], values[1], 1.0);
        }
    }

    void scale_from_point(Primitive* item, glm::vec3 point, double x_factor, double y_factor) {
        if (item == nullptr) {
            return;
        }

        auto scale_matrix = this->build_scale_matrix(point[0], point[1], x_factor, y_factor);

        auto points = item->points;

        for (int i = 0; i < points.size(); i++) {
            glm::vec3 values = glm::vec3(points[i][0], points[i][1], 1);
            values = scale_matrix * values;
            item->points[i] = glm::vec3(values[0], values[1], 1);;
        }
    }

    void rotate_90(Primitive* item) {
        if (item == nullptr) {
            return;
        }
        auto point = get_center(item->points);
        this->rotate_around_point(item, point, 90.0);
    }
};