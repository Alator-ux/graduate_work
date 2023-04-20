#include "PhotonMap.h"
#include <GLM/ext/scalar_constants.hpp>
#include "PhotonMapping.h"

/* ========== PhotonMapp class begin ========== */

/* ==========! PhotonMapp substruct !========== */
/* ========== Node struct begin ========== */
PhotonMap::Node::Node(Node* left, Node* right) {
    this->left = left;
    this->right = right;
}
PhotonMap::Node::Node(const Photon& value, short plane, Node* left, Node* right) {
    this->value = value;
    this->plane = plane;
    this->left = left;
    this->right = right;
}
PhotonMap::Node::~Node() {
    delete left;
    delete right;
    left = nullptr;
    right = nullptr;
}
/* ========== Node struct end ========== */

/* ==========! PhotonMapp substruct !========== */
/* ========== NPNode struct begin ========== */
PhotonMap::NPNode::NPNode(const Node* photon, float sq_dist) : node(photon), sq_dist(sq_dist) {}
/* ========== NPNode struct end ========== */

/* ==========! PhotonMapp substruct !========== */
/* ========== NPNodeCopmarator struct begin ========== */
bool PhotonMap::NPNodeCopmarator::operator() (const NPNode& f, const NPNode& s) const {
    return f.sq_dist < s.sq_dist;
}
/* ========== NPNodeCopmarator struct end ========== */

/* ==========! PhotonMapp substruct !========== */
/* ========== NPContainer class begin ========== */
PhotonMap::NPContainer::NPContainer(size_t capacity) : _capacity(capacity) {}
void PhotonMap::NPContainer::push(const NPNode& elem) {
    set::insert(elem);
    if (this->size() == _capacity + 1) {
        this->erase(std::prev(this->end()));
    }
}
void PhotonMap::NPContainer::pop_last() {
    if (this->size() == 0) {
        throw std::exception("Trying pop from zero size container");
    }
    set::erase(std::prev(this->end()));
}
const PhotonMap::NPNode PhotonMap::NPContainer::top() const {
    return *this->begin();
}
size_t PhotonMap::NPContainer::size() const {
    return set::size();
}
std::vector<Photon> PhotonMap::NPContainer::to_vector() const {
    std::vector<Photon> res;
    for (auto it = this->begin(); it != this->end(); it = std::next(it)) {
        res.push_back((*it).node->value);
    }
    return res;
}
float PhotonMap::NPContainer::max_sq_dist() const { return std::prev(this->end())->sq_dist; }
float PhotonMap::NPContainer::min_sq_dist() const { return this->begin()->sq_dist; }
size_t PhotonMap::NPContainer::capacity() const { return _capacity; }
/* ========== NPContainer class end ========== */

/* ==========! PhotonMapp substruct !========== */
/* ========== NearestPhotons struct begin ========== */
PhotonMap::NearestPhotons::NearestPhotons(const glm::vec3& pos, size_t capacity) : pos(pos), container(NPContainer(capacity))
{}
/* ========== NearestPhotons struct end ========== */

/* ==========! PhotonMapp subclass !========== */
/* ========== Filter abstract class begin ========== */
PhotonMap::Filter::Filter(float norm_coef) : norm_coef(norm_coef) {}
/* ========== Filter abstract class end ========== */

/* ==========! PhotonMapp subclass !========== */
/* ========== ConeFilter class begin ========== */
PhotonMap::ConeFilter::ConeFilter(float k) : k(k), Filter(1.f - 2 / (3 * k)) {}
float PhotonMap::ConeFilter::call(const glm::vec3& ppos, const glm::vec3& loc, float r) {
    float dist = glm::distance(ppos, loc);
    return 1.f - dist / (k * r);
}
/* ========== ConeFilter class end ========== */

