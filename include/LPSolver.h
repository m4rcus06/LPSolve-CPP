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
    const T INF = std::numeric_limits<T>::max();


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

    void initTableau() {
        for (int i = 0; i < numConstraints; ++i) {
            //main variables
            for (int j = 0; j < lp.numVariables; ++j) {
                tableau(i, j) = lp.A(i, j);
            }

            //slack variables
            tableau(i, lp.numVariables + i) = static_cast<T>(1);
            tableau(i, numTotalVars) = lp.b[i];
            basis[i] = lp.numVariables + i;

            //Objective func coeff
            for (int j = 0; j < lp.numVariables; ++j) {
                tableau(numConstraints, j) = lp.c[j];
            }

            tableau(numConstraints, numTotalVars) = static_cast<T>(0);
        }
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
        int rhsCol = numTotalVars;
        for (int r = 0; r < numConstraints; ++r) {
            T val = tableau(r, pCol);
            if (val > EPS) {
                T ratio = tableau(r, rhsCol) / val;
                if (bestRow == -1 || minRatio - EPS > ratio) {
                    bestRow = r;
                    minRatio = ratio;
                } else if (std::abs(minRatio - ratio) < EPS) {
                    if (bestRow != -1 && basis[r] < basis[bestRow]) {
                        bestRow = r;
                    }
                }
            }
        }
        return bestRow;
    }

    void pivot(int r, int c) {
        T pVal = tableau(r, c);
        assert(std::abs(pVal) > EPS);

        tableau.multiplyRow(r, static_cast<T>(1) / pVal);
        for (int i = 0; i <= numConstraints; ++i) {
            if (i != r) {
                T factor = -tableau(i, c);
                if (std::abs(factor) > EPS) {
                    tableau.addRow(r, i, factor);
                }
            }
        }
        basis[r] = c;
    }
