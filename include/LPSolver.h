#ifndef SIMPLEX_SOLVER_H
#define SIMPLEX_SOLVER_H

#include "matrix.h"
#include "linear_program.h"
#include <vector>
#include <string>
#include <limits>

enum class SolverStatus {
    OPTIMAL,        //< Optimal solution found
    INFEASIBLE,     //< No feasible solution exists
    UNBOUNDED,      //< Objective function can be increased infinitely
    RUNNING         //< Solver still running
};

enum class PivotRule {
    DANTZIG, BLAND
};

template <typename T>
using LP = LinearProgram<T>;

template <typename T>
class Solver {
private:
    struct varMap {
        int originalIdx;
        int positiveCol;
        int negativeCol;
        varMap(int i): originalIdx(i), positiveCol(-1), negativeCol(-1) {}
    };
    std::vector<varMap> mappings;// < Variables mapping

    LP<T> originalLP;       //< input LP
    LP<T> lp;               //normalized
    Matrix<T> tableau;      //LP Table
    std::vector <int> basis;     //store basis indices
    int numConstraints;     //number of constrains
    int numTotalVars;       //toal variables count
    const T EPS = static_cast<T>(1e-9); //Precision
    const T INF = std::numeric_limits<T>::max;


    std::vector<T> vecMul(const std::vector<T>& v, T k) {
        std::vector<T> result(v.size());
        for (int i = 0; i < (int) v.size(); ++i) {
            result[i] = k * v[i];
        }
        return result;
    }

    LP<T> normalize(const LP<T>& inputLP) {
        std::vector<T> newC;
        std::vector<std::vector<T>> rows;
        std::vector<T> newB;
        std::vector<Relation> newRelations;
        std::vector<varType> newVariableTypes;
        std::vector<std::string> newVariableNames;

        mappings.clear();
        for (int i = 0; i < inputLP.numVariables; ++i) {
            varMap map(i);
            std::string baseName = "x" + std::to_string(i + 1);
            if (inputLP.variableTypes[i] == varType::POSITIVE) {
                //positive so we keep it
                map.positiveCol = (int) newC.size();
                newC.push_back(inputLP.c[i]);
                newVariableTypes.push_back(varType::POSITIVE);
                newVariableNames.push_back(baseName);
            } else if (inputLP.variableTypes[i] == varType::NEGATIVE) {
                //negative so we flip it
                map.negativeCol = (int) newC.size();
                newC.push_back(-inputLP.c[i]);
                newVariableTypes.push_back(varType::POSITIVE);
                newVariableNames.push_back(baseName + "'");
            } else {
                //free vars, represent it as the form x - y where x and y is non-negative number
                map.positiveCol = (int) newC.size();
                newC.push_back(inputLP.c[i]);
                newVariableTypes.push_back(varType::POSITIVE);
                newVariableNames.push_back(baseName + "'");

                map.negativeCol = (int) newC.size();
                newC.push_back(-inputLP.c[i]);
                newVariableTypes.push_back(varType::POSITIVE);
                newVariableNames.push_back(baseName + "''");
            }

            mappings.push_back(map);
        }

        if (inputLP.goal == Goal::MAX) {
            for (int i = 0; i < (int) newC.size(); ++i) {
                newC[i] = -newC[i];
            }
        }

        for (int i = 0; i < (int) inputLP.numConstraints; ++i) {
            std::vector<T> row(newC.size(), static_cast<T>(0));
            for (int j = 0; j < (int) inputLP.numVariables; ++j) {
                T a = inputLP.A(i, j);
                if (mappings[j].positiveCol != -1) {
                    row[mappings[j].positiveCol] = a;
                }
                if (mappings[j].negativeCol != -1) {
                    row[mappings[j].negativeCol] = -a;
                }
            }

            if (inputLP.relations[i] == Relation::LESS_EQUAL) {
                rows.push_back(row);
                newB.push_back(inputLP.b[i]);
                newRelations.push_back(Relation::LESS_EQUAL);
            } else if (inputLP.relations[i] == Relation::GREATER_EQUAL) {
                rows.push_back(vecMul(row, static_cast<T>(-1)));
                newB.push_back(-inputLP.b[i]);
                newRelations.push_back(Relation::LESS_EQUAL);
            } else {
                rows.push_back(row);
                newB.push_back(inputLP.b[i]);
                newRelations.push_back(Relation::LESS_EQUAL);

                rows.push_back(vecMul(row, static_cast<T>(-1)));
                newB.push_back(-inputLP.b[i]);
                newRelations.push_back(Relation::LESS_EQUAL);
            }
        }

        Matrix<T> newA = Matrix<T>::toMatrix(rows);

        return LP<T>(Goal::MIN, newC, newA, newB, newRelations, newVariableTypes, newVariableNames);
    }

    //currently support Dantzig's rule and Bland's rule
    int findPivotCol(PivotRule rule) {
        int targetRow = numConstraints;
        if (rule == PivotRule::DANTZIG) {
            int bestCol = -1;
            T minCoeff = -EPS;
            for (int c = 0; c < numTotalVars; ++c) {
                if (tableau(targetRow, c) < minCoeff) {
                    minCoeff = tableau(targetRow, c); 
                    bestCol = c;
                }
            }
            return bestCol;
        }

        //BLAND
        for (const auto& m: mappings) {
            if (m.positiveCol != -1 && tableau(targetRow, m.positiveCol) < -EPS) return m.positiveCol;
            if (m.negativeCol != -1 && tableau(targetRow, m.negativeCol) < -EPS) return m.negativeCol;
        }

        int slackStart = lp.numVariables;
        for (int i = slackStart; i < numTotalVars; ++i) {
            if (tableau(targetRow, i) < -EPS) return i;
        }
        //not found
        return -1;
    }

    int findPivotRow(int pCol) {
        int bestRow = -1;
        T minRatio = -1;
        for (int r = 0; r < numConstraints; ++r) {
            T val = tableau(i, pCol);
            if (val > EPS) {
            }
        }
    }

    void pivot(int r, int c) {
        T pVal = tableau(r, c);
        tableau.multiplyRow(r, static_cast<T>(1) / pVal);
    }
public:
    Solver(const LP<T>& inputLP): originalLP(inputLP) {
        this->lp = this->normalize(originalLP);
        numConstraints = lp.numConstraints;
        numTotalVars = lp.numConstraints + lp.numVariables;
        tableau = Matrix<T>(numConstraints + 1, numTotalVars + 1);
        basis.resize(numConstraints);
    }

    void display() {
        this->lp.display();
    }
};
#endif
