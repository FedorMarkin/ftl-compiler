#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

enum type {S, N, F, A, O, E, C, R};

/*
* S - separator ({[,.;]})
* N - numeric constant (int)
* F - fraction (floating point)
* A - name
* O - operator
* E - end state
* C - string constant
* R - reserved name
*/

fstream fin;
pair <type, string> lex;
string s;
stringstream ss;

void g_lex() {
    if (!getline(fin, s) || s == "") {
        lex.first = E;
        lex.second = "#";
        return;
    };
    //cout << s << endl;
    lex.first = (type)(s[0]-'0');
    lex.second = s.substr(2);
    //cout << lex.second << endl;
    cout << lex.first << " " << lex.second << endl;
}


// ***
/*
    op_2 = + | -
    op_3 = * | /
    expr_1 = expr_2 T2
    T2 = op_2 expr_2 | eps
    expr_2 = expr_3 T3
    T3 = op_3 expr_3
    expr_3 = num | (expr_1)
*/
// ***

// ******************************
// *** SYNTAX FUNCTIONS *********
// ******************************

void expr_0();
void expr_1();
void expr_2();
void expr_3();
void expr_4();
void expr_5();
void expr_6();
void expr_7();
void expr_8();

void operators();
void operator_();
void struct_();
void assign_op();
void func();
void for_loop();
void program();
void import();

/*void op_2();
void op_3();*/

void expr_0() {
    expr_1();
    if (lex.second == "=") {g_lex(); expr_0();}
}

void expr_1() {
    expr_2();
    //g_lex();
    if (lex.second == "&&" || lex.second == "||") {g_lex(); expr_1();}
}

void expr_2() {
    expr_3();
    //g_lex();
    if (lex.second == "==" || lex.second == "!=") {g_lex(); expr_2();}
}

void expr_3() {
    expr_4();
    if (lex.second == ">" || lex.second == "<" || lex.second == ">=" || lex.second == "<=") {g_lex(); expr_3();}
}

void expr_4() {
    expr_5();
    if (lex.second == "+" || lex.second == "-") {g_lex(); expr_4();}
}

void expr_5() {
    expr_6();
    if (lex.second == "*" || lex.second == "/" || lex.second == "%") {g_lex(); expr_5();}
}

void expr_6() {
    expr_7();
    if (lex.second == "**") {g_lex(); expr_6();}
}

void expr_7() {
    expr_8();
    if (lex.second == "|" || lex.second == "&" || lex.second == "~" || lex.second == "^") {g_lex(); expr_7();}
}

void expr_8() {
    if (lex.first == N || lex.first == A) {
        type t = lex.first;
        cout << (t == A) << endl;
        g_lex();
        if (lex.second == "++" || lex.second == "--") g_lex();
        else if (t == A) { /** function call **/
            if (lex.second == "(") {
                cout << "func_call" << endl;
                g_lex();
                if (lex.second != ")") {
                    if (lex.first == C) g_lex();
                    else expr_0();
                }
                while (lex.second == ",") {
                    g_lex();
                    if (lex.first == C) g_lex();
                    else expr_0();
                }
                if (lex.second != ")") throw -1;
                g_lex();
            }
        }
        return;
    }
    else if (lex.second == "(") {
        g_lex();
        expr_0();
        cout << "here" << endl;
        //cout << lex.first << " " << lex.second << endl;
        if (lex.second != ")") {
            cout << lex.first << " " << lex.second << endl;
            cout << "bracket balance error" << endl;
            throw -1;
        }
        g_lex();
        return;
    }
    else {
        cout << "error " << lex.second << endl;
        throw -1;
    }
}

//int n = 1;
void operators() {
    //cout << n++ << endl;
    operator_();
    cout << "next: " << lex.second << endl;
    if (lex.second == ";") {
        g_lex();
        if (lex.second == "#") return;
        operators();
    }
}

void struct_() {
   // cout << "struct*" << endl;
    if (lex.first == A) {
        g_lex();
        if (lex.second == "{") {
            g_lex();
            if (lex.second == "int" || lex.second == "double" || lex.second == "string") { g_lex(); assign_op();}
            else throw -1;
            while (lex.second == ",") {
                g_lex();
                if (lex.second == "int" || lex.second == "double" || lex.second == "string") { g_lex(); assign_op(); }
                else throw -1;
            }
            if (lex.second != "}") throw -1;
            g_lex();
        } else throw -1;
    } else throw -1;
}

void for_loop() {
    if (lex.second == "(") {
        g_lex();
        if (lex.second == "int" || lex.second == "double" || lex.second == "string") {
            g_lex();
            assign_op();
            if (lex.second != ";") throw -1;
            g_lex();
            cout << '*' << endl;
            expr_0();
            if (lex.second != ";") throw -1;
            g_lex();
            expr_0();
        } else {
            g_lex();
            expr_0();
        }
        if (lex.second != ")") throw -1;
        g_lex();
        if (lex.second == "{") {
            g_lex();
            operators();
            if (lex.second != "}") throw -1;
            g_lex();
        } else operator_();
    } else throw -1;
}

void assign_op() {
    if (lex.first == A) {
        g_lex();
        if (lex.second != "=") return;
        g_lex();
        if (lex.first == N || lex.first == A || lex.second == "(") expr_0();
        else if (lex.first == C) g_lex();
        else if (lex.second == "[") {
            g_lex();
            if (lex.first == N || lex.first == A || lex.second == "(") expr_0();
            else if (lex.first == C) g_lex();
            while (lex.second == ",") {
                g_lex();
                if (lex.first == N || lex.first == A || lex.second == "(") expr_0();
                else if (lex.first == C) g_lex();
            }
            if (lex.second != "]") throw -1;
            g_lex();
        }
    } else throw -1;
}

void func() {
    cout << lex.second << endl;
    if (lex.first == A) {
        g_lex();
        if (lex.second == "(") {
            g_lex();
            if (lex.second == "int" || lex.second == "double" || lex.second == "string") {
                g_lex();
                if (lex.first != A) throw -1;
                g_lex();
            }
            while (lex.second == ",") {
                g_lex();
                if (lex.second == "int" || lex.second == "double" || lex.second == "string") {
                    g_lex();
                    if (lex.first != A) throw -1;
                    g_lex();
                }
                else throw -1;
            }
            if (lex.second != ")") throw -1;
            g_lex();
            if (lex.second == "{") {
                g_lex();
                operators();
                if(lex.second != "}") throw -1;
                g_lex();
                cout << "here " << lex.second << endl;
            } else throw -1;
        } else throw -1;
    } else throw -1;
}

void operator_() {
    if (lex.first == N || lex.first == A || lex.first == F || lex.second == "(") expr_0();
    else if (lex.second == "int" || lex.second == "double" || lex.second == "string") { g_lex(); assign_op(); }
    else if (lex.second == "struct") { g_lex(); struct_(); }
    else if (lex.second == "func") { g_lex(); func(); }
    else if (lex.second == "for") { g_lex(); for_loop(); }
}

void import() {
    if (lex.second == "(") {
        g_lex();
        if (lex.first == C) g_lex();
        while (lex.second == ",") {
            if (lex.first == C) g_lex();
        }
        if (lex.second != ")") throw -1;
        g_lex();
    } else throw -1;
}

void program() {
    if (lex.second == "import") {g_lex(); import();}
    operators();
}