public:
    Solver(const LP<T>& inputLP): originalLP(inputLP) {
        this->lp = this->normalize(originalLP);
        numConstraints = lp.numConstraints;
        numTotalVars = lp.numConstraints + lp.numVariables;
        tableau = Matrix<T>(numConstraints + 1, numTotalVars + 1);
        basis.resize(numConstraints);
    }

    /**
     * @brief Dictionary form display, 3-digits precision
     */
    void displayDictionary() const {
        std::cout << "\n==================== CURRENT DICTIONARY =======================\n";
        std::cout << std::fixed << std::setprecision(3);

        // varName helper
        auto getVarName = [&](int j) -> std::string {
            if (j == lp.numVariables + numConstraints) return "x0";
            if (j < lp.numVariables) return lp.variableNames[j];
            return "w" + std::to_string(j - lp.numVariables + 1);
        };

        // Mark basis variables, free variables
        std::vector<bool> isBasis(numTotalVars, false);
        for (int r = 0; r < numConstraints; ++r) {
            isBasis[basis[r]] = true;
        }

        std::vector<int> nonBasicVars;
        for (int j = 0; j < numTotalVars; ++j) {
            if (!isBasis[j]) nonBasicVars.push_back(j);
        }

        // Column width calculating
        std::vector<int> colWidths(numTotalVars, 0);
        for (int j : nonBasicVars) {
            size_t maxW = 0;
            std::string name = getVarName(j);
            
            // Z
            T zCoeff = tableau(numConstraints, j);
            if (std::abs(zCoeff) > EPS) {
                std::stringstream ss;
                ss << " + " << std::fixed << std::setprecision(3) << std::abs(zCoeff) << "*" << name;
                maxW = std::max(maxW, ss.str().length());
            }
            // constraints
            for (int r = 0; r < numConstraints; ++r) {
                T cCoeff = -tableau(r, j);
                if (std::abs(cCoeff) > EPS) {
                    std::stringstream ss;
                    ss << " + " << std::fixed << std::setprecision(3) << std::abs(cCoeff) << "*" << name;
                    maxW = std::max(maxW, ss.str().length());
                }
            }
            colWidths[j] = static_cast<int>(maxW);
        }

        // Print Z
        T zConst = -tableau(numConstraints, numTotalVars);
        if (std::abs(zConst) < EPS) zConst = static_cast<T>(0);

        bool isPhase1 = (this->numTotalVars == this->lp.numVariables + this->numConstraints + 1);
        std::stringstream ssZ;
        ssZ << (isPhase1 ? "(Phase 1) Z" : "Z");

        std::cout << "  " << std::left << std::setw(11) << ssZ.str() << " = " 
                  << std::right << std::setw(8) << zConst;

        for (int j : nonBasicVars) {
            T coeff = tableau(numConstraints, j);
            if (std::abs(coeff) > EPS) {
                std::stringstream ssTerm;
                ssTerm << (coeff > 0 ? " + " : " - ") << std::fixed << std::setprecision(3) << std::abs(coeff) << "*" << getVarName(j);
                std::cout << std::left << std::setw(colWidths[j] + 2) << ssTerm.str();
            } else {
                if (colWidths[j] > 0) std::cout << std::string(colWidths[j] + 2, ' ');
            }
        }
        std::cout << "\n\nSubject to:\n";

        // Print constraints
        for (int i = 0; i < numConstraints; ++i) {
            int basisVarIdx = basis[i];
            T rConst = tableau(i, numTotalVars);
            if (std::abs(rConst) < EPS) rConst = static_cast<T>(0);

            std::cout << "  " << std::left << std::setw(11) << getVarName(basisVarIdx) << " = " 
                  << std::right << std::setw(8) << rConst;

            for (int j : nonBasicVars) {
                T coeff = -tableau(i, j);
                if (std::abs(coeff) > EPS) {
                    std::stringstream ssTerm;
                    ssTerm << (coeff > 0 ? " + " : " - ") << std::fixed << std::setprecision(3) << std::abs(coeff) << "*" << getVarName(j);
                    std::cout << std::left << std::setw(colWidths[j] + 2) << ssTerm.str();
                } else {
                    if (colWidths[j] > 0) std::cout << std::string(colWidths[j] + 2, ' ');
                }
            }
            std::cout << "\n";
        }
        std::cout << "===============================================================\n";
    }

    SolverStatus runSimplex(PivotRule rule) {
        while (true) {
            displayDictionary();
            int pCol = findPivotCol(rule);
            if (pCol == -1) {
                return SolverStatus::OPTIMAL;
            }

            int pRow = findPivotRow(pCol);
            if (pRow == -1) {
                return SolverStatus::UNBOUNDED;
            }
            pivot(pRow, pCol);
        }
    }
    
    SolverStatus solve(PivotRule rule) {
        bool Phase1 = false;
        for (int i = 0; i < this->numConstraints; ++i) {
            if (this->lp.b[i] < -EPS) {
                Phase1 = true;
                break;
            }
        }
        
        if (Phase1 == false) {
            this->initTableau();
            return runSimplex(rule);
        }

        /*===================== PHASE 1 =============================*/
        int p1NumTotalVars = this->numTotalVars + 1;
        int x0Col = this->numTotalVars;
        int p1RhsCol = p1NumTotalVars;

        Matrix<T> p1Tableau(this->numConstraints + 1, p1NumTotalVars + 1);
        for (int i = 0; i < this->numConstraints; ++i) {
            for (int j = 0; j < this->lp.numVariables; ++j) {
                p1Tableau(i, j) = this->lp.A(i, j);
            }
            p1Tableau(i, this->lp.numVariables + i) = static_cast<T>(1);
            p1Tableau(i, x0Col) = static_cast<T>(-1);
            p1Tableau(i, p1RhsCol) = this->lp.b[i];
            basis[i] = this->lp.numVariables + i;
        }

        for (int j = 0; j <= p1NumTotalVars; ++j) {
            p1Tableau(this->numConstraints, j) = static_cast<T>(0);
        }
        p1Tableau(this->numConstraints, x0Col) = static_cast<T>(1);

        this->tableau = p1Tableau;
        int orgNumTotalVars = this->numTotalVars;
        this->numTotalVars = p1NumTotalVars;

        int minRow = 0;
        T minB = tableau(0, p1RhsCol);
        for (int i = 1; i < numConstraints; ++i) {
            if (this->tableau(i, p1RhsCol) + EPS < minB) {
                minB = tableau(i, p1RhsCol);
                minRow = i;
            }
        }

        pivot(minRow, x0Col);
        runSimplex(rule);

        T x0RealVal = static_cast<T>(0);
        for (int i = 0; i < numConstraints; ++i) {
            if (basis[i] == x0Col) {
                x0RealVal = tableau(i, p1RhsCol);
                break;
            }
        }

        if (std::abs(x0RealVal) > EPS) {
            return SolverStatus::INFEASIBLE;
        }

        //remove any row that is a linear combination of other
        std::vector<bool> isRedundant(numConstraints, false);
        int redundantCount = 0;

        for (int i = 0; i < numConstraints; ++i) {
            if (basis[i] == x0Col) {
                int repCol = -1;
                for (int j = 0; j < x0Col; ++j) {
                    if (std::abs(tableau(i, j)) > EPS) {
                        repCol = j;
                        break;
                    }
                }
                
                if (repCol != -1) {
                    pivot(i, repCol);
                } else {
                    isRedundant[i] = true;
                    redundantCount++;
                }
            }
        }

        /*========================== PHASE 2 ========================================*/
        std::cout << "\n===================================================";
        std::cout << "\n                    PHASE 2                        ";
        std::cout << "\n===================================================\n";

        int p2NumConstraints = this->numConstraints - redundantCount;
        this->numTotalVars = orgNumTotalVars; 

        Matrix<T> p2Tableau(p2NumConstraints + 1, this->numTotalVars + 1);
        std::vector<int> p2Basis;

        int p2Row = 0;
        for (int i = 0; i < this->numConstraints; ++i) {
            if (isRedundant[i]) continue;

            for (int j = 0; j < this->numTotalVars; ++j) {
                p2Tableau(p2Row, j) = tableau(i, j);
            }
            p2Tableau(p2Row, this->numTotalVars) = tableau(i, p1RhsCol);
            p2Basis.push_back(basis[i]);
            p2Row++;
        }

        for (int j = 0; j < this->lp.numVariables; ++j) {
            p2Tableau(p2NumConstraints, j) = this->lp.c[j];
        }
        for (int j = this->lp.numVariables; j <= this->numTotalVars; ++j) {
            p2Tableau(p2NumConstraints, j) = static_cast<T>(0);
        }

        this->numConstraints = p2NumConstraints;
        this->tableau = p2Tableau;
        this->basis = p2Basis;

        for (int i = 0; i < numConstraints; ++i) {
            int bVar = basis[i];
            T coeff = tableau(numConstraints, bVar);
            if (std::abs(coeff) > EPS) {
                tableau.addRow(i, numConstraints, -coeff); 
            }
        }

        return runSimplex(rule);
    }

    /**
     * @brief Print solution 
     * @param status: LP status from solve
     */
    void printSolution(SolverStatus status) const {
        std::cout << "\n==================== KẾT LUẬN NGHIỆM =======================\n";
        
        if (status == SolverStatus::INFEASIBLE) {
            std::cout << "Bài toán VÔ NGHIỆM, miền chấp nhận được là miền rỗng\n";
            std::cout << "============================================================\n";
            return;
        }
        
        if (status == SolverStatus::UNBOUNDED) {
            std::cout << "Bài toán KHÔNG GIỚI NỘI\n";
            if (this->originalLP.goal == Goal::MAX) {
                std::cout << "Giá trị tối ưu: inf" << '\n';
            } else {
                std::cout << "Giá trị tối ưu: -inf" << '\n';
            }
            std::cout << "============================================================\n";
            return;
        }
        
        if (status == SolverStatus::RUNNING) {
            std::cout << "Thuật toán vẫn đang trong trạng thái chạy...\n";
            std::cout << "============================================================\n";
            return;
        }

        std::cout << "Bài toán có NGHIỆM TỐI ƯU:\n\n";
        std::cout << std::fixed << std::setprecision(3);

        auto getColValue = [&](int col) -> T {
            if (col == -1) return static_cast<T>(0);
            for (int r = 0; r < numConstraints; ++r) {
                if (basis[r] == col) {
                    T val = tableau(r, numTotalVars);
                    return (std::abs(val) < EPS) ? static_cast<T>(0) : val;
                }
            }
            return static_cast<T>(0); // Nếu là biến tự do thì giá trị bằng 0
        };

        std::vector<T> xOriginal(originalLP.numVariables, static_cast<T>(0));
        
        for (int i = 0; i < originalLP.numVariables; ++i) {
            T posVal = (mappings[i].positiveCol != -1) ? getColValue(mappings[i].positiveCol) : static_cast<T>(0);
            T negVal = (mappings[i].negativeCol != -1) ? getColValue(mappings[i].negativeCol) : static_cast<T>(0);

            if (originalLP.variableTypes[i] == varType::POSITIVE) {
                xOriginal[i] = posVal;
            } else if (originalLP.variableTypes[i] == varType::NEGATIVE) {
                xOriginal[i] = -negVal; // if negative, we flip sign
            } else { 
                xOriginal[i] = posVal - negVal; // Biến tự do: x = x' - x''
            }
            
            std::cout << "  * " << std::left << std::setw(8) << originalLP.variableNames[i] << " = " << xOriginal[i] << "\n";
        }

        T zOptimal = static_cast<T>(0);
        for (int i = 0; i < originalLP.numVariables; ++i) {
            zOptimal += originalLP.c[i] * xOriginal[i];
        }

        std::cout << "\n--> Giá trị tối ưu hàm mục tiêu: " 
                  << (originalLP.goal == Goal::MAX ? "Max Z" : "Min Z") << " = " << zOptimal << "\n";
        std::cout << "============================================================\n";
    }
};
#endif
