#pragma once
#include <random>
#include <numeric>
#include <ctime>
#include <iostream>
#include <GLM/ext/matrix_transform.hpp>
#include <queue>
#include <set>
#include <stack>
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