/* ==========! PhotonMapp subclass !========== */
/* ========== GaussianFilter class begin ========== */
PhotonMap::GaussianFilter::GaussianFilter(float alpha, float beta) : alpha(alpha), beta(beta), Filter(1.f) {}
float PhotonMap::GaussianFilter::call(const glm::vec3& ppos, const glm::vec3& loc, float r) {
    float dist = glm::distance(ppos, loc);
    return alpha * (1.f - (1.f - std::pow(M_E, -beta * dist * dist / (2 * r))) /
        (1.f - std::pow(M_E, -beta)));
}
/* ========== GaussianFilter class end ========== */

short PhotonMap::find_largest_plane(const glm::vec3& bigp, const glm::vec3& smallp) {
    size_t largest_plane = 0;
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
                largest_plane = i;
            }
        }
    }
    return largest_plane;
}
void PhotonMap::update_cube(const glm::vec3& p, glm::vec3& bigp, glm::vec3& smallp) {
    for (size_t point_i = 0; point_i < 3; point_i++) {
        if (p[point_i] > bigp[point_i]) {
            bigp[point_i] = p[point_i];
        }
        if (p[point_i] < smallp[point_i]) {
            smallp[point_i] = p[point_i];
        }
    }
}
void PhotonMap::locate(NearestPhotons* const np,
    const int index, const Node* photon) const {
    if (photon == nullptr) {
        return;
    }
    const Node* p = photon;
    float dist1;
    dist1 = np->pos[p->plane] - p->value.pos[p->plane];
    if (dist1 > 0.f) { // if dist1 is positive search right plane
        locate(np, 2 * index + 1, p->right);
        if (dist1 * dist1 < np->container.max_sq_dist()) {
            locate(np, 2 * index, p->left);
        }
    }
    else { // dist1 is negative search left first
        locate(np, 2 * index, p->left);
        if (dist1 * dist1 < np->container.max_sq_dist()) {
            locate(np, 2 * index + 1, p->right);
        }
    }
    // compute squared distance between current photon and np->pos
    auto dists1 = p->value.pos - np->pos;
    float sq_dist = dists1.x * dists1.x + dists1.y * dists1.y + dists1.z * dists1.z;
    np->container.push(NPNode(p, sq_dist));
}
std::vector<Photon>  PhotonMap::locate(const Node* elem, size_t capacity) const {
    NearestPhotons np(elem->value.pos, capacity);
    np.container.push({ elem, max_distance * max_distance });
    locate(&np, 1, root);
    if (np.container.size() < np.container.capacity()) {
        np.container.pop_last();
    }
    return np.container.to_vector();
}
std::vector<Photon> PhotonMap::locate(const glm::vec3& value, size_t capacity) const {
    NearestPhotons np(value, capacity);
    Node* susNode = new Node(Photon(value, glm::vec3(0.f), glm::vec3(0.f)), 0);
    np.container.push({ susNode, max_distance * max_distance });
    locate(&np, 1, root);
    if (np.container.size() < np.container.capacity()) {
        np.container.pop_last();
    }
    delete susNode;
    return np.container.to_vector();
}
const int np_size = 8;
bool PhotonMap::radiance_estimate(const glm::vec3& inc_dir, const glm::vec3& iloc, const glm::vec3& norm,
    Type type, glm::vec3& out_rad) {
    
    auto nearest_photons = locate(iloc, np_size);
    if (nearest_photons.size() < np_size) {
        return false;
    }

    out_rad.x = out_rad.y = out_rad.z = 0;
    float r, filter_r;
    filter_r = r = glm::distance(iloc, (std::prev(nearest_photons.end()))->pos);
    if (type == Type::caustic) {
        filter_r *= filter_r;
    }
    for (auto& p : nearest_photons) { // str 88
        float cosNL = glm::dot(-p.inc_dir, norm);
        if (cosNL > 0.f) { 
            out_rad += p.power *cosNL;// *(this->filters[type])->call(p.pos, iloc, filter_r);
            //out_rad += p.power * (this->filters[type])->call(p.pos, iloc, filter_r);
        }
    }

    //float temp = (1.f / (M_PI * r * r * this->filters[type]->norm_coef));
    float temp = (1.f / (M_PI * (r * r)));// *this->filters[type]->norm_coef));
    out_rad *= temp;
    return true;
}
PhotonMap::PhotonMap() {
    this->size = 0;
    this->root = nullptr;
}
void PhotonMap::fill_balanced(const std::vector<Photon>& photons) {
    size = photons.size();
    
    filters = std::vector<Filter*>(2);
    filters[0] = new ConeFilter(cf_k);
    filters[1] = new GaussianFilter(gf_alpha, gf_beta);
    //heap = new glm::vec3*[size];

    if (photons.size() == 0) {
        root = nullptr;
        return;
    }
    std::cout << "Filling balanced KD-tree (photon map) started" << std::endl;
    /*
    * нахождение "куба", захватывающего все точки и заполнение вектор указателей для работы с ними
    * + перевод вектор точек в вектор указателей на точки для дальнейшего удобства
    */
    std::vector<const Photon*> photons_pointers(photons.size());
    glm::vec3 bigp(photons[0].pos), smallp(photons[0].pos);
    for (size_t i = 0; i < photons.size(); i++) {
        photons_pointers[i] = &photons[i];
        update_cube(photons[i].pos, bigp, smallp);
    }

    /*
    * нахождение измерения с наибольшей длиной
    */
    auto largest_plane = find_largest_plane(bigp, smallp);

    size_t count = 0;
    // from, to, plane, node
    std::stack<std::tuple<size_t, size_t, short, Node**>> ftpn_recur;
    ftpn_recur.push({ 0, photons_pointers.size(), largest_plane, &root });
    while (!ftpn_recur.empty()) {
        short plane;
        size_t from, to;
        Node** node;
        std::tie(from, to, plane, node) = ftpn_recur.top();
        ftpn_recur.pop();
        if (to - from == 0) {
            node = nullptr;
            continue;
        }
        count++;
        
        if (count % ((PHOTONS_COUNT / 10)) == 0) {
            std::cout << "\tPhotons inserted: " << count << std::endl;
        }
        *node = new Node();
        (*node)->plane = plane;
        if (to - from == 1) {
            (*node)->value = *photons_pointers[from];
        }

        // Сортировка для нахождения среднего. Сортируем указатели.
        std::sort(std::next(photons_pointers.begin(), from), std::next(photons_pointers.begin(), to),
            [&plane](const Photon* p1, const Photon* p2) { return p1->pos[plane] < p2->pos[plane]; });
        size_t mid = (to - from) / 2 + from;
        const Photon* medium = photons_pointers[mid];

        std::vector<const Photon*> s1, s2;
        glm::vec3 left_bigp(std::numeric_limits<float>::lowest()), left_smallp(std::numeric_limits<float>::max());
        glm::vec3 right_bigp(left_bigp), right_smallp(left_smallp);
        std::for_each(std::next(photons_pointers.begin(), from), std::next(photons_pointers.begin(), mid),
            [&left_bigp, &left_smallp, this](const Photon* p) {
                update_cube(p->pos, left_bigp, left_smallp);
            });
        std::for_each(std::next(photons_pointers.begin(), mid + 1), std::next(photons_pointers.begin(), to),
            [&right_bigp, &right_smallp, this](const Photon* p) {
                update_cube(p->pos, right_bigp, right_smallp);
            });

        size_t left_plane = find_largest_plane(left_bigp, left_smallp);
        size_t right_plane = find_largest_plane(right_bigp, right_smallp);

        (*node)->value = *medium;
        ftpn_recur.push({ from, mid, left_plane, &(*node)->left }); // left
        ftpn_recur.push({ mid + 1, to, right_plane, &(*node)->right }); // right
    }
    std::cout << "\tPhotons inserted: " << count << std::endl;
    std::cout << "Filling balanced KD-tree (photon map) ended" << std::endl;
}
PhotonMap::~PhotonMap() {
    delete root;
    root = nullptr;
    for (Filter* filter : filters) {
        delete filter;
    }
}

/* ========== PhotonMapp class end ========== */
