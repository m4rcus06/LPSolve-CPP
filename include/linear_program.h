#ifndef LINEAR_PROGRAM_H
#define LINEAR_PROGRAM_H

#include <string>
#include <vector>
#include <iomanip>
#include <cassert>
#include <cmath>
#include "matrix.h"

enum class Goal {
    MAX = 0, MIN = 1
};

enum class Relation {
    LESS_EQUAL,             //< Less or equal
    GREATER_EQUAL,          //< Greater or equal
    EQUAL,                  //< Equal
};

enum class varType {
    POSITIVE,               //< >= 0
    NEGATIVE,               //< <= 0
    FREE                    //< any real numbers
};

template <typename T>
struct LinearProgram {
    Goal goal;
    std::vector<T> c;                   //Objective coefficients
    Matrix<T> A;                        //Constraint matrix
    std::vector<T> b;                   //Right hand side values
    std::vector<Relation> relations;    //Relations type for each constraint
    std::vector<varType> variableTypes; //Variables sign correspond to varType
    std::vector<std::string> variableNames;

    int numVariables, numConstraints;
    LinearProgram() {}
    LinearProgram(
            Goal g,
            const std::vector<T>& objective,
            const Matrix<T>& constraints,
            const std::vector<T>& rhs,
            const std::vector<Relation>& rels,
            const std::vector<varType>& types,
            const std::vector<std::string>& labs)
    : goal(g), c(objective), A(constraints), b(rhs), relations(rels), variableTypes(types), variableNames(labs) {
        numVariables = c.size();
        numConstraints = b.size();
        assert((int) A.rowSize() == numConstraints);
        assert((int) A.colSize() == numVariables);
        assert((int) relations.size() == numConstraints);
        assert((int) variableTypes.size() == numVariables);
        assert((int) variableNames.size() == numVariables);
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

    /**
     * @brief add constraints
     * */
    void addConstraints(const std::vector<T>& coeff, T rhs, Relation rel) {
        assert((int) coeff.size() == numVariables);
        A.expandRow(coeff);
        b.push_back(rhs);
        relations.push_back(rel);
        numConstraints = (int) b.size();
    }

   void display() const {
        size_t maxNameLen = 0;
        for (const auto& name: variableNames) maxNameLen = std::max(maxNameLen, name.length());
        int width = (int) maxNameLen;
        std::cout << std::fixed << std::setprecision(4);
        std::cout << "\n======================= Linear Program =========================\n";

        // 1. Print Objective Function
        std::cout << (goal == Goal::MAX ? "Max: " : "Min: ");
        std::cout << "Z = ";
        for (int j = 0; j < numVariables; ++j) {
            T val = c[j];
            if (j > 0) std::cout << (val >= 0 ? " + " : " - ");
            else if (val < 0) std::cout << "-";

            std::cout << std::abs(val) << "*" << std::left << std::setw(maxNameLen) << variableNames[j] << "    ";
        }
        std::cout << "\n\nSubject to:\n";

        // 2. Print Constraints
        int indentSize = 9;
        for (int i = 0; i < numConstraints; ++i) {
            std::cout << std::string(indentSize, ' ');
            for (int j = 0; j < numVariables; ++j) {
                T val = A(i, j);
                if (j > 0) {
                    std::cout << (val >= 0 ? " + " : " - ");
                } else {
                    std::cout << (val < 0 ? "-" : " "); 
                }
                std::cout << std::setw(maxNameLen) << std::abs(val) << "*" << variableNames[j] << "    ";
            }

            // 3. Print Relation & RHS
            switch (relations[i]) {
                case Relation::LESS_EQUAL:    std::cout << "  <=  "; break;
                case Relation::GREATER_EQUAL: std::cout << "  >=  "; break;
                case Relation::EQUAL:         std::cout << "   =  "; break;
            }
            std::cout << std::setw(width) << b[i] << "\n";
        }

        // 4. Print Non-negativity
        std::cout << "\nVariables:\n";
        for (int i = 0; i < numVariables; ++i) {
            std::cout << " " << std::left << std::setw(maxNameLen + 2) << variableNames[i];
            if (variableTypes[i] == varType::POSITIVE) {
                std::cout << ">= 0";
            } else if (variableTypes[i] == varType::NEGATIVE) {
                std::cout << "<= 0";
            } else {
                std::cout << "free";
            }
            std::cout << std::endl;
        }
        std::cout << "================================================================\n" << std::endl;
    } 
};
#endif
