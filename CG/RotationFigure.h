#pragma once
#include "ThreeDInterface.h"
#include <functional>
#include "3DChanger.h"
#include "Primitives.h"

class RotationFigure : public HighLevelInterface {
    primitives::Line base;
public:
    RotationFigure(primitives::Line base) {
        this->base = base;
        this->objects = std::vector<primitives::Primitive>();
        this->objects.push_back(this->base);
        this->type = ThreeDTypes::rotation_figure;
    }

    void build(Axis axis, size_t partitions_count) {
        float step = 360.f / partitions_count;
        float angle = 0;

        for (size_t i = 0; i < partitions_count; i++) {
            primitives::Line new_line = base.copy();
            rotate_line1(&new_line, axis, angle);
            objects.push_back(new_line);
            angle += step;
        }
    }

    glm::vec3 center() {
        float x = 0, y = 0, z = 0;
        auto res = glm::vec3(0.0f);
        for (auto& prim : objects) {
            primitives::Line* line = reinterpret_cast<primitives::Line*>(&prim);
            res += line->center();
        }
        size_t size = objects.size();
        res /= size;
        return res;
    }

    void transform(const glm::mat4x4& transform_matrix) {
        for (size_t i = 0; i < objects.size(); i++) {
            primitives::Line* line = reinterpret_cast<primitives::Line*>(&objects[i]);
            line->transform(transform_matrix);
        }
    }
};