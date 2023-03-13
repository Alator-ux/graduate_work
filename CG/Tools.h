#pragma once
#include <random>
#include <numeric>
template <typename T>
struct Random {
    /// <summary>
    /// Возвращает случайное число от 0.0 до 1.0 включительно
    /// </summary>
    static T random() {
        return static_cast<T>(rand()) / static_cast<T>(RAND_MAX);
    }
    /// <summary>
    /// Возвращает случайное число от 'from' до 'to' включительно
    /// </summary>
    static T random(T from, T to) {
        return from + static_cast<T>(rand()) / (static_cast<T>(RAND_MAX) / static_cast<T>(to - from));
    }
};
struct KDTree {
    struct Vec3Comp {
        const std::vector<const glm::vec3*>& array;
        size_t dim;
        Vec3Comp(size_t dim, const std::vector<const glm::vec3*>& array) : array(array) {
            this->dim = dim;
        }
        inline bool operator()(const size_t& i, const size_t& j) {
            return (*array[i])[dim] < (*array[j])[dim];
        }
    };
    glm::vec3* heap;
    size_t size;
    /*std::pair<size_t, size_t> find_median(const std::vector<glm::vec3*>& points) { // попоробовать искать по заранее отсортированному
        glm::vec3 bigp(*points[0]), smallp(*points[0]), average;
        for (size_t vec_i = 1; vec_i < points.size(); vec_i++) {
            for (size_t point_i = 0; point_i < 3; point_i++) {
                average[point_i] += (*points[vec_i])[point_i];

                if ((*points[vec_i])[point_i] > bigp[point_i]) {
                    bigp[point_i] = (*points[vec_i])[point_i];
                }
                if ((*points[vec_i])[point_i] < smallp[point_i]) { // TODO скорее всего заменить на елсе
                    smallp[point_i] = (*points[vec_i])[point_i];
                }
            }
        }
        average /= points.size();

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

        size_t medium_ind = 0;
        float medium_dist = std::abs((*points[0])[largest_dim] - average[largest_dim]);
        for (size_t vec_i = 1; vec_i < points.size(); vec_i++) {
            float cur_dist = std::abs((*points[vec_i])[largest_dim] - average[largest_dim]);
            if (cur_dist < medium_dist) {
                medium_dist = cur_dist;
                medium_ind = vec_i;
            }
        }

        return { medium_ind, largest_dim };
    }*/
    /// <summary>
    /// Возвращает измерение по которому "куб" имеет наибольшую длину
    /// </summary>
    /// <param name="bigp"> Первая точка, описывающая куб</param>
    /// <param name="smallp">Вторая точка, описывающа куб</param>
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
            if ((*p)[point_i] < smallp[point_i]) { // TODO скорее всего заменить на елсе
                smallp[point_i] = (*p)[point_i];
            }
        }
    }
    void fill_balanced(size_t cur_i, size_t dim, std::vector<const glm::vec3*>& points) {
        if (points.size() == 1) {// А если 0?
            set(cur_i, *points[0]);
            return;
        }
        // Сортировка для нахождения среднего. Сортируем указатели.
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
        for (size_t i = mid + 1; i < points.size(); i++) { // два цикла, чтобы не проверять на ==
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

        set(cur_i - 1, (*medium)); // смещение индкса влево, т.к. изначально передается единица для корректного умножения
        fill_balanced(cur_i * 2, left_dim, s1); // left
        fill_balanced(cur_i * 2 + 1, right_dim, s2); // right
    }
    KDTree(const std::vector<glm::vec3>& points) {
        size = points.size();
        heap = new glm::vec3[size];

        /*
        * нахождение "куба", захватывающего все точки и заполнение вектор указателей для работы с ними
        * + перевод вектор точек в вектор указателей на точки для дальнейшего удобства
        */
        std::vector<const glm::vec3*> points_pointers(points.size());
        glm::vec3 bigp(points[0]), smallp(points[0]);
        for (size_t vec_i = 1; vec_i < points.size(); vec_i++) {
            points_pointers[vec_i] = &points[vec_i];
            for (size_t point_i = 0; point_i < 3; point_i++) {
                if (points[vec_i][point_i] > bigp[point_i]) {
                    bigp[point_i] = points[vec_i][point_i];
                }
                if (points[vec_i][point_i] < smallp[point_i]) { // TODO скорее всего заменить на елсе
                    smallp[point_i] = points[vec_i][point_i];
                }
            }
        }

        /*
        * нахождение измерения с наибольшей длиной
        */
        auto largest_dim = find_largest_dim(bigp, smallp);

        fill_balanced(1, largest_dim, points_pointers);

        std::for_each(points_pointers.begin(), points_pointers.end(), [](const glm::vec3* p) { delete p; });
    }
    ~KDTree() {
        //std::for_each(heap, heap + size, [](glm::vec3* pointer) { delete pointer; }); // а мб и нет
        delete[] heap;
    }
    void set(size_t i, const glm::vec3& value) {
        heap[i] = value;
    }
    void set_left(size_t i, const glm::vec3& value) {
        heap[2 * i] = value;
    }
    void set_right(size_t i, const glm::vec3& value) {
        heap[2 * i + 1] = value;
    }
};
