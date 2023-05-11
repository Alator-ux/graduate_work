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
    struct NPNode { // TODO как укомплектовать
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
    class Filter abstract {
    public:
        /// <summary>
        /// Normalization coefficient
        /// </summary>
        /// <returns></returns>
        const float norm_coef;
        Filter(float norm_coef);
        virtual float call(const glm::vec3& ppos, const glm::vec3& loc, float r) abstract;
    };
    class ConeFilter : public Filter {
        float k;
    public:
        ConeFilter(float k);
        /// <summary>
        /// ¬озвращает коэфициент дл€ изменени€ вклада фотона в зависимости от его рассто€ни€ до рассматриваемой точки.
        /// </summary>
        /// <param name="ppos">Photon position</param>
        /// <param name="loc">Location where ray hitting a surface</param>
        /// <param name="r">Max distance between loc and nearests photons</param>
        /// <returns></returns>
        float call(const glm::vec3& ppos, const glm::vec3& loc, float r) override;
    };
    class GaussianFilter : public Filter {
        float alpha, beta;
    public:
        GaussianFilter(float alpha, float beta);
        /// <summary>
        /// ¬озвращает коэфициент дл€ изменени€ вклада фотона в зависимости от его рассто€ни€ до рассматриваемой точки.
        /// </summary>
        /// <param name="ppos">Photon position</param>
        /// <param name="loc">Location where ray hitting a surface</param>
        /// <param name="r">Squared max distance between loc and nearests photons</param>
        /// <returns></returns>
        float call(const glm::vec3& ppos, const glm::vec3& loc, float r) override;
    };
public:
    enum Type { def = 0, caustic = 1};
private:
    struct Settings {
        size_t np_size = 2000;
        // Cone filter coef k
        const float cf_k = 1.1f; // Must be >= 1
        // Gaussian filter coef alpha
        const float gf_alpha = 1.818f;
        // Gaussian filter coef beta
        const float gf_beta = 1.953f;
        float disc_compression = 1.6f;
    };
    Type type;
    // glm::vec3** heap; “отальный проигрыш куче, т.к. непон€тен размер.
    float max_distance = 1000; // TODO перенести в cpp пока что
    Node* root;
    size_t size;
    std::vector<Filter*> filters;
    Settings settings;
    /// <summary>
    /// ¬озвращает измерение по которому "куб" имеет наибольшую длину
    /// </summary>
    /// <param name="bigp"> ѕерва€ точка, описывающа€ куб</param>
    /// <param name="smallp">¬тора€ точка, описывающа куб</param>
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
    void update_np_size(size_t size);
    void update_disc_compression(float coef);
};

