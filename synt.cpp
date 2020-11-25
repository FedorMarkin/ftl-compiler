#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <map>

using namespace std;

enum type {S, N, F, A, O, E, C, R, NL};

/*
* S - separator ({[,.;]})
* N - numeric constant (int)
* F - fraction (floating point)
* A - name
* O - operator
* E - end state
* C - string constant
* R - reserved name
* NL - new line
*/

fstream fin;
pair <type, string> lex;
string s;
stringstream ss;
int line_cnt = 1;
string f_name = "";

map<string, int> runtime_fn_list;

string itos(int k) {
    cout << k << "***" << endl;
    stringstream s;
    string str;
    s << k;
    s >> str;
    return str;
}

enum val {Int, Double, String, Bool, Op, Type, Func, Name};

val curr_type;

struct val_type {
    val type;
    bool lvalue; // can be assigned to a value
    bool operatable = true; // by default assuming that we deal with primitive types, but when working with arrays no operations like +-*/ are allowed
    //val_type(val v): type(v), lvalue(false) {};
    val_type(val v, bool is_lv): type(v), lvalue(is_lv) {};
    val_type(val v, bool is_lv, bool op): type(v), lvalue(is_lv), operatable(op) {};
    val_type() {}
};

struct func_prototype {
    val type;
    int params_cnt;
    vector<val> param_types;
    vector<bool> is_arr;
    func_prototype(val tp, int pm): type(tp), params_cnt(pm) {};
    func_prototype(val tp, int pm, vector<val> &prm, vector<bool> &arr): type(tp), params_cnt(pm) {
        for (int i = 0; i < prm.size(); ++i) {
            param_types.push_back(prm[i]);
        }
        for (int i = 0; i < arr.size(); ++i) {
            is_arr.push_back(arr[i]);
        }
    }
};

vector<pair<map<string,func_prototype>, map<string, val_type> > > scopes;
vector<set<string>> struct_tid;
vector<val_type> stk; // for type calculation

void push_op(val_type vt) {
    stk.push_back(vt);
}

enum op_type {Logic, Arythm, Assignment, Bit};

string t = "";

struct unit {
    val tp;
    bool is_adress = false;
    bool is_op = false;
    bool is_name = false;
    bool is_arr = false;
    string value;
    string arr_name;
    int index;
    vector<string> *v;
    unit(val t, bool adr, bool op, bool name, string s): tp(t), is_adress(adr), is_op(op), is_name(name), value(s) {};
    unit(val t, bool adr, bool op, bool name, bool arr, string s): tp(t), is_adress(adr), is_op(op), is_name(name), is_arr(arr), value(s) {};
};

struct op {
    string opr;
    int prior;
    op(string OP, int Pr): opr(OP), prior(Pr) {};
};

vector<unit> plz;
vector<op> op_stk;

void check_op(op_type op) {
    val_type b(stk.back().type, stk.back().lvalue, stk.back().operatable);
    //b = stk.back();
    stk.pop_back();
    val_type a(stk.back().type, stk.back().lvalue, stk.back().operatable);
    //a = stk.back();
    stk.pop_back();
    if (op == Logic) {
        if (a.type != String && a.operatable && b.type != String && b.operatable) {
            stk.push_back(val_type(Int, false));
        } else {
            cout << a.type << " " << b.type << endl;
            cout << "invalid argument to logic operation" << endl;
            throw -2;
        }
    } else if (op == Arythm) {
        cout << "checking operation +" << endl;
        if (a.type != String && b.type != String && a.operatable && b.operatable) {
            if (a.type == Double || b.type == Double) stk.push_back(val_type(Double, false));
            else stk.push_back(val_type(Int, false));
        }
        else {
            cout << a.type << " " << b.type << "; " << a.operatable << " " << b.operatable << endl;
            cout << "invalid argument to arythmetic operation" << endl;
            throw -2;
        }
    } else if (op == Assignment) {
        if (a.lvalue && a.type == b.type) stk.push_back(val_type(a.type, false));
        else {
            cout << "invalid assignment argumnent" << endl;
            throw -2;
        }
    } else if (op == Bit) {
        if (a.type == b.type && a.type == Int) stk.push_back(val_type(a.type, false));
        else {
            cout << "invalid argument to bit operaion" << endl;
            throw -2;
        }
    }
}

void create_tid() { scopes.emplace_back(); struct_tid.emplace_back(); }

void delete_tid() {
    scopes.pop_back();
    struct_tid.pop_back();
}

void push_struct(string name) {
    if (struct_tid.back().find(name) != struct_tid.back().end()) {
        cout << "multiple struct declaration" << endl;
        throw -2;
    }
    struct_tid.back().insert(name);
}

void push_fn(string name, func_prototype fp) {
    if (scopes.size() == 1) scopes.back().first.insert({name, fp});
    else scopes[scopes.size()-2].first.insert({name, fp}); /** may cause bad behaviour and segmentation faults MUST BE FIXED!!! **/
}

void push_id(string name, val_type t) {
    if (scopes.back().second.count(name)) {
        cout << "multiple declaration of " << name << " on line " << line_cnt << endl;
        throw -2;
    }
    scopes.back().second.insert({name, t});
}

bool check_params(string name, vector<func_prototype> pr) {
   // if (pr.size() != scopes.back().first[name].params);
}

func_prototype check_fn(string name) {
    // if (scopes.back().first.count(name) == 0) return false;
    for (int i = scopes.size()-1; i >= 0; --i) {
        if (scopes[i].first.count(name)) return scopes[i].first.at(name);
    }
    if (name == "print" || name == "return") return func_prototype(Func, 1);
    cout << "undeclared function" << endl;
    throw -2;
}

val_type check_id(string name) {
    for (int i = scopes.size()-1; i >= 0; --i) {
        if (scopes[i].second.count(name)) return scopes[i].second.at(name);
    }
    cout << "undeclared variable on line " << line_cnt << endl;
    throw -2;
}

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
    if (lex.first == NL) {
        g_lex();
        line_cnt++;
        return;
    }
    if (lex.second == ";") {
        while (!op_stk.empty()) {
            plz.push_back(unit(Op, false, true, false, op_stk.back().opr));
            op_stk.pop_back();
        }
        if (!plz.empty() && plz.back().value == ";") ;
        else plz.push_back(unit(Op, false, true, false, ";"));
    }

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
void if_op();

