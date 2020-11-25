#include <bits/stdc++.h>

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

pair<type, string> lex;
fstream fin;
string s;
int line_cnt = 0;
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
    //cout << lex.first << " " << lex.second << endl;
    if (lex.first == NL) {
        g_lex();
        line_cnt++;
        return;
    }
}

enum u_type {Int, Double, String, Bool, Op};

struct unit {
    u_type tp;
    bool is_adress = false;
    bool is_op = false;
    bool is_name = false;
    string val;
    unit(u_type t, bool adr, bool op, bool name, string s): tp(t), is_adress(adr), is_op(op), is_name(name), val(s) {};
};

struct op {
    string opr;
    int prior;
    op(string OP, int Pr): opr(OP), prior(Pr) {};
};

int main() {
    fin.open("out.txt");
    string s;
    vector<unit> plz;
    vector<op> stk;
    while (!fin.eof()) {
        g_lex();
        cout << lex.second << endl;
        if (lex.first == N) {
            plz.push_back(unit(Int, false, false, false, lex.second));
        } else if (lex.first == F) {
            plz.push_back(unit(Double, false, false, false, lex.second));
        } else if (lex.first == C) {
            plz.push_back(unit(String, false, false, false, lex.second));
        } else if (lex.first == S) {
            if (lex.second == "(") {
                stk.push_back(op("(", -1));
            } else if (lex.second == ")") {
                while(!stk.empty() && stk.back().opr!="(") {
                    plz.push_back(unit(Op, false, true, false, stk.back().opr));
                    stk.pop_back();
                }
                if (stk.back().opr=="(") stk.pop_back();
            }
        } else if (lex.first == O) {
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
            while(!stk.empty()&&stk.back().prior>=pr){
                if (pr==7&&stk.back().prior==pr) break;
                plz.push_back(unit(Op, false, true, false, stk.back().opr));
                stk.pop_back();
            }
            stk.push_back(op(lex.second, pr));
        } else if (lex.second == "#") {
            while(!stk.empty()) {
                plz.push_back(unit(Op, false, true, false, stk.back().opr));
                stk.pop_back();
            }
        }
    }

    for (auto p : plz) cout << p.val << " ";

    return 0;
}
