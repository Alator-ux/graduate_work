#pragma once
#include <random>
#include <numeric>
#include <iostream>
template <typename T>
struct Random {
    /// <summary>
    /// ¬озвращает случайное число от 0.0 до 1.0 включительно
    /// </summary>
    static T random() {
        return static_cast<T>(rand()) / static_cast<T>(RAND_MAX);
    }
    /// <summary>
    /// ¬озвращает случайное число от 'from' до 'to' включительно
    /// </summary>
    static T random(T from, T to) {
        return from + static_cast<T>(rand()) / (static_cast<T>(RAND_MAX) / static_cast<T>(to - from));
    }
};
struct BalancedKDTree {
    struct Node {
        glm::vec3 value;
        Node* left = nullptr;
        Node* right = nullptr;
        ~Node() {
            delete left;
            delete right;
        }
    };
    // glm::vec3** heap; “отальный проигрыш куче, т.к. непон€тен размер.
    Node root;
    size_t size;
    /// <summary>
    /// ¬озвращает измерение по которому "куб" имеет наибольшую длину
    /// </summary>
    /// <param name="bigp"> ѕерва€ точка, описывающа€ куб</param>
    /// <param name="smallp">¬тора€ точка, описывающа куб</param>
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
    void fill_balanced(size_t cur_i, size_t dim, std::vector<const glm::vec3*>& points) {
        if (points.size() == 0) {
            return;
        }
        if (points.size() == 1) {// ј если 0?
            set(cur_i - 1, *points[0]);
            return;
        }

        // —ортировка дл€ нахождени€ среднего. —ортируем указатели.
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
        for (size_t i = mid + 1; i < points.size(); i++) { // два цикла, чтобы не провер€ть на ==
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

        set(cur_i - 1, (*medium)); // смещение индкса влево, т.к. изначально передаетс€ единица дл€ корректного умножени€
        fill_balanced(cur_i * 2, left_dim, s1); // left
        fill_balanced(cur_i * 2 + 1, right_dim, s2); // right
    }
    BalancedKDTree(const std::vector<glm::vec3>& points) {
        size = points.size();
        //heap = new glm::vec3*[size];
        
        /*
        * нахождение "куба", захватывающего все точки и заполнение вектор указателей дл€ работы с ними
        * + перевод вектор точек в вектор указателей на точки дл€ дальнейшего удобства
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
        * нахождение измерени€ с наибольшей длиной
        */
        auto largest_dim = find_largest_dim(bigp, smallp);

        fill_balanced(1, largest_dim, points_pointers);
        //std::for_each(points_pointers.begin(), points_pointers.end(), [](const glm::vec3* p) { delete p; });
    }
    ~BalancedKDTree() {
        //std::for_each(heap, heap + size, [](glm::vec3* pointer) { delete pointer; }); // а мб и нет
        //delete[] heap;
    }
    void set(size_t i, const glm::vec3& value) {
        heap[i] = new glm::vec3(value);
    }
    void print() {
        for (size_t i = 1; i <= size; i++) {
            std::cout << "Index: " << i << "; value: (" << (*heap)[i - 1].x << "," << (*heap)[i - 1].y << "," << (*heap)[i - 1].z
                << "); left|right: " << i * 2 << "|" << i * 2 + 1 << std::endl;
        }
    }
};