/*void op_2();
void op_3();*/


/*int get_prior() {
    int pr = 0;
    if (lex.second == "=") pr=1;
    else if (lex.second == "&&" || lex.second == "||") pr=2;
    else if (lex.second == "==" || lex.second == "!=") pr=3;
    else if (lex.second == ">"||lex.second=="<"||lex.second==">="||lex.second=="<=") pr=4;
    else if (lex.second=="+"||lex.second=="-")pr=5;
    else if (lex.second=="*"||lex.second=="/"||lex.second=="%")pr=6;
    else if (lex.second=="**")pr=7;
    else if (lex.second=="|"||lex.second=="&"||lex.second=="~"||lex.second=="^")pr=8;
    else if (lex.second=="++"||lex.second=="--"||lex.second=="!")pr=9;
    return pr;
}*/
//enum u_type {Int, Double, String, Bool, Op};
void expr_0() {
    expr_1();
    if (lex.second == "=" || lex.second == "*=" || lex.second == "/=" || lex.second == "+=" || lex.second == "-="
        || lex.second == "%=") {
            int pr = 0;
            string t = lex.second;
            while (!op_stk.empty()&&pr<=op_stk.back().prior) {
                cout << "*********564654" << endl;
                if (pr==6&&op_stk.back().prior==pr) break;
                plz.push_back(unit(Op, false, true, false, op_stk.back().opr));
                op_stk.pop_back();
            }
            plz.back().is_adress = true;
            op_stk.push_back(op(t, pr));
            g_lex(); expr_0(); check_op(Assignment);
        }
}

void expr_1() {
    expr_2();
    //g_lex();
    //if (lex.second == "&&" || lex.second == "||") {g_lex(); expr_1(); ñheck_op(Logic);}
    if (lex.second == "&&" || lex.second == "||") {
        int pr = 1;
        string t = lex.second;
       while (!op_stk.empty()&&pr<=op_stk.back().prior) {
                cout << "*********564654" << endl;
            if (pr==6&&op_stk.back().prior==pr) break;
            plz.push_back(unit(Op, false, true, false, op_stk.back().opr));
            op_stk.pop_back();
        }
        op_stk.push_back(op(t, pr));
        g_lex();
        expr_1();
        check_op(Logic);
    }
}

void expr_2() {
    expr_3();
    //g_lex();
    if (lex.second == "==" || lex.second == "!=") {
        string t = lex.second;
        int pr=2;
        while (!op_stk.empty()&&pr<=op_stk.back().prior) {
                cout << "*********564654" << endl;
            if (pr==6&&op_stk.back().prior==pr) break;
            plz.push_back(unit(Op, false, true, false, op_stk.back().opr));
            op_stk.pop_back();
        }
        op_stk.push_back(op(t, pr));
        g_lex(); expr_2();check_op(Logic);
    }
}

void expr_3() {
    expr_4();
    if (lex.second == ">" || lex.second == "<" || lex.second == ">=" || lex.second == "<=") {
        string t = lex.second;
        int pr=3;
        while (!op_stk.empty()&&pr<=op_stk.back().prior) {
                cout << "*********564654" << endl;
            if (pr==6&&op_stk.back().prior==pr) break;
            plz.push_back(unit(Op, false, true, false, op_stk.back().opr));
            op_stk.pop_back();
        }
        op_stk.push_back(op(t, pr));
        g_lex(); expr_3(); check_op(Logic);
    }
}

void expr_4() {

    expr_5();
    if (lex.second == "+" || lex.second == "-") {
        string t = lex.second;
        int pr=4;
        while (!op_stk.empty()&&pr<=op_stk.back().prior) {
                cout << "*********564654" << endl;
            //if (pr==6&&op_stk.back().prior==pr) break;
            cout << plz.back().value << endl;
            cout << "123" << endl;
            cout << op_stk.back().opr << endl;
            plz.push_back(unit(Op, false, true, false, op_stk.back().opr));
            op_stk.pop_back();
        }
        op_stk.push_back(op(t, pr));
        cout << op_stk.back().opr << endl;
        g_lex(); expr_4();check_op(Arythm);
    }
}

void expr_5() {
    expr_6();
    if (lex.second == "*" || lex.second == "/" || lex.second == "%") {
        cout << "multip_3424" << endl;
        int pr=5;
        while (!op_stk.empty()&&pr<=op_stk.back().prior) {
                cout << "*********564654" << endl;
            if (pr==6&&op_stk.back().prior==pr) break;
            plz.push_back(unit(Op, false, true, false, op_stk.back().opr));
            op_stk.pop_back();
        }
        op_stk.push_back(op(lex.second, pr));
        g_lex(); expr_5();check_op(Arythm);
    }
}

void expr_6() {
    expr_7();
    if (lex.second == "**") {
        int pr=6;
        while (!op_stk.empty()&&pr<=op_stk.back().prior) {
                cout << "*********564654" << endl;
            if (pr==6&&op_stk.back().prior==pr) break;
            plz.push_back(unit(Op, false, true, false, op_stk.back().opr));
            op_stk.pop_back();
        }
        op_stk.push_back(op(lex.second, pr));
        g_lex(); expr_6();check_op(Arythm);
    }
}

