#pragma once
#include <random>
#include <numeric>
#include <ctime>
#include <iostream>
#include <GLM/ext/matrix_transform.hpp>
#include <queue>
#include <set>
template <typename T>
struct Random {
    /// <summary>
    /// ������������� ������� ������ ������� �����
    /// </summary>
    static void set_seed() {
        std::srand(static_cast <unsigned> (std::time(0)));
    }
    /// <summary>
    /// ������������� ������� ������ ���������� ��������
    /// </summary>
    /// <param name="seed"></param>
    static void set_seed(unsigned seed) {
        std::srand(seed);
    }
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

class PhotonMap {
    struct Node {
        glm::vec3 value;
        short plane;
        Node* left = nullptr;
        Node* right = nullptr;
        ~Node() {
            delete left;
            delete right;
            left = nullptr;
            right = nullptr;
        }
    };
    struct NPNode { // TODO ��� ��������������
        const Node* photon;
        unsigned int sq_dist;
        NPNode(const Node* photon, unsigned int sq_dist) : photon(photon), sq_dist(sq_dist) {}
    };
    class NPNodeCopmarator {
        NPNodeCopmarator(){}
        bool operator() (const NPNode* f, const NPNode* s) {
            return f->sq_dist < s->sq_dist;
        }
    };
    class NPContainer : private std::set<const NPNode, NPNodeCopmarator> {
        size_t capacity;
    public:
        NPContainer(size_t capacity) : capacity(capacity) {}
        void push(const NPNode& elem) {
            set::insert(elem);
            if (this->size() == capacity + 1) {
                this->erase(std::prev(this->end()));
            }
        }
        const NPNode top() const {
            return *this->begin();
        }
    };
    struct NearestPhotons {
        glm::vec3 pos;
        NPContainer container;
        NearestPhotons(glm::vec3 pos) : pos(pos), container(NPContainer(20)) // TODO ������ ������
        {}
    };
    // glm::vec3** heap; ��������� �������� ����, �.�. ��������� ������.
    size_t max_distance = 1000; // TODO ��������� � cpp ���� ���
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
        node->plane = dim;
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
    void locate(NearestPhotons* const np,
        const int index, const Node* photon) const {
        if (photon == nullptr) {
            return;
        }
        const Node* p = photon;
        float dist1;
        dist1 = np->pos[p->plane] - p->value[p->plane];
        if (dist1 > 0.f) { // if dist1 is positive search right plane
            locate(np, 2 * index + 1, p->right);
            if (dist1 * dist1 < np->container.top().sq_dist) {
                locate(np, 2 * index, p->left);
            }
        }
        else { // dist1 is negative search left first
            locate(np, 2 * index, p->left);
            if (dist1 * dist1 < np->container.top().sq_dist) {
                locate(np, 2 * index + 1, p->right);
            }
        }
        // compute squared distance between current photon and np->pos
        auto dists1 = p->value - np->pos;
        float sq_dist = dists1.x * dists1.x + dists1.y * dists1.y + dists1.z + dists1.z;
        if (sq_dist < np->container.top().sq_dist) {
            np->container.push({ p, sq_dist });
        }
    }
public:
    void locate(const Node* elem) {
        NearestPhotons np(elem->value);
        np.container.push({ elem, max_distance * max_distance });
        locate(&np, 1, root);
    }
    PhotonMap(const std::vector<glm::vec3>& points) {
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
    ~PhotonMap() {
        delete root;
        root = nullptr;
    }
};

template<typename T>
class Stack : public std::stack<T>
{
    T* arr;
    int top;
    int capacity;
    void resize() {
        T* newarr = new T[capacity * 2];
        std::memcpy(newarr, arr, sizeof(T) * capacity);
        delete[] arr;
        arr = newarr;
        capacity *= 2;
    }
public:
    Stack(int size = 10) {
        arr = new T[size];
        capacity = size;
        top = -1;
    }
    ~Stack() {
        delete[] arr;
    }
    void push(T elem) {
        if (top == capacity - 1)
        {
            resize();
        }
        arr[++top] = elem;
    }
    T pop() {
        if (is_empty())
        {
            throw std::exception("Stack is empty");
        }
        return arr[top--];
    }
    T peek(size_t i = 0) {
        if (is_empty())
        {
            throw std::exception("Stack is empty");
        }
        if (top - i < 0) {
            throw std::exception(top - i + " < 0");
        }
        return arr[top - i];
    }
    int size() {
        return top + 1;
    }
    bool is_empty() {
        return top == -1;
    }
};
