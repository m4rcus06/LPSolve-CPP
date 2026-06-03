#ifndef SIMPLEX_SOLVER_H
#define SIMPLEX_SOLVER_H

#include "matrix.h"
#include "linear_program.h"
#include <vector>
#include <string>
#include <limits>
#include <set>

enum class SolverStatus {
    OPTIMAL,        //< Optimal solution found
    INFEASIBLE,     //< No feasible solution exists
    UNBOUNDED,      //< Objective function can be increased infinitely
    CYCLING,        //< Solver is cycling (no progress)
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
    std::vector<int> basis;     //store basis indices
    int numConstraints;     //number of constrains
    int numTotalVars;       //toal variables count
    std::vector<std::vector<T>> varCoords;  //store variable's coordinates
    std::vector<std::vector<T>> pathHistory;  //store path for GUI visualization
    const T EPS = static_cast<T>(1e-9); //Precision
    const T INF = std::numeric_limits<T>::max();
    std::set<std::string> visitedBases;  //for cycling detection


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

        return LP<T>(Goal::MIN, newC, newA, newB, newRelations, newVariableTypes, newVariableNames, inputLP.objectiveConstant);
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

    std::vector<T> getCoords() const {
        auto getColValue = [&](int col) -> T {
            if (col == -1) return static_cast<T>(0);
            for (int r = 0; r < numConstraints; ++r) {
                if (basis[r] == col) {
                    T val = tableau(r, numTotalVars);
                    return (std::abs(val) < EPS) ? static_cast<T>(0) : val;
                }
            }
            return static_cast<T>(0);
        };

        std::vector<T> curX(originalLP.numVariables, static_cast<T>(0));
        for (int i = 0; i < originalLP.numVariables; ++i) {
            T posVal = (mappings[i].positiveCol != -1) ? getColValue(mappings[i].positiveCol) : static_cast<T>(0);
            T negVal = (mappings[i].negativeCol != -1) ? getColValue(mappings[i].negativeCol) : static_cast<T>(0);
            if (originalLP.variableTypes[i] == varType::POSITIVE) {
                curX[i] = posVal;
            } else if (originalLP.variableTypes[i] == varType::NEGATIVE) {
                curX[i] = -negVal;
            } else {
                curX[i] = posVal - negVal;
            }
        }
        return curX;
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
     * @brief Dictionary form display - clean, aligned output
     */
    void displayDictionary() const {
        int screenWidth = 100;
        int separatorLen = screenWidth - 4;
        
        std::string sep(separatorLen, '=');
        
        std::cout << "\n" << sep << "\n";

        // Helper: format number - no decimal for integers, 3 decimals for floats
        auto formatNum = [](T val) -> std::string {
            if (std::abs(val) < static_cast<T>(1e-9)) val = static_cast<T>(0);
            if (std::abs(val - std::round(val)) < static_cast<T>(1e-6)) {
                return std::to_string(static_cast<long long>(std::round(val)));
            }
            std::stringstream ss;
            ss << std::fixed << std::setprecision(3) << val;
            return ss.str();
        };

        // varName helper
        auto getVarName = [&](int j) -> std::string {
            if (j == lp.numVariables + numConstraints) return "x0";
            if (j < lp.numVariables) return lp.variableNames[j];
            return "w" + std::to_string(j - lp.numVariables + 1);
        };

        // Mark basis variables
        std::vector<bool> isBasis(numTotalVars, false);
        for (int r = 0; r < numConstraints; ++r) {
            isBasis[basis[r]] = true;
        }

        std::vector<int> nonBasicVars;
        for (int j = 0; j < numTotalVars; ++j) {
            if (!isBasis[j]) nonBasicVars.push_back(j);
        }

        // Calculate max width for variable column
        int maxVarNameLen = 4;
        for (int j : nonBasicVars) {
            maxVarNameLen = std::max(maxVarNameLen, (int)getVarName(j).length());
        }
        for (int i = 0; i < numConstraints; ++i) {
            maxVarNameLen = std::max(maxVarNameLen, (int)getVarName(basis[i]).length());
        }
        
        int constColWidth = 12;
        int termColWidth = 18;
        
        // Calculate "=" column position for alignment
        int eqPos = 2 + maxVarNameLen + 2;
        std::string eqSpacing(constColWidth + 1, ' ');
        
        // Print Z row
        // Note: For MAX problems, the tableau stores negated coefficients but NOT the constant term.
        // So Z_display = tableau_constant + originalObjectiveConstant (no negation needed)
        T zConst = tableau(numConstraints, numTotalVars) + originalLP.objectiveConstant;
        if (std::abs(zConst) < EPS) zConst = static_cast<T>(0);

        bool isPhase1 = (this->numTotalVars == this->lp.numVariables + this->numConstraints + 1);
        std::string zLabel = isPhase1 ? "(Phase 1) Z" : "Z";

        std::cout << "  " << std::left << std::setw(maxVarNameLen + 2) << zLabel;
        std::cout << eqSpacing.substr(0, eqPos - (2 + maxVarNameLen + 2) + 1) << "= ";
        std::cout << std::right << std::setw(constColWidth - 2) << formatNum(zConst);

        for (int j : nonBasicVars) {
            T coeff = tableau(numConstraints, j);
            if (std::abs(coeff) > EPS) {
                std::string term = (coeff > 0 ? " + " : " - ") + formatNum(std::abs(coeff)) + "*" + getVarName(j);
                std::cout << std::left << std::setw(termColWidth) << term;
            } else {
                std::cout << std::string(termColWidth, ' ');
            }
        }
        std::cout << "\n" << sep << "\n\n";

        // Print constraints
        std::cout << "  Subject to:\n\n";
        for (int i = 0; i < numConstraints; ++i) {
            int basisVarIdx = basis[i];
            T rConst = tableau(i, numTotalVars);
            if (std::abs(rConst) < EPS) rConst = static_cast<T>(0);

            std::cout << "  " << std::left << std::setw(maxVarNameLen + 2) << getVarName(basisVarIdx);
            std::cout << "= ";
            std::cout << std::right << std::setw(constColWidth - 2) << formatNum(rConst);

            for (int j : nonBasicVars) {
                T coeff = -tableau(i, j);
                if (std::abs(coeff) > EPS) {
                    std::string term = (coeff > 0 ? " + " : " - ") + formatNum(std::abs(coeff)) + "*" + getVarName(j);
                    std::cout << std::left << std::setw(termColWidth) << term;
                } else {
                    std::cout << std::string(termColWidth, ' ');
                }
            }
            std::cout << "\n";
        }
        
        std::cout << "\n" << sep << "\n\n";
    }

    SolverStatus runSimplex(PivotRule rule) {
        int cnt = 0;
        pathHistory.clear();
        visitedBases.clear();
        while (true) {
            // Cycling detection: encode current basis as string
            std::string basisKey;
            for (int b : basis) {
                basisKey += std::to_string(b) + ",";
            }
            
            if (visitedBases.find(basisKey) != visitedBases.end()) {
                std::cout << "\n  >> Phat hien vong lap, tien hanh giai bang BLAND.\n";
                return SolverStatus::CYCLING;
            }
            visitedBases.insert(basisKey);
            
            pathHistory.push_back(getCoords());
            std::cout << "\n[ Iteration #" << cnt + 1 << " ]\n";
            ++cnt;
            displayDictionary();
            int pCol = findPivotCol(rule);
            if (pCol == -1) {
                std::cout << "\n  >> Optimal solution found!\n";
                return SolverStatus::OPTIMAL;
            }

            int pRow = findPivotRow(pCol);
            if (pRow == -1) {
                std::cout << "\n  >> Problem is unbounded!\n";
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
            SolverStatus status = runSimplex(rule);
            
            // If cycling detected, retry with Bland's rule
            if (status == SolverStatus::CYCLING) {
                std::cout << "\n  >> Retrying with Bland's rule...\n";
                this->initTableau();
                status = runSimplex(PivotRule::BLAND);
            }
            return status;
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
        std::cout << "\n";
        std::cout << "  " << std::string(60, '=') << "\n";
        std::cout << "                         PHASE 2 - Tiep tuc giai\n";
        std::cout << "  " << std::string(60, '=') << "\n\n";

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

        SolverStatus status = runSimplex(rule);
        
        // If cycling detected in Phase 2, retry entire solve with Bland's rule
        if (status == SolverStatus::CYCLING) {
            std::cout << "\n  >> Phat hien vong lap o Phase 2, tien hanh giai lai bang BLAND.\n";
            this->initTableau();
            status = runSimplex(PivotRule::BLAND);
            
            // Handle Phase 1 if needed (re-check)
            for (int i = 0; i < this->numConstraints; ++i) {
                if (this->lp.b[i] < -EPS) {
                    std::cout << "\n  !! Bland's rule also failed Phase 1! Problem may be inherently unstable.\n";
                    break;
                }
            }
        }
        return status;
    }

    std::vector<std::vector<T>> getPath() const {
        return this->varCoords;
    }

    /**
     * @brief Print solution with clean alignment
     * @param status: LP status from solve
     */
    void printSolution(SolverStatus status) const {
        int sepLen = 96;  // Wider
        
        std::cout << "\n" << std::string(sepLen, '=') << "\n";
        std::cout << "                                    KET LUAN NGHIEM\n";
        std::cout << std::string(sepLen, '=') << "\n\n";
        
        if (status == SolverStatus::INFEASIBLE) {
            std::cout << "  [VO NGHIEM]\n";
            std::cout << "  Mien chap nhan duoc la mien rong.\n";
            std::cout << "\n" << std::string(sepLen, '=') << "\n";
            return;
        }
        
        if (status == SolverStatus::UNBOUNDED) {
            std::cout << "  [KHONG GIOI NOI]\n";
            std::cout << "  Gia tri ham muc tieu co the tang vo han.\n";
            if (this->originalLP.goal == Goal::MAX) {
                std::cout << "  Max Z = +infinity\n";
            } else {
                std::cout << "  Min Z = -infinity\n";
            }
            std::cout << "\n" << std::string(sepLen, '=') << "\n";
            return;
        }
        
        if (status == SolverStatus::CYCLING) {
            std::cout << "  [XOAY VONG - CYCLING]\n";
            std::cout << "  Thuat toan bi lap vo han.\n";
            std::cout << "  Goi y: Su dung Bland's Rule.\n";
            std::cout << "\n" << std::string(sepLen, '=') << "\n";
            return;
        }
        
        if (status == SolverStatus::RUNNING) {
            std::cout << "  [DUNG SOM]\n";
            std::cout << "  Thuat toan dat gioi han so lan lap.\n";
            std::cout << "\n" << std::string(sepLen, '=') << "\n";
            return;
        }

        std::cout << "  [NGHIEM TOI UU]\n\n";

        // Helper: format number - no decimal for integers, 3 decimals for floats
        auto formatNum = [](T val) -> std::string {
            if (std::abs(val) < static_cast<T>(1e-9)) val = static_cast<T>(0);
            if (std::abs(val - std::round(val)) < static_cast<T>(1e-6)) {
                return std::to_string(static_cast<long long>(std::round(val)));
            }
            std::stringstream ss;
            ss << std::fixed << std::setprecision(3) << val;
            return ss.str();
        };

        int maxNameLen = 8;
        for (int i = 0; i < originalLP.numVariables; ++i) {
            maxNameLen = std::max(maxNameLen, (int)originalLP.variableNames[i].length());
        }
        
        std::cout << "  Cac gia tri bien:\n\n";
        std::vector<T> xOriginal = getCoords();
        for (int i = 0; i < originalLP.numVariables; ++i) {
            std::cout << "    " << std::left << std::setw(maxNameLen) << originalLP.variableNames[i] 
                      << "  =  " << std::right << std::setw(14) << formatNum(xOriginal[i]) << "\n";
        }

        T zOptimal = static_cast<T>(0);
        for (int i = 0; i < originalLP.numVariables; ++i) {
            zOptimal += originalLP.c[i] * xOriginal[i];
        }
        zOptimal += originalLP.objectiveConstant;

        std::cout << "\n" << std::string(sepLen, '-') << "\n";
        std::string zLabel = (originalLP.goal == Goal::MAX ? "Max Z" : "Min Z");
        std::cout << "  Gia tri toi uu:  " << zLabel << " = " << formatNum(zOptimal) << "\n";
        std::cout << std::string(sepLen, '=') << "\n";
    }
    
    /**
     * Output path for GUI visualization
     * Format: GUI_PATH:x1,y1;x2,y2;...
     */
    void printGUIPath() const {
        std::cout << "\n=== GUI_PATH ===\n";
        const auto& path = pathHistory.empty() ? varCoords : pathHistory;
        for (size_t i = 0; i < path.size(); ++i) {
            for (size_t j = 0; j < path[i].size(); ++j) {
                std::cout << path[i][j];
                if (j < path[i].size() - 1) std::cout << ",";
            }
            if (i < path.size() - 1) std::cout << ";";
        }
        std::cout << "\n";
    }
};
#endif