void expr_7() {
    expr_8();
    if (lex.second == "|" || lex.second == "&" || lex.second == "~" || lex.second == "^") {
        int pr=7;
        while (!op_stk.empty()&&pr<=op_stk.back().prior) {
                cout << "*********564654" << endl;
            //if (pr==6&&op_stk.back().prior==pr) break;
            plz.push_back(unit(Op, false, true, false, op_stk.back().opr));
            cout << plz.back().value << endl;
            op_stk.pop_back();
        }
        op_stk.push_back(op(lex.second, pr));
        g_lex(); expr_7();check_op(Bit);
    }
}
/// TODO:
/// function calls;
void expr_8() {
    bool operatable = true;
    string t_name;
    if (lex.first == N || lex.first == A || lex.first == F) {
        if (lex.first == N) {
            stk.push_back(val_type(Int, false));
            plz.push_back(unit(Int, false, false, false, lex.second));
        }
        else if (lex.first == F) {
            stk.push_back(val_type(Double, false));
            plz.push_back(unit(Double, false, false, false, lex.second));
        }
        else if (lex.first == A) {
            val_type vt = check_id(lex.second);
            stk.push_back(val_type(vt.type, true));
            if (!vt.operatable) {
                cout << "is not operatable" << endl;
                stk.back().operatable = false;
                /*cout << "cannot operate with unoperatable types :(" << endl;
                throw -2;*/
                operatable = false;
            }
            //t_name = lex.second;
            plz.push_back(unit(vt.type, false, false, true, lex.second));
            //cout << "******" << (scopes.back().second[lex.second].type == String) << endl;
        }
        type t = lex.first;
        string func_name = lex.second;
        //cout << t << endl;
        string nm = lex.second;
        cout << (t == A) << endl;
        g_lex();
        if (lex.second == "++" || lex.second == "--") {

            plz.push_back(unit(Op, false, true, false, lex.second+"#")); // postfix
            if (stk.back().type == Int && stk.back().operatable) {
                stk.back().lvalue = false;
                g_lex();
            }
            else throw -2;
        }
        else if (t == A) { /** function call **/ /** TODO: check parameters match with declared previously func **/
            if (lex.second == "(") {
                auto t_func = plz.back();
                plz.pop_back();
                func_prototype fp = check_fn(nm);
                int pt = fp.params_cnt;
                int k = 0;
                cout << "func_call" << endl;
                g_lex();
                if (lex.second != ")") {
                    if (nm == "print" || nm == "return") {
                        cout << "*************************" << endl;
                        if (lex.first == C) {
                            stk.push_back(val_type(String, false));
                            plz.push_back(unit(String, false, false, false, lex.second));
                            g_lex();
                        }
                        else expr_0();
                    }
                    else {if (lex.first == C) {
                        stk.push_back(val_type(String, false));
                        plz.push_back(unit(String, false, false, false, lex.second));
                        g_lex();
                    }
                    else expr_0();
                    if (fp.param_types.empty() && nm != "print" && nm != "return") {
                        cout << "parameters mismatch" << endl;
                        throw -2;
                    }
                    if (fp.param_types[k] != stk.back().type && fp.is_arr[k] != stk.back().operatable && nm != "print" && nm != "return") {
                        cout << fp.param_types[k] << " " << stk.back().type << endl;
                        cout << "parameters mismatch" << endl;
                        throw -2;
                    }
                    ++k;
                    pt--;}
                }
                //cout << "***********************" << endl;
                while (lex.second == ",") {
                    g_lex();
                    if (lex.first == C) {
                        stk.push_back(val_type(String, false));
                        plz.push_back(unit(String, false, false, false, lex.second));
                        g_lex();
                    }
                    else expr_0();
                    if (k >= fp.param_types.size()) {
                        //cout << fp.params_types[k] << " " << stk.back().type << endl;
                        cout << "parameters mismatch" << endl;
                        throw -2;
                    }
                    if (fp.param_types[k] != stk.back().type && nm != "print" && nm != "return") {
                        cout << fp.param_types[k] << " " << stk.back().type << endl;
                        cout << "parameters mismatch" << endl;
                        throw -2;
                    }
                    ++k;
                    pt--;
                }
                if (pt != 0 && nm != "print" && nm != "return") {
                    cout << "invalid parameters passed to function" << endl;
                    throw -2;
                }
                if (lex.second != ")") {
                    cout << "expected ')'" << endl;
                    throw -1;
                }
                while (!op_stk.empty()) {
                    //if (pr==6&&op_stk.back().prior==pr) break;
                    plz.push_back(unit(Op, false, true, false, op_stk.back().opr));
                    op_stk.pop_back();
                }
                plz.push_back(unit(Int, false, false, false, itos(fp.params_cnt)));
                plz.push_back(t_func);
                plz.back().tp = Func;
                plz.push_back(unit(Op, false, true, false, "$N")); /** go to func operator **/
                g_lex();
            } else if (lex.second == "[") {
                auto t_arr = plz.back();
                t_arr.is_adress = true;
                plz.pop_back();
                g_lex();
                if (lex.first != N) {
                    cout << "expected an integer constant" << endl;
                    throw -1;
                }
                //cout << lex.second << " : " << lex.first << endl;
                plz.push_back(unit(Int, false, false, false, lex.second));
                plz.push_back(t_arr);
                plz.push_back(unit(Op, false,true, false, "[]"));
                g_lex();
                if (lex.second != "]") {
                    cout << "expected ']'" << endl;
                    throw -1;
                }
                g_lex();
                stk.back().operatable = true;
                if (operatable) {
                    cout << "cannot use scalar value as array" << endl;
                    throw -2;
                }
            } else if (lex.second == ".") {
                while (lex.second == ".") {
                    g_lex();
                    if (lex.first != A) {
                        cout << "invalid name" << endl;
                        throw -1;
                    }
                    g_lex();
                }
                if (lex.second == "(") {
                    g_lex();
                    if (lex.first == N || lex.first == A || lex.first == F || lex.second == "(" || lex.second == "*" ||
                        lex.second == "+" || lex.second == "-" || lex.second == "!" || lex.second == "&" || lex.second == "++" || lex.second == "--") expr_0();
                    while(lex.second == ",") {
                        g_lex();
                        if (lex.first == N || lex.first == A || lex.first == F || lex.second == "(" || lex.second == "*" ||
                            lex.second == "+" || lex.second == "-" || lex.second == "!" || lex.second == "&" || lex.second == "++" || lex.second == "--") expr_0();
                    }
                    if (lex.second != ")"){
                        cout << "expected ')'" << endl;
                        throw -1;
                    }
                    g_lex();
                }
            }
        }
        return;
    } else if (lex.second == "*" || lex.second == "+" || lex.second == "-" || lex.second == "!" || lex.second == "&") {
        string t = lex.second;
        op_stk.push_back(op(t+"#", 100));
        g_lex();
        expr_0();
        if (stk.back().type != Int) {
            cout << "incompatible types in unary operation ***" << endl;
            throw -2;
        }
        stk.back().lvalue = false;
        //plz.push_back(unit(Op, false, true, false, t));
    } else if (lex.second == "++" || lex.second == "--") {
        string t = lex.second;
        op_stk.push_back(op(t+"#", 100));
        g_lex();
        if (lex.first == A) {
            val_type vt = check_id(lex.second);
            g_lex();
            if (vt.type != Int) {
                cout << "incompatible types in unary operation" << endl;
                throw -2;
            }
            return;
        } else throw -1;
    }
    else if (lex.second == "(") {
        g_lex();
        op_stk.push_back(op("(", -1));
        expr_0();
        cout << "here" << endl;

        //cout << lex.first << " " << lex.second << endl;
        if (lex.second != ")") {
            cout << "expected ')', found '" << lex.second << "'" << endl;
            //cout << lex.first << " " << lex.second << endl;
            cout << "bracket balance error" << endl;
            throw -1;
        }
        cout << "puren____________________" << endl;
        while (!op_stk.empty() && op_stk.back().opr!="(") {
        cout << "puren____________________" << endl;
            plz.push_back(unit(Op, false, true, false, op_stk.back().opr));
            cout << op_stk.back().opr << endl;
            op_stk.pop_back();
        }
        if (!op_stk.empty()) op_stk.pop_back();
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
    if (lex.second == "}") return;
    operator_();
    cout << "next: " << lex.second << endl;
    if (lex.second == ";") {
        g_lex();
        if (lex.second == "#" || lex.second == "}") return;
        operators();
    }
}

void struct_() {
   // cout << "struct*" << endl;
    if (lex.first == A) {
        push_struct(lex.second);
        string f = lex.second+".";
        g_lex();
        if (lex.second == "{") {
            g_lex();
            if (lex.second == "int" || lex.second == "double" || lex.second == "string") {
                if (lex.second == "int") curr_type = Int;
                else if (lex.second == "double") curr_type == Double;
                else curr_type = String;
                f_name = f;
                g_lex(); assign_op();
            }
            else throw -1;
            while (lex.second == ",") {
                g_lex();
                if (lex.second == "int" || lex.second == "double" || lex.second == "string") {
                    if (lex.second == "int") curr_type = Int;
                    else if (lex.second == "double") curr_type == Double;
                    else curr_type = String;
                    f_name = f;
                    g_lex(); assign_op();
                }
                else throw -1;
            }
            if (lex.second != "}") {
                cout << "expected '}'" << endl;
                throw -1;
            }
            g_lex();
        } else throw -1;
    } else throw -1;
}

void for_loop() {
    plz.push_back(unit(Op, false, true, false, "$CT"));
    if (lex.second == "(") {
        create_tid();
        g_lex();
        int t1, t2, t3, t4;
        bool whl = false;
        if (lex.second == "int" || lex.second == "double" || lex.second == "string") {
            if (lex.second == "int") curr_type = Int;
            else if (lex.second == "double") curr_type = Double;
            else curr_type = String;
            g_lex();
            assign_op();
            if (lex.second != ";") {
                cout << "expected ';'" << endl;
                throw -1;
            }
            g_lex();
            t1 = plz.size();
            expr_0();
            if (lex.second != ";") {
                cout << "expected ';'" << endl;
                throw -1;
            }
            auto _tmp = plz.back();
            plz.pop_back();
            while (!op_stk.empty()) {
                plz.push_back(unit(Op, false, true, false, op_stk.back().opr));
                op_stk.pop_back();
            }
            plz.push_back(unit(Int, false, false, false, "_"));
            plz.push_back(unit(Op, false, true, false, "$F"));
            plz.push_back(unit(Int, false, false, false, "_"));
            plz.push_back(unit(Op, false, true, false, "$"));
            t2 = plz.size()-4;
            t4 = plz.size()-2;
            t3 = plz.size();
            plz.push_back(_tmp);
            cout << '*' << endl;
            if (stk.back().type == String) {
                cout << "invalid type of clause in for loop" << endl;
                throw -2;
            }

            g_lex();
            expr_0();
            while (!op_stk.empty()) {
                plz.push_back(unit(Op, false, true, false, op_stk.back().opr));
                op_stk.pop_back();
            }
            plz.push_back(unit(Int, false, false, false, itos(t1)));
            plz.push_back(unit(Op, false, true, false, "$"));
            plz.push_back(_tmp);
            plz[t4].value = itos(plz.size());
            //plz[t2].value = itos(plz.size());
        } else {
            whl=true;
            //g_lex();
            t1=plz.size();
            expr_0();
            while (!op_stk.empty()) {
                plz.push_back(unit(Op, false, true, false, op_stk.back().opr));
                op_stk.pop_back();
            }
            plz.push_back(unit(Int, false, false, false, "_"));
            plz.push_back(unit(Op, false, true, false, "$F"));
            t2 = plz.size()-2;
            if (stk.back().type == String) {
                cout << "invalid type of clause in for loop" << endl;
                throw -2;
            }
        }
        if (lex.second != ")") {
            cout << "expected ')'" << endl;
            throw -1;
        }
        g_lex();
        if (lex.second == "{") {
            g_lex();
            operators();
            if (whl) {
                plz.push_back(unit(Int, false, false, false, itos(t1)));
                plz.push_back(unit(Op, false, true, false, "$"));
                plz[t2].value = itos(plz.size());
            } else {
                plz.push_back(unit(Int, false, false, false, itos(t3+1))); // fixed
                plz.push_back(unit(Op, false, true, false, "$"));
                plz[t2].value = itos(plz.size());
            }
            if (lex.second != "}") {
                cout << "expected '}'" << endl;
                throw -1;
            }
            g_lex();
        } else {
            operator_();
            plz.push_back(unit(Int, false, false, false, itos(t3)));
            plz.push_back(unit(Op, false, true, false, "$"));
            plz[t2].value = itos(plz.size());
        }
        delete_tid();
    } else throw -1;
    plz.push_back(unit(Op, false ,true, false ,"$DT"));
}

string temp_name = "";
void assign_op() {
    if (lex.first == A) {
        temp_name = lex.second;
        plz.push_back(unit(curr_type, true, false, true, f_name+temp_name));
        plz.push_back(unit(Op, false, true, false, "@"));
        cout << f_name + temp_name << endl;
        g_lex();
        if (lex.second != "=") {
            push_id(f_name + temp_name, val_type(curr_type, true));
            f_name = "";
            //plz.push_back(unit(Op, false, true, false, ";"));
            return;
        }
        op_stk.push_back(op("=", 0));
        g_lex();
        if ((lex.first == N || lex.first == F || lex.first == A || lex.second == "(") && curr_type != String) {
            expr_0();
            if (stk.back().type != Int && curr_type == Int) {
                cout << "type mismatch" << endl;
                throw -2;
            }
            push_id(f_name + temp_name, val_type(curr_type, true));
            f_name = "";
        }
        else if (lex.first == C) {
            if (curr_type != String) {
                cout << "type mismatch" << endl;
                throw -2;
            }
            push_id(f_name + temp_name, val_type(curr_type, true));
            f_name = "";
            g_lex();
        }
        else if (lex.second == "[") {
            op_stk.pop_back();
            plz.pop_back();
            auto t = plz.back();
            plz.pop_back();
            t.is_arr = true;
            push_id(f_name + temp_name, val_type(curr_type, true, false)); // arrays cannot be under operation
            f_name = "";
            g_lex();
            val ct = curr_type;
            if ((lex.first == N || lex.first == A || lex.second == "(" || lex.first == F) && ct != String) {
                expr_0();
                if (stk.back().type != Int && ct == Int) {
                    cout << "type mismatch 1" << endl;
                    throw -2;
                }
            }
            else if (lex.first == C && ct == String) {
                plz.push_back(unit(String, false, false, false, lex.second));
                g_lex();
            }
            else {
                cout << ct << " " << lex.first << endl;
                cout << "type mismatch 2" << endl;
                throw -2;
            }
            int arr_count = 1;
            /*if (stk.back().type != ct) {
                cout << "type mismatch" << endl;
                throw -2;
            }*/
            while (lex.second == ",") {
                ++arr_count;
                g_lex();
                if ((lex.first == N || lex.first == A || lex.second == "(" || lex.first == F) && ct != String) {
                    expr_0();
                    if (stk.back().type != Int && ct == Int) {
                        cout << "type mismatch 1" << endl;
                        throw -2;
                    }
                }
                else if (lex.first == C && ct == String) {
                    plz.push_back(unit(String, false, false, false, lex.second));
                    g_lex();
                }
                else {
                    cout << "type mismatch 2" << endl;
                    throw -2;
                }
            }
            if (lex.second != "]") {
                cout << "expected ']'" << endl;
                throw -1;
            }
            plz.push_back(t);
            plz.push_back(unit(Int, false, false, false, itos(arr_count)));
            plz.push_back(unit(Op, false, false, false, "@[]"));
            g_lex();
        }
        //plz.push_back(unit(Op, false, true, false, "="));
    } else {
        cout << "expected a valid identifier" << endl;
        throw -1;
    }
}

void func() {
    cout << lex.second << endl;
    int t1;
    if (lex.first == A) {
        push_id(lex.second, val_type(Int, false, true)); /** must be false false, and expr_8 must be fixed **/ /** Why man? **/ /** I dont know **/
        //scopes.back().second[lex.second].operatable = false;
        create_tid();
        t1 = plz.size();
        plz.push_back(unit(Int, false, false, false, "_"));
        plz.push_back(unit(Op, false ,true, false, "$"));
        plz.push_back(unit(Op, false, true, false, "$CT"));
        string func_name = lex.second;
        runtime_fn_list[func_name] = plz.size()-1;
        int param_count = 0;
        vector<val> param_tp;
        vector<bool> is_arr;
        g_lex();
        if (lex.second == "(") {
            g_lex();
            if (lex.second == "int" || lex.second == "double" || lex.second == "string") {
                if (lex.second == "int") curr_type = Int;
                else if (lex.second == "double") curr_type == Double;
                else curr_type = String;
                param_tp.push_back(curr_type);
                //plz.push_back(unit(Type, false, false, false, lex.second));
                g_lex();
                string name = lex.second;
                plz.push_back(unit(curr_type, true, false, true, name));
                if (lex.first != A) throw -1;
                bool operatable = true;
                g_lex();
                if (lex.second == "[") {
                    g_lex();
                    if (lex.second != "]") {
                        throw -1;
                    }
                    g_lex();
                    is_arr.push_back(true);
                    operatable=false;
                } else is_arr.push_back(false);
                push_id(name, val_type(curr_type, true, operatable));
                param_count++;
            }
            while (lex.second == ",") {
                g_lex();
                if (lex.second == "int" || lex.second == "double" || lex.second == "string") {
                    if (lex.second == "int") curr_type = Int;
                    else if (lex.second == "double") curr_type == Double;
                    else curr_type = String;
                    param_tp.push_back(curr_type);
                    g_lex();
                    string name = lex.second;
                    plz.push_back(unit(curr_type, true, false, true, name));
                    if (lex.first != A) throw -1;
                    bool operatable = true;
                    g_lex();
                    param_count++;
                    if (lex.second == "[") {
                        g_lex();
                        if (lex.second != "]") {
                            throw -1;
                        }
                        g_lex();
                        is_arr.push_back(true);
                        operatable = false;
                    } else is_arr.push_back(false);
                    push_id(name, val_type(curr_type, true, operatable));
                }
                else throw -1;
            }
            plz.push_back(unit(Int, false ,false ,false, itos(param_count))); /** amount of parameters **/
            plz.push_back(unit(Op, false, true, false, "@!"));
            push_fn(func_name, func_prototype(Int, param_count, param_tp, is_arr));
            if (lex.second != ")") {
                cout << "expected ')'" << endl;
                throw -1;
            }
            g_lex();
            if (lex.second == "{") {
                g_lex();
                operators();
                if(lex.second != "}") {
                    cout << "expected '}'" << endl;
                    throw -1;
                }
                g_lex();
                cout << "here " << lex.second << endl;
            } else throw -1;
        } else throw -1;
    } else throw -1;
    delete_tid();
    plz.push_back(unit(Op, false ,false ,false, "$R"));
    plz[t1].value = itos(plz.size());
}

void operator_() {
    if (lex.first == N || lex.first == A || lex.first == F || lex.second == "(" || lex.second == "*" ||
        lex.second == "+" || lex.second == "-" || lex.second == "!" || lex.second == "&" || lex.second == "++" || lex.second == "--") expr_0();
    else if (lex.second == "int" || lex.second == "double" || lex.second == "string") {
        if (lex.second == "int") curr_type = Int;
        else if (lex.second == "double") {
            curr_type = Double;
            cout << lex.second << " " << curr_type;
        }
        else curr_type = String;
        //plz.push_back(unit(Type, false, false, false, lex.second));
        g_lex(); assign_op();
    }
    else if (lex.second == "struct") { g_lex(); struct_(); }
    else if (lex.second == "func") { g_lex(); func(); }
    else if (lex.second == "for") { g_lex(); for_loop(); }
    else if (lex.second == "if") { g_lex(); if_op(); }
}

void if_op() {
    //create_tid();
    int t1, t2;
    if (lex.second == "(") {
        create_tid();
        plz.push_back(unit(Op, false, true, false, "$CT")); /** Adding create tid operator **/
        g_lex();
        expr_0();
        while (!op_stk.empty()) {
            plz.push_back(unit(Op, false, true, false, op_stk.back().opr));
            op_stk.pop_back();
        }
        plz.push_back(unit(Int, false, false, false, "_"));
        plz.push_back(unit(Op, false, true, false, "$F"));
        t1 = plz.size()-2;
        if (lex.second != ")") throw -1;
        if (stk.back().type == String) {
            cout << "invalid type of clause in if" << endl;
            throw -2;
        }
        g_lex();
        //create_tid();
        if (lex.second == "{") {
            g_lex();
            operators();
            cout << "got out" << endl;
            if (lex.second != "}") {
                cout << "expected '}'" << endl;
                throw -1;
            }
            g_lex();
        } else operator_();
        delete_tid();
        if (lex.second == "else") {
            plz.push_back(unit(Int, false, false, false, "_"));
            plz.push_back(unit(Op, false, true, false, "$"));
            t2 = plz.size()-2;
            plz[t1].value = itos(plz.size());
            create_tid();
            g_lex();
            if (lex.second == "{") {
                cout << "if: " << lex.second << endl;
                g_lex();
                operators();
                if (lex.second != "}") {
                    cout << "expected '}'" << endl;
                    throw -1;
                }
                g_lex();
            } else operator_();
            delete_tid();
            plz[t2].value = itos(plz.size());
        } else plz[t1].value = itos(plz.size());
    } else throw -1;
    plz.push_back(unit(Op, false, true, false, "$DT"));
    //delete_tid();
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
    create_tid();
    push_fn("print", func_prototype(Func, 1));
    push_fn("return", func_prototype(Func, 1));
    push_id("print", val_type(Func, false));
    push_id("return", val_type(Func, false));
    push_fn("resize", func_prototype(Func, 2));
    push_id("resize", val_type(Func, false));
    operators();
}

// ##############################

double stod(string s) {
    ss << s;
    double k;
    ss >> k;
    ss.clear();
    ss.str("");
    return k;
}

string dtos(double d) {
    ss << d;
    string s;
    ss >> s;
    ss.clear();
    ss.str("");
    return s;
}

vector<map<string, pair<val, string>>> runtime_var; // runtime tid stack. {var_name, {var_type, var_value}}
vector<map<string, pair<val, vector<string>>>> runtime_arr; // runtime array tids. {arr_name, {arr_type, array values}}
void var_assign(string name, string value) {
    for (int i = runtime_var.size()-1; i >= 0; --i) {
        if (runtime_var[i].count(name)) {
            runtime_var[i][name].second = value;
            break;
        }
    }
}

unit get_var_val(string name) {
    for (int i = runtime_var.size()-1; i >= 0; --i) {
        if (runtime_var[i].count(name)) {
            auto t = runtime_var[i][name];
            //cout << t.second << " " << t.first << endl;
            return unit(t.first, false, false, false, t.second);
        }
    }
}

void arr_assign(unit arr, int sz, vector<unit> &values) {
    for (int i = runtime_arr.size()-1; i >= 0; --i) {
        if (runtime_arr[i].count(arr.value)) {
            runtime_arr[i][arr.value].second.clear();

            break;
        }
    }
}

string get_arr_value(string arr, int ind) {
    for (int i = runtime_arr.size()-1; i >= 0; --i) {
        if (runtime_arr[i].count(arr)) {
            if (ind >= runtime_arr[i][arr].second.size()) throw -10;
            return runtime_arr[i][arr].second[ind];
        }
    }
    return "undefined";
}

vector<vector<unit>> runtime_stack_of_stacks;

void exec() {
    runtime_stack_of_stacks.emplace_back();
    vector<unit> runtime_stack;
    runtime_var.emplace_back();
    runtime_arr.emplace_back();
    vector<int> ret_pos;
    cout << "execution" << endl;
    vector<string> params;
    //unit t1, t2();
    for (int i = 0; i < plz.size(); ++i) {
        //cout << i << endl;
        if (plz[i].tp != Op) {
            if (plz[i].is_name && !(plz[i].value == "print" || plz[i].value == "return") && plz[i].tp != Func) {
                if (!plz[i].is_adress) {runtime_stack.push_back(get_var_val(plz[i].value));/*cout << runtime_stack.back().value << "***" << endl;*/}
                else runtime_stack.push_back(plz[i]);
            }
            else runtime_stack.push_back(plz[i]);
            //cout <<runtime_stack.back().value << " ";
        }
        else {
            if (plz[i].value == "$N") {
                //cout << runtime_stack.back().value << endl;
                //cout << "h" << endl;
                auto t = runtime_stack.back();
                runtime_stack.pop_back();
                if (t.value == "print") {
                    //runtime_stack.pop_back();
                    runtime_stack.pop_back();
                    cout << runtime_stack.back().value << endl;
                    runtime_stack.pop_back();
                } else if (t.value == "return") {
                    runtime_stack.pop_back();
                    cout << "returning: " << runtime_stack.back().value << endl;
                    if (!ret_pos.empty()) {
                        auto t = runtime_stack.back();
                        i = ret_pos.back();
                        ret_pos.pop_back();
                        runtime_stack_of_stacks.pop_back();
                        runtime_stack = runtime_stack_of_stacks.back();
                        runtime_stack.push_back(t);
                    } else {
                        cout << "invalid return usage" << endl;
                        throw -10;
                    }
                    runtime_stack.pop_back();
                } else {
                    //cout << "here" << endl;
                    int param_count = (int)stod(runtime_stack.back().value);
                    runtime_stack.pop_back();
                    params.clear();
                    while (param_count > 0) {
                        params.push_back(runtime_stack.back().value);
                        runtime_stack.pop_back();
                        param_count--;
                    }
                    reverse(params.begin(), params.end());
                    ret_pos.push_back(i+1);
                    i = runtime_fn_list[t.value]-1;
                    runtime_stack_of_stacks.emplace_back();
                    //runtime_stack = runtime_stack_of_stacks.back();
                    //cout << "ok new stack" << endl;
                }
            } else if (plz[i].value == "$R") {
                i = ret_pos.back();
                ret_pos.pop_back();
                //runtime_stack_of_stacks.pop_back();
                //runtime_stack = runtime_stack_of_stacks.back();
                //cout << "ok del stack" << endl;
            } else if (plz[i].value == "$DT") {
                //cout << "deleting tid" << endl;
                runtime_var.pop_back();
                runtime_arr.pop_back();
            } else if (plz[i].value == "$CT") {
                //cout << "creating tid" << endl;
                runtime_var.emplace_back();
                runtime_arr.emplace_back();
            } else if (plz[i].value == "+") {
                auto b = runtime_stack.back();
                runtime_stack.pop_back();
                auto a = runtime_stack.back();
                runtime_stack.pop_back();
                double res = stod(a.value) + stod(b.value);
                if (a.tp == Double || b.tp == Double) runtime_stack.push_back(unit(Double, false, false, false, dtos(res)));
                else runtime_stack.push_back(unit(Double, false, false, false, dtos((int)res)));
               // cout << "sum: " << runtime_stack.back().value << endl;
            } else if (plz[i].value == "-") {
                auto b = runtime_stack.back();
                runtime_stack.pop_back();
                auto a = runtime_stack.back();
                runtime_stack.pop_back();
                double res = stod(a.value) - stod(b.value);
                if (a.tp == Double || b.tp == Double) runtime_stack.push_back(unit(Double, false, false, false, dtos(res)));
                else runtime_stack.push_back(unit(Double, false, false, false, dtos((int)res)));
            } else if (plz[i].value == "*") {
                auto b = runtime_stack.back();
                runtime_stack.pop_back();
                auto a = runtime_stack.back();
                runtime_stack.pop_back();
                double res = stod(a.value) * stod(b.value);
                if (a.tp == Double || b.tp == Double) runtime_stack.push_back(unit(Double, false, false, false, dtos(res)));
                else runtime_stack.push_back(unit(Double, false, false, false, dtos((int)res)));
            } else if (plz[i].value == "/") {
                auto b = runtime_stack.back();
                runtime_stack.pop_back();
                auto a = runtime_stack.back();
                runtime_stack.pop_back();
                double res = stod(a.value) / stod(b.value);
                if (a.tp == Double || b.tp == Double) runtime_stack.push_back(unit(Double, false, false, false, dtos(res)));
                else runtime_stack.push_back(unit(Double, false, false, false, dtos((int)res)));
            } else if (plz[i].value == "@") {
                if (runtime_var.back().count(runtime_stack.back().value) == 0) runtime_var.back().insert({runtime_stack.back().value, {runtime_stack.back().tp, ""}});
                else runtime_var.back()[runtime_stack.back().value] = {runtime_stack.back().tp, ""};
            } else if (plz[i].value == "@!") {
                int param_count = (int)stod(runtime_stack.back().value);
                //cout << "done" << endl;
               // cout << "size: " << runtime_stack.size() << endl;
                runtime_stack.pop_back();
                //cout << "pop" << endl;
                if (param_count != params.size()) {
                    //cout << "runtime error function call" << endl;
                    throw -10;
                }
                while (param_count > 0) {
                    //cout << "inserting: " << runtime_stack.back().value << endl;
                    runtime_var.back().insert({runtime_stack.back().value, {runtime_stack.back().tp, ""}});
                    var_assign(runtime_stack.back().value, params.back());
                    runtime_stack.pop_back();
                    params.pop_back();
                    param_count--;
                }
                //cout << "heh" << endl;
            } else if (plz[i].value == "@[]") {
                auto sz = runtime_stack.back();
                runtime_stack.pop_back();
                auto arr = runtime_stack.back();
                runtime_stack.pop_back();
                //runtime_arr.back().insert({arr.value, {arr.tp, vector<string>(0)}});
                vector<string> k;
                int s = (int)stod(sz.value);
                //cout << s << "***\n";
                while (s > 0) {
                    k.push_back(runtime_stack.back().value);
                    runtime_stack.pop_back();
                    --s;
                }
                //arr_assign(arr, (int)stod(sz.value), runtime_stack);
                reverse(k.begin(), k.end());
                runtime_arr.back().insert({arr.value, {arr.tp, k}});
                //cout << runtime_arr.back()[arr.value].second[0] << endl;
            } else if (plz[i].value == "[]") {
                //cout << "here" << endl;
                //cout << runtime_stack.size() << endl;
                auto arr = runtime_stack.back();
                runtime_stack.pop_back();
                auto index = runtime_stack.back();
                runtime_stack.pop_back();
                runtime_stack.push_back(unit(arr.tp, true, false, false, true, get_arr_value(arr.value, (int)stod(index.value))));
                runtime_stack.back().arr_name = arr.value;
                runtime_stack.back().index = (int)stod(index.value);
                //cout << "ok" << endl;
            } else if (plz[i].value == "=") {
                auto b = runtime_stack.back();
                runtime_stack.pop_back();
                auto a = runtime_stack.back();
                //cout << a.value << " " << b.value << endl;
                runtime_stack.pop_back();
                if (a.is_arr) {
                    for (int i = runtime_arr.size()-1; i >= 0; --i) {
                        if (runtime_arr[i].count(a.arr_name)) {
                            runtime_arr[i][a.arr_name].second[a.index] = b.value;
                        }
                    }
                }
                else var_assign(a.value, b.value);
            } else if (plz[i].value == ";") runtime_stack.clear();
            else if (plz[i].value == "$F") {
                auto b = runtime_stack.back();
                runtime_stack.pop_back();
                auto a = runtime_stack.back();
                runtime_stack.pop_back();
                if (a.value == "0") i = (int)stod(b.value)-1;
            } else if (plz[i].value == "<") {
                auto b = runtime_stack.back();
                runtime_stack.pop_back();
                auto a = runtime_stack.back();
                runtime_stack.pop_back();
                int res = stod(a.value) < stod(b.value);
                if (a.tp == Double || b.tp == Double) runtime_stack.push_back(unit(Double, false, false, false, dtos((int)res)));
                else runtime_stack.push_back(unit(Double, false, false, false, dtos((int)res)));
            } else if (plz[i].value == "$") {
                auto b = runtime_stack.back();
                runtime_stack.pop_back();
                i = (int)stod(b.value)-1;
                //cout << i << endl;
            } else if (plz[i].value == ">") {
                auto b = runtime_stack.back();
                runtime_stack.pop_back();
                auto a = runtime_stack.back();
                runtime_stack.pop_back();
                double res = stod(a.value) > stod(b.value);
                if (a.tp == Double || b.tp == Double) runtime_stack.push_back(unit(Double, false, false, false, dtos(res)));
                else runtime_stack.push_back(unit(Double, false, false, false, dtos((int)res)));
            } else if (plz[i].value == ">=") {
                auto b = runtime_stack.back();
                runtime_stack.pop_back();
                auto a = runtime_stack.back();
                runtime_stack.pop_back();
                double res = stod(a.value) >= stod(b.value);
                if (a.tp == Double || b.tp == Double) runtime_stack.push_back(unit(Double, false, false, false, dtos(res)));
                else runtime_stack.push_back(unit(Double, false, false, false, dtos((int)res)));
            } else if (plz[i].value == "<=") {
                auto b = runtime_stack.back();
                runtime_stack.pop_back();
                auto a = runtime_stack.back();
                runtime_stack.pop_back();
                double res = stod(a.value) <= stod(b.value);
                if (a.tp == Double || b.tp == Double) runtime_stack.push_back(unit(Double, false, false, false, dtos(res)));
                else runtime_stack.push_back(unit(Double, false, false, false, dtos((int)res)));
            } else if (plz[i].value == "==") {
                auto b = runtime_stack.back();
                runtime_stack.pop_back();
                auto a = runtime_stack.back();
                runtime_stack.pop_back();
                double res = stod(a.value) == stod(b.value);
                if (a.tp == Double || b.tp == Double) runtime_stack.push_back(unit(Double, false, false, false, dtos(res)));
                else runtime_stack.push_back(unit(Double, false, false, false, dtos((int)res)));
            } else if (plz[i].value == "&&") {
                auto b = runtime_stack.back();
                runtime_stack.pop_back();
                auto a = runtime_stack.back();
                runtime_stack.pop_back();
                double res = (int)stod(a.value) && (int)stod(b.value);
                if (a.tp == Double || b.tp == Double) runtime_stack.push_back(unit(Double, false, false, false, dtos(res)));
                else runtime_stack.push_back(unit(Double, false, false, false, dtos((int)res)));
            } else if (plz[i].value == ">") {
                auto b = runtime_stack.back();
                runtime_stack.pop_back();
                auto a = runtime_stack.back();
                runtime_stack.pop_back();
                double res = stod(a.value) > stod(b.value);
                runtime_stack.push_back(unit(Int, false, false, false, dtos((int)res)));
            } else if (plz[i].value == "||") {
                auto b = runtime_stack.back();
                runtime_stack.pop_back();
                auto a = runtime_stack.back();
                runtime_stack.pop_back();
                double res = stod(a.value) || stod(b.value);
                if (a.tp == Double || b.tp == Double) runtime_stack.push_back(unit(Double, false, false, false, dtos(res)));
                else runtime_stack.push_back(unit(Double, false, false, false, dtos((int)res)));
            } else if (plz[i].value == "!#") {
                auto b = runtime_stack.back();
                runtime_stack.pop_back();
                double res = !(bool)(int)stod(b.value);
                runtime_stack.push_back(unit(Int, false, false, false, dtos((int)res)));
            } else if (plz[i].value == "%") {
                auto b = runtime_stack.back();
                runtime_stack.pop_back();
                auto a = runtime_stack.back();
                runtime_stack.pop_back();
                double res = (int)stod(a.value) % (int)stod(b.value);
                runtime_stack.push_back(unit(Int, false, false, false, dtos((int)res)));
            }
        }
    }
    //    cout << endl;
    //for (int i = 0; i < runtime_stack.size(); ++i) cout << runtime_stack[i].value << " ";
    /*cout << endl;
    for (auto m : runtime_var) {
        for (auto p : m) cout << p.first << " " << p.second.second << endl;
    }*/
}

int main()
{
    fin.open("out.txt");
    //while (getline(fin, s)) cout << s << endl;
    g_lex();
    //cout << lex.first << " : " << lex.second;
    try {
        program();
        if (lex.second != "#") throw -1;
        cout << "ok" << endl;
    } catch (int e) {
        cout << "error on line " << line_cnt << " on " << lex.second << endl;
    }
    fin.close();
    //cout << op_stk.back().opr <<endl;
    while (!op_stk.empty()) {
        plz.push_back(unit(Op, false, true, false, op_stk.back().opr));
        op_stk.pop_back();
    }
    fstream fout ("plz.txt");
    for (auto p : plz) cout << p.value/* << ":" << p.is_adress/* << ":" << p.is_name */<<" ";
    for (auto p : runtime_fn_list) cout << p.first << " : " << p.second << endl;
    exec();
    fout.close();

    return 0;
}
