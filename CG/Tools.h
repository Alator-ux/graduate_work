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
public:
    void start() {
        start_time = std::chrono::high_resolution_clock::now();
    }
    void end() {
        end_time = std::chrono::high_resolution_clock::now();
    }
    long long milliseconds(std::chrono::steady_clock::time_point end, std::chrono::steady_clock::time_point begin)
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
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
