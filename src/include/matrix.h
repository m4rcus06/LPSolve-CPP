#ifndef MATRIX_H
#define MATRIX_H

#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <vector>
#include <initializer_list>
#include <cassert>
#include <cmath>

template <typename T>
class Matrix {
private:
    int rows, cols;
    std::vector<std::vector<T>> data;

    void setRow(int r) { this->rows = r; }
    void setCol(int c) { this->cols = c; }
public:
    /*Constructor*/
    Matrix(): rows(0), cols(0), data() {}
    Matrix(int r, int c, T val = 0): rows(r), cols(c) {
        data.assign(r, std::vector<T>(c, val));
    }
    Matrix(std::initializer_list<std::initializer_list<T>> list) {
        this->setRow(list.size());
        this->setCol((rows > 0) ? list.begin()->size() : 0);
        data.reserve(rows);
        for (const auto& row_list: list) {
            assert((int) row_list.size() == cols);
            data.push_back(std::vector<T>(row_list));
        }
    }
    
    /*Access Methods*/
    T& operator()(int r, int c) {
        assert(r >= 0 && c >= 0 && r < rows && c < cols);
        return data[r][c];
    }

    const T& operator()(int r, int c) const {
        assert(r >= 0 && c >= 0 && r < rows && c < cols);
        return data[r][c];
    }

    int rowSize() const { return rows; }
    int colSize() const { return cols; }
    bool empty() const  { return data.empty(); }

     /** 
     * @brief Multiply r-th row by k (0-based index)
     * */
    void multiplyRow(int r, T k) {
        assert(r >= 0 && r < rows);
        for (int j = 0; j < cols; ++j) (*this)(r, j) *= k;
    }

    /**
     * @brief Row(to) = Row(to) + k * Row(from) 
     * */
    void addRow(int from, int to, T k) {
        assert(from >= 0 && from < rows && to >= 0 && to < rows);
        for (int j = 0; j < cols; ++j) (*this)(to, j) += k * (*this)(from, j);
    }

    static Matrix<T> toMatrix(const std::vector<std::vector<T>>& raw_data) {
        int r = (int) raw_data.size();
        int c = (r > 0) ? (int) raw_data[0].size() : 0;
        Matrix<T> result(r, c);
        for (int i = 0; i < r; ++i) {
            assert((int) raw_data[i].size() == c);
            for (int j = 0; j < c; ++j) {
                result(i, j) = raw_data[i][j];
            }
        }
        return result;
    }

    void display() {
        for (int i = 0; i < this->rowSize(); ++i) {
            for (int j = 0; j < this->colSize(); ++j) {
                T print_value = (std::abs(data[i][j]) < 1e-9) ? static_cast<T>(0) : data[i][j];
                std::cout << std::setw(10) << std::fixed << std::setprecision(4) << print_value << ' ';
            }
            std::cout << '\n';
        }
        std::cout << "-----------------------------------------------------------------\n";
    }
};

#endif
