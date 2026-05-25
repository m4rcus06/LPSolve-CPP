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
        Goal::MAX,
        {-1, -3, -1},
        Matrix<double>{{2, -5, 1}, {2, -1, 2}},
        {-5, 4},
        {Relation::LESS_EQUAL, Relation::LESS_EQUAL},
        {varType::POSITIVE, varType::POSITIVE, varType::POSITIVE},
        {"x1", "x2", "x3"}
    );

    solve<double>(lp);
    return 0;
}
