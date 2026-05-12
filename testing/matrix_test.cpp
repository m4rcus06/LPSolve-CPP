#include "../include/matrix.h"

int main() {
    // Khởi tạo ma trận thử nghiệm
    Matrix<double> A = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };

    std::cout << "Ma tran ban dau:\n";
    A.display();

    // Thử phép xoay (Pivoting) giả lập: 
    // Biến A(1, 1) thành 1 và triệt tiêu các số khác trên cột 1
    std::cout << "Chuan hoa hang 1 (chia cho 5):\n";
    A.multiplyRow(1, 1.0/5.0); 
    A.display();

    std::cout << "Khu hang 0 bang hang 1:\n";
    A.addRow(1, 0, -2.0); // R0 = R0 - 2*R1
    std::cout << "Khu hang 2 bang hang 1:\n";
    A.addRow(1, 2, -8.0); // R2 = R2 - 8*R1
    A.display();

    return 0;
}
