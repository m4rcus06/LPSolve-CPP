#include "../include/matrix.h"
#include "../include/linear_program.h"
#include "../include/LPSolver.h"

template <typename T>
void solve(const LinearProgram<T>& lp) {
    Solver<double> sol(lp);
    SolverStatus stats = sol.solve(PivotRule::DANTZIG);
    sol.printSolution(stats);
}

int main() {
    LinearProgram<double> lp(
        Goal::MIN,
        {-10, 57, 9, 24},
        Matrix<double>{{0.5, -5.5, -2.5, 9}, {0.5, -1.5, -0.5, 1}, {1, 0, 0, 0}},
        {0, 0, 1},
        {Relation::LESS_EQUAL, Relation::LESS_EQUAL, Relation::LESS_EQUAL},
        {varType::POSITIVE, varType::POSITIVE, varType::POSITIVE, varType::POSITIVE},
        {"x1", "x2", "x3", "x4"}
    );

    solve<double>(lp);
    return 0;
}
