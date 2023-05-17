#pragma once
#include "GL/glew.h"
#include "GLM/vec3.hpp"
#include <GLM/geometric.hpp>
#define _USE_MATH_DEFINES
#include <math.h>
#include <set>
#include <vector>
#include <algorithm>
#include "Photon.h"
#include "queue"
#include "PMSettingsUpdater.h"
class PhotonMap {
    struct Node {
        Photon value;
        short plane;
        Node* left;
        Node* right;
        Node(Node* left = nullptr, Node* right = nullptr);
        Node(const Photon& value, short plane, Node* left = nullptr, Node* right = nullptr);
        ~Node();
    };
    struct NPNode { // TODO ��� ��������������
        const Node* node;
        float sq_dist;
        NPNode(const Node* photon, float sq_dist);
    };
    struct NPNodeCopmarator {
        NPNodeCopmarator() {}
        bool operator() (const NPNode& f, const NPNode& s) const;
    };
    class NPContainer : public std::set<NPNode, NPNodeCopmarator> {
        size_t _capacity;
    public:
        NPContainer(size_t capacity);
        void push(const NPNode& elem);
        void pop_last();
        const NPNode top() const;
        std::vector<Photon> to_vector() const;
        size_t capacity() const ;
        float max_sq_dist() const;
        float min_sq_dist() const;
    };
    class NPContainerQ : public std::priority_queue<NPNode, std::vector<NPNode>, NPNodeCopmarator> {
        size_t _capacity;
    public:
        NPContainerQ(size_t capacity);
        void fpush(const NPNode& elem);
        void push(const NPNode& elem);
        void pop();
        size_t capacity() const;
        float max_dist() const;
        float min_dist() const;
        const Photon& operator[](size_t index);
    };
    struct NearestPhotons {
        glm::vec3 pos;
        glm::vec3 normal;
        NPContainerQ container;
        NearestPhotons(const glm::vec3& pos, const glm::vec3& normal, size_t capacity);
    };
    class Filter {
    public:
        /// <summary>
        /// Normalization coefficient
        /// </summary>
        /// <returns></returns>
        const float norm_coef;
        Filter(float norm_coef);
        float call(const glm::vec3& ppos, const glm::vec3& loc, float r);
    };
    class ConeFilter : public Filter {
        float k;
    public:
        ConeFilter(float k);
        /// <summary>
        /// ���������� ���������� ��� ��������� ������ ������ � ����������� �� ��� ���������� �� ��������������� �����.
        /// </summary>
        /// <param name="ppos">Photon position</param>
        /// <param name="loc">Location where ray hitting a surface</param>
        /// <param name="r">Max distance between loc and nearests photons</param>
        /// <returns></returns>
        float call(const glm::vec3& ppos, const glm::vec3& loc, float r);
    };
    class GaussianFilter : public Filter {
        float alpha, beta;
    public:
        GaussianFilter(float alpha, float beta);
        /// <summary>
        /// ���������� ���������� ��� ��������� ������ ������ � ����������� �� ��� ���������� �� ��������������� �����.
        /// </summary>
        /// <param name="ppos">Photon position</param>
        /// <param name="loc">Location where ray hitting a surface</param>
        /// <param name="r">Squared max distance between loc and nearests photons</param>
        /// <returns></returns>
        float call(const glm::vec3& ppos, const glm::vec3& loc, float r);
    };
public:
    enum Type { def = 0, caustic };
private:
    Type type;
    // glm::vec3** heap; ��������� �������� ����, �.�. ��������� ������.
    float max_distance = 1000; // TODO ��������� � cpp ���� ���
    Node* root;
    size_t size;
    std::vector<Filter*> filters;
    PhotonMapSettings settings;
    /// <summary>
    /// ���������� ��������� �� �������� "���" ����� ���������� �����
    /// </summary>
    /// <param name="bigp"> ������ �����, ����������� ���</param>
    /// <param name="smallp">������ �����, ���������� ���</param>
    short find_largest_plane(const glm::vec3& bigp, const glm::vec3& smallp);
    void update_cube(const glm::vec3& p, glm::vec3& bigp, glm::vec3& smallp);
    void locate(NearestPhotons* const np) const;
    void locate(NearestPhotons* const np, const Node* photon) const;
public:
    std::vector<Photon> locate(const Node* elem, size_t capacity) const;
    std::vector<Photon> locate(const glm::vec3& value, size_t capacity) const;
    bool locate_r(NearestPhotons* np) const;
    bool radiance_estimate(const glm::vec3& inc_dir, const glm::vec3& iloc, 
        const glm::vec3& norm, glm::vec3& out_rad);
    void fill_balanced(const std::vector<Photon>& points);
    PhotonMap();
    PhotonMap(Type type);
    ~PhotonMap();
    void clear();
    void total_locate_time();
    void set_settings_updater(PMSettingsUpdater& pmsu);
};

