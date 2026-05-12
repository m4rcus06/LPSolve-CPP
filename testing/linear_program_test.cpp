#include <iostream>
#include <vector>
#include "../include/matrix.h"
#include "../include/linear_program.h"

int main() {
    // 1. Khai báo Goal: Maximize
    Goal myGoal = Goal::MAX;

    // 2. Hệ số hàm mục tiêu: Z = 3x1 - 5.5x2 + 0.0001x3
    // Test logic: dấu cộng, dấu trừ, và số thập phân nhỏ
    std::vector<double> c = {3.0, -5.5, 0.0001};

    // 3. Ma trận ràng buộc A (3 dòng, 3 cột)
    Matrix<double> A(3, 3);
    // Dòng 1: 1.5x1 + 2x2 - 4x3
    A(0, 0) = 1.5;  A(0, 1) = 2.0;  A(0, 2) = -4.0;
    // Dòng 2: -1x1 + 0x2 + 3.1415x3
    A(1, 0) = -1.0; A(1, 1) = 0.0;  A(1, 2) = 3.1415926;
    // Dòng 3: 0x1 - 10x2 + 0x3
    A(2, 0) = 0.0;  A(2, 1) = -10.0; A(2, 2) = 0.0;

    // 4. Vế phải b
    std::vector<double> b = {100.0, 0.0, -25.5};

    // 5. Các loại dấu ràng buộc
    std::vector<Relation> rels = {
        Relation::LESS_EQUAL,    // <=
        Relation::EQUAL,         // =
        Relation::GREATER_EQUAL  // >=
    };

    // 6. Khởi tạo Object LP
    LinearProgram<double> testLP(myGoal, c, A, b, rels);

    // 7. Gọi hàm display của Hào
    testLP.display();

    // 8. Thử nghiệm Normalize (sau khi Hào viết xong hàm normalize)
    /*
    std::cout << "\n--- After Normalization ---\n";
    testLP.normalize();
    testLP.display();
    */

    return 0;
}
