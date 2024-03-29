#pragma once
#include <random>
#include <numeric>
#include <iostream>
#include <GLM/ext/matrix_transform.hpp>
template <typename T>
struct Random {
    /// <summary>
    /// ���������� ��������� ����� �� 0.0 �� 1.0 ������������
    /// </summary>
    static T random() {
        return static_cast<T>(rand()) / static_cast<T>(RAND_MAX);
    }
    /// <summary>
    /// ���������� ��������� ����� �� 'from' �� 'to' ������������
    /// </summary>
    static T random(T from, T to) {
        return from + static_cast<T>(rand()) / (static_cast<T>(RAND_MAX) / static_cast<T>(to - from));
    }
};

class BalancedKDTree {
    struct Node {
        glm::vec3 value;
        Node* left = nullptr;
        Node* right = nullptr;
        ~Node() {
            delete left;
            delete right;
            left = nullptr;
            right = nullptr;
        }
    };
    // glm::vec3** heap; ��������� �������� ����, �.�. ��������� ������.
    Node* root;
    size_t size;
    /// <summary>
    /// ���������� ��������� �� �������� "���" ����� ���������� �����
    /// </summary>
    /// <param name="bigp"> ������ �����, ����������� ���</param>
    /// <param name="smallp">������ �����, ���������� ���</param>
    size_t find_largest_dim(const glm::vec3& bigp, const glm::vec3& smallp) {
        size_t largest_dim = 0;
        {
            glm::vec3 dims(
                std::abs(bigp.x - smallp.x),
                std::abs(bigp.y - smallp.y),
                std::abs(bigp.z - smallp.z)
            );
            float max = dims[0];
            for (size_t i = 1; i < 3; i++) {
                if (dims[i] > max) {
                    max = dims[i];
                    largest_dim = i;
                }
            }
        }
        return largest_dim;
    }
    void update_cube(const glm::vec3* p, glm::vec3& bigp, glm::vec3& smallp) {
        for (size_t point_i = 0; point_i < 3; point_i++) {
            if ((*p)[point_i] > bigp[point_i]) {
                bigp[point_i] = (*p)[point_i];
            }
            if ((*p)[point_i] < smallp[point_i]){
                smallp[point_i] = (*p)[point_i];
            }
        }
    }
    Node* fill_balanced(size_t dim, std::vector<const glm::vec3*>& points) {
        if (points.size() == 0) {
            return nullptr;
        }
        Node* node = new Node();
        if (points.size() == 1) {
            node->value = *points[0];
            return node;
        }

        // ���������� ��� ���������� ��������. ��������� ���������.
        std::sort(points.begin(), points.end(),
            [&dim](const glm::vec3* p1, const glm::vec3* p2) { return (*p1)[dim] < (*p2)[dim]; });
        size_t mid = points.size() / 2;
        const glm::vec3* medium = points[mid];

        std::vector<const glm::vec3*> s1, s2;
        glm::vec3 left_bigp(std::numeric_limits<float>::lowest()), left_smallp(std::numeric_limits<float>::max());
        glm::vec3 right_bigp(left_bigp), right_smallp(left_smallp);
        for (size_t i = 0; i < mid; i++) {
            if ((*points[i])[dim] < (*medium)[dim]) {
                s1.push_back(points[i]); // all points below median
                update_cube(points[i], left_bigp, left_smallp);
            }
            else {
                s2.push_back(points[i]); // all points above median
                update_cube(points[i], right_bigp, right_smallp);
            }
        }
        for (size_t i = mid + 1; i < points.size(); i++) { // ��� �����, ����� �� ��������� �� ==
            if ((*points[i])[dim] < (*medium)[dim]) {
                s1.push_back(points[i]);
                update_cube(points[i], left_bigp, left_smallp);
            }
            else {
                s2.push_back(points[i]);
                update_cube(points[i], right_bigp, right_smallp);
            }
        }

        size_t left_dim = find_largest_dim(left_bigp, left_smallp);
        size_t right_dim = find_largest_dim(right_bigp, right_smallp);

        node->value = *medium;
        node->left = fill_balanced(left_dim, s1); // left
        node->right = fill_balanced(right_dim, s2); // right
        return node;
    }
public:
    BalancedKDTree(const std::vector<glm::vec3>& points) {
        size = points.size();
        //heap = new glm::vec3*[size];
        
        /*
        * ���������� "����", �������������� ��� ����� � ���������� ������ ���������� ��� ������ � ����
        * + ������� ������ ����� � ������ ���������� �� ����� ��� ����������� ��������
        */
        std::vector<const glm::vec3*> points_pointers(points.size());
        points_pointers[0] = &points[0];
        glm::vec3 bigp(points[0]), smallp(points[0]);
        for (size_t vec_i = 1; vec_i < points.size(); vec_i++) {
            points_pointers[vec_i] = &points[vec_i];
            for (size_t point_i = 0; point_i < 3; point_i++) {
                if (points[vec_i][point_i] > bigp[point_i]) {
                    bigp[point_i] = points[vec_i][point_i];
                }
                else { // if (points[vec_i][point_i] < smallp[point_i])
                    smallp[point_i] = points[vec_i][point_i];
                }
            }
        }

        /*
        * ���������� ��������� � ���������� ������
        */
        auto largest_dim = find_largest_dim(bigp, smallp);

        root = fill_balanced(largest_dim, points_pointers);
    }
    ~BalancedKDTree() {
        delete root;
        root = nullptr;
    }
};
