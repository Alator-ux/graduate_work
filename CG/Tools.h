#pragma once
#include <random>
#include <numeric>
#include <ctime>
#include <iostream>
#include <GLM/ext/matrix_transform.hpp>
#include <queue>
#include <set>
#include <chrono>
#include <stack>
template <typename T>
struct Random {
    /// <summary>
    /// Устанавливает текущим зерном текущее время
    /// </summary>
    static void set_seed() {
        std::srand(static_cast <unsigned> (std::time(0)));
    }
    /// <summary>
    /// Устанавливает текущим зерном переданный параметр
    /// </summary>
    /// <param name="seed"></param>
    static void set_seed(unsigned seed) {
        std::srand(seed);
    }
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
class Timer {
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point end_time;
    long long _duration;
    long long total;
    int status;
    long long milliseconds()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    }
public:
    Timer() {
        clear();
    }
    void start() {
        status = 1;
        start_time = std::chrono::high_resolution_clock::now();
    }
    void end() {
        status = 0;
        end_time = std::chrono::high_resolution_clock::now();
        _duration = milliseconds();
    }
    void check_point() {
        status = 2;
        end_time = std::chrono::high_resolution_clock::now();
        _duration = milliseconds();
    }
    void sum_total() {
        total += _duration;
    }
    long long duration() {
        if (status == 0) {
            throw std::exception("Timer is still running");
        }
        return _duration;
    }
    void print() {
        std::cout << "Time: " << duration() << std::endl;
    }
    void print_total() {
        std::cout << "Total time: " << total << std::endl;
    }
    void clear() {
        total = 0;
        status = 0;
    }
};
bool vec3_equal(const glm::vec3& f, const glm::vec3& s);
struct Vec3Hash {
    size_t operator()(const glm::vec3& v) const {
        size_t h1 = std::hash<float>()(v.x);
        size_t h2 = std::hash<float>()(v.y);
        size_t h3 = std::hash<float>()(v.z);
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

template<typename T>
class DeepLookStack : public std::stack<T>
{
    int top = -1;
public:
    void push(const T& elem) {
        std::stack<T>::push(elem);
        top++;
    }
    T peek(size_t left_shift = 0) {
        if (this->empty())
        {
            throw std::exception("Stack is empty");
        }
        if (top - left_shift < 0) {
            throw std::exception(top - left_shift + " < 0");
        }
        return this->c[top - left_shift];
    }
    void pop() {
        std::stack<T>::pop();
        top--;
    }
};
