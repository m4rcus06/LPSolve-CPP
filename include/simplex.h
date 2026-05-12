#ifdef SIMPLEX_SOLVER_H
#define SIMPLEX_SOLVER_H

#include "matrix.h"
#include <vector>
#include <string>

enum class SolverStatus {
    OPTIMAL,        //< Optimal solution found
    INFEASIBLE,     //< No feasible solution exists
    UNBOUNDED,      //< Objective function can be increased infinitely
    RUNNING         //< Solver still running
};

template <typename T>
class SimplexSolver {
private:
    Matrix<T> tableau;
    int numConstraints;         //< of constrains
    int numVariables;           //< of decision variables
    std::vector <int> basis;    //< Indices of basis variable for each row
    const T EPS = 1e-9;         //< precision
public:
    SimplexSolver();
    void solve();
};
#endif
