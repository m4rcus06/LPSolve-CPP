/**
 * @file LPInterface.cpp
 * @brief C++ wrapper for Linear Programming solver - interfaces with Python
 */

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <limits>
#include "include/LPSolver.h"
#include "include/linear_program.h"
#include "include/matrix.h"

using namespace std;

const double EPS = 1e-9;

// Parse comma-separated string to vector of doubles
vector<double> parseVector(const string& str) {
    vector<double> result;
    stringstream ss(str);
    string item;
    
    while (getline(ss, item, ',')) {
        size_t start = item.find_first_not_of(" \t");
        size_t end = item.find_last_not_of(" \t");
        if (start != string::npos) {
            item = item.substr(start, end - start + 1);
            result.push_back(stod(item));
        }
    }
    return result;
}

// Parse relations string (0=LE, 1=GE, 2=EQ)
vector<Relation> parseRelations(const string& str) {
    vector<Relation> result;
    stringstream ss(str);
    string item;
    
    while (getline(ss, item, ',')) {
        size_t start = item.find_first_not_of(" \t");
        size_t end = item.find_last_not_of(" \t");
        if (start != string::npos) {
            item = item.substr(start, end - start + 1);
            int val = stoi(item);
            if (val == 0) result.push_back(Relation::LESS_EQUAL);
            else if (val == 1) result.push_back(Relation::GREATER_EQUAL);
            else result.push_back(Relation::EQUAL);
        }
    }
    return result;
}

// Parse variable types string (0=POS, 1=NEG, 2=FREE)
vector<varType> parseVarTypes(const string& str) {
    vector<varType> result;
    stringstream ss(str);
    string item;
    
    while (getline(ss, item, ',')) {
        size_t start = item.find_first_not_of(" \t");
        size_t end = item.find_last_not_of(" \t");
        if (start != string::npos) {
            item = item.substr(start, end - start + 1);
            int val = stoi(item);
            if (val == 0) result.push_back(varType::POSITIVE);
            else if (val == 1) result.push_back(varType::NEGATIVE);
            else result.push_back(varType::FREE);
        }
    }
    return result;
}

// Parse goal string ("MAX" or "MIN")
Goal parseGoal(const string& str) {
    string s = str;
    transform(s.begin(), s.end(), s.begin(), ::toupper);
    return (s == "MAX") ? Goal::MAX : Goal::MIN;
}

// Parse pivot rule ("DANTZIG" or "BLAND")
PivotRule parsePivotRule(const string& str) {
    string s = str;
    transform(s.begin(), s.end(), s.begin(), ::toupper);
    return (s == "BLAND") ? PivotRule::BLAND : PivotRule::DANTZIG;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: ./LPInterface <command> [args...]" << endl;
        return 1;
    }
    
    string command = argv[1];
    
    // === SOLVE COMMAND ===
    if (command == "solve") {
        if (argc < 10) {
            cerr << "Error: Not enough arguments for solve command" << endl;
            return 1;
        }
        
        try {
            Goal goal = parseGoal(argv[2]);
            vector<double> c = parseVector(argv[3]);
            
            vector<vector<double>> A_rows;
            string matrixStr = argv[4];
            stringstream ss(matrixStr);
            string rowStr;
            while (getline(ss, rowStr, '|')) {
                A_rows.push_back(parseVector(rowStr));
            }
            Matrix<double> A = Matrix<double>::toMatrix(A_rows);
            
            vector<double> b = parseVector(argv[5]);
            vector<Relation> relations = parseRelations(argv[6]);
            vector<varType> varTypes = parseVarTypes(argv[7]);
            PivotRule pivotRule = parsePivotRule(argv[8]);
            double objConstant = stod(argv[9]);
            
            vector<string> varNames;
            for (int i = 0; i < (int)c.size(); ++i) {
                varNames.push_back("x" + to_string(i + 1));
            }
            
            LinearProgram<double> lp(goal, c, A, b, relations, varTypes, varNames, objConstant);
            Solver<double> solver(lp);
            
            // Display problem
            lp.display();
            
            // Solve and display all dictionaries
            cout << "\n=== SOLVING PROCESS ===" << endl;
            SolverStatus status = solver.solve(pivotRule);
            
            // Print solution
            solver.printSolution(status);
            
            // Output path for GUI
            solver.printGUIPath();
            
            cout << "\n=== SOLVER_STATUS ===" << endl;
            switch (status) {
                case SolverStatus::OPTIMAL: cout << "OPTIMAL"; break;
                case SolverStatus::INFEASIBLE: cout << "INFEASIBLE"; break;
                case SolverStatus::UNBOUNDED: cout << "UNBOUNDED"; break;
                case SolverStatus::RUNNING: cout << "RUNNING"; break;
            }
            cout << endl;
            
        } catch (const exception& e) {
            cerr << "Error: " << e.what() << endl;
            return 1;
        }
    }
    // === DICTIONARY COMMAND ===
    else if (command == "dictionary") {
        if (argc < 8) {
            cerr << "Error: Not enough arguments for dictionary command" << endl;
            return 1;
        }
        
        try {
            Goal goal = parseGoal(argv[2]);
            vector<double> c = parseVector(argv[3]);
            
            vector<vector<double>> A_rows;
            string matrixStr = argv[4];
            stringstream ss(matrixStr);
            string rowStr;
            while (getline(ss, rowStr, '|')) {
                A_rows.push_back(parseVector(rowStr));
            }
            Matrix<double> A = Matrix<double>::toMatrix(A_rows);
            
            vector<double> b = parseVector(argv[5]);
            vector<Relation> relations = parseRelations(argv[6]);
            vector<varType> varTypes = parseVarTypes(argv[7]);
            
            vector<string> varNames;
            for (int i = 0; i < (int)c.size(); ++i) {
                varNames.push_back("x" + to_string(i + 1));
            }
            
            LinearProgram<double> lp(goal, c, A, b, relations, varTypes, varNames);
            
            // Display problem first
            lp.display();
            
            // Display all dictionaries by running solve
            cout << "\n=== DICTIONARY FORM ===" << endl;
            Solver<double> solver(lp);
            SolverStatus status = solver.solve(PivotRule::DANTZIG);
            
            solver.printSolution(status);
            
            cout << "\n=== SOLVER_STATUS ===" << endl;
            switch (status) {
                case SolverStatus::OPTIMAL: cout << "OPTIMAL"; break;
                case SolverStatus::INFEASIBLE: cout << "INFEASIBLE"; break;
                case SolverStatus::UNBOUNDED: cout << "UNBOUNDED"; break;
                case SolverStatus::RUNNING: cout << "RUNNING"; break;
            }
            cout << endl;
            
        } catch (const exception& e) {
            cerr << "Error: " << e.what() << endl;
            return 1;
        }
    }
    else {
        cerr << "Unknown command: " << command << endl;
        return 1;
    }
    
    return 0;
}
