#ifndef LINEAR_PROGRAM_H
#define LINEAR_PROGRAM_H

#include <string>
#include <vector>
#include <iomanip>
#include "matrix.h"

enum class Goal {
    MAX = 0, MIN = 1
};

enum class Relation {
    LESS_EQUAL,        //< Less or equal
    GREATER_EQUAL,        //< Greater or equal
    EQUAL          //< Equal
};

template <typename T>
struct LinearProgram {
    Goal goal;
    std::vector<T> c;                   //Objective coefficients
    Matrix<T> A;                        //Constraint matrix
    std::vector<T> b;                   //Right hand side values
    std::vector<Relation> relations;    //Relations type for each constraint

    int numVariables, numConstraints;
    LinearProgram(
            Goal g,
            const std::vector<T>& objective,
            const Matrix<T>& constraints,
            const std::vector<T>& rhs,
            const std::vector<Relation>& rels)
    : goal(g), c(objective), A(constraints), b(rhs), relations(rels)
    {
        numVariables = c.size();
        numConstraints = b.size();
        assert((int) A.rowSize() == numConstraints);
        assert((int) A.colSize() == numVariables);
        assert((int) relations.size() == numConstraints);
    }

    /**
     * @return r-th row of matrix A as a vector
     * */
    std::vector<T> getRow(int r) const {
        std::vector<T> result(numVariables);
        for (int j = 0; j < numVariables; ++j) {
            result[j] = A(r, j);
        }
        return result;
    }

   void display() const {
        std::cout << std::fixed << std::setprecision(4);
        std::cout << "\n======================= Linear Program =========================\n";

        // 1. Print Objective Function
        std::cout << (goal == Goal::MAX ? "Max: " : "Min: ");
        std::cout << "Z = ";
        for (int j = 0; j < numVariables; ++j) {
            T val = c[j];
            if (j > 0) std::cout << (val >= 0 ? " + " : " - ");
            else if (val < 0) std::cout << "-";
            else std::cout << " ";

            std::cout << std::setw(8) << std::abs(val) << "*x" << j + 1;
        }
        std::cout << "\n\nSubject to:\n";

        // 2. Print Constraints
        for (int i = 0; i < numConstraints; ++i) {
            std::cout << "  ";
            for (int j = 0; j < numVariables; ++j) {
                T val = A(i, j);
                if (j > 0) {
                    std::cout << (val >= 0 ? " + " : " - ");
                } else {
                    std::cout << (val < 0 ? "-" : " "); 
                }
                std::cout << std::setw(8) << std::abs(val) << "*x" << j + 1;
            }

            // 3. Print Relation & RHS
            switch (relations[i]) {
                case Relation::LESS_EQUAL:    std::cout << "  <=  "; break;
                case Relation::GREATER_EQUAL: std::cout << "  >=  "; break;
                case Relation::EQUAL:         std::cout << "   =  "; break;
            }
            std::cout << std::setw(10) << b[i] << "\n";
        }

        // 4. Print Non-negativity
        std::cout << "\n  Variables: ";
        for (int j = 0; j < numVariables; ++j) {
            std::cout << "x" << j + 1 << (j + 1 < numVariables ? ", " : "");    
        }
        std::cout << " >= 0\n";
        std::cout << "================================================================\n" << std::endl;
    } 
};
#endif
