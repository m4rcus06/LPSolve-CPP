#include "../include/matrix.h"
#include "../include/linear_program.h"
#include "../include/LPSolver.h"

int main() {
    LinearProgram<double> lp(
        Goal::MAX,
        {3, 2, -1},
        Matrix<double>{{1, -2, 3}, {2, 1, -1}, {1, 1, 1}},
        {4, 5, 6},
        {Relation::LESS_EQUAL, Relation::GREATER_EQUAL, Relation::EQUAL},
        {varType::POSITIVE, varType::FREE, varType::NEGATIVE},
        {"x1", "x2", "x3"}
    );

    Solver<double> sol(lp);
    sol.display();
    return 0;
}
