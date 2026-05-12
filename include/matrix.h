#ifndef MATRIX_H
#define MATRIX_H

#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <vector>
#include <initializer_list>
#include <cassert>

template <typename T>
class Matrix {
private:
    int rows, cols;
    std::vector<T> data;
public:
    /*Constructor*/
    Matrix(): rows(0), cols(0), data() {}
    Matrix(int r, int c, T val = 0): rows(r), cols(c), data(r * c, val) {}
    Matrix(std::initializer_list<std::initializer_list<T>> list) {
        rows = list.size();
        cols = (rows > 0) ? list.begin()->size() : 0;
        data.reserve(rows * cols);
        for (const auto& row_list: list) {
            for (const auto& element: row_list) {
                data.push_back(element);
            }
        }
    }
    
    /*Access Methods*/
    T& operator()(int r, int c) {
        assert(r * cols + c < rows * cols);
        return data[r * cols + c];
    }

    const T& operator()(int r, int c) const {
        return data[r * cols + c];
    }

    int rowSize() const { return rows; }
    int colSize() const { return cols; }

    /** 
     * @brief Multiply r-th row by k (0-based index)
     * */
    void multiplyRow(int r, T k) {
        assert(r >= 0 && r < rows);
        for (int j = 0; j < cols; ++j) (*this)(r, j) *= k;
    }

    /** 
     * @brief Multiply c-th column by k (0-based index)
     * */
    void multiplyCol(int c, T k) {
        assert(c >= 0 && c < cols);
        for (int i = 0; i < rows; ++i) (*this)(i, c) *= k;
    }

    /**
     * @brief Row(to) = Row(to) + k * Row(from) 
     * */
    void addRow(int from, int to, T k) {
        assert(from >= 0 && from < rows && to >= 0 && to < rows);
        for (int j = 0; j < cols; ++j) (*this)(to, j) += k * (*this)(from, j);
    }

    /**
     * @brief Col(to) = Col(to) + k * Col(from) 
     * */
    void addCol(int from, int to, T k) {
        assert(from >= 0 && from < cols && to >= 0 && to < cols);
        for (int i = 0; i < rows; ++i) (*this)(i, to) += k * (*this)(i, from);
    }

    /**
     * @brief Swap two row r1 an r2
     * */
    void swapRow(int r1, int r2) {
        assert(r1 >= 0 && r1 < rows && r2 >= 0 && r2 < rows);
        for (int j = 0; j < cols; ++j) std::swap((*this)(r1, j), (*this)(r2, j));
    }

    static Matrix<T> Identity(int n) {
        Matrix<T> result(n, n);
        for (int i = 0; i < n; ++i) result(i, i) = static_cast<T>(1);
        return result;
    }

    void display() {
        for (int i = 0; i < rows * cols; ++i) {
            if (i > 0 && i % cols == 0) std::cout << '\n';
            T print_value = (std::abs(data[i]) < 1e-9) ? static_cast<T>(0) : data[i];
            std::cout << std::setw(10) << std::fixed << std::setprecision(4) << print_value << ' ';
        }
        std::cout << '\n';
    }
};

#endif
