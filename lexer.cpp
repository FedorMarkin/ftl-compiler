#include <iostream>
#include <vector>
#include <fstream>

#define LEXER_ERROR -1

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

#define SEP_LEN 9
char sep[] = {'{', '}', '[', ']', ',', '(', ')', ' ', ';', '\n'};

#define OP_LEN 14
char op[] = {'<', '>', '=', '+', '-', '*', '/', '!', '&', '|', '^', '~', '%'};

#define DOP_LEN 15
string double_operators[] = {"++", "--", "+=", "*=", "/=", "-=", ">=", "<=", "==", "!=", "<<", ">>", "**", "&&", "||"};

#define RESERVED_LEN 12
string reserved[] = {"int", "double", "string", "for", "while", "if", "switch", "case", "struct", "import", "func", "else"};

bool is_double_op(string c) {
    for (int i = 0; i < DOP_LEN; ++i) {
        if (c == double_operators[i]) return true;
    }
    return false;
}

bool is_separator(char c) {
    for (int i = 0; i < SEP_LEN; ++i) {
        if (c == sep[i]) return true;
    }
    return false;
}

bool is_operator(char c) {
    for (int i = 0; i < OP_LEN; ++i) {
        if (c == op[i]) return true;
    }
    return false;
}

bool is_alpha(char c) {
    if (c <= 'z' && c >= 'a' || c <= 'Z' && c >= 'A' || c == '_' || c == '.') return true;
    return false;
}

bool is_digit(char c) {
    if (c <= '9' && c >= '0') return true;
    return false;
}

bool is_reserved(string tmp) {
    for (int i = 0; i < RESERVED_LEN; ++i) {
        if (tmp == reserved[i]) return true;
    }
    return false;
}

bool is_end_comment(char a, char b) {
    if (a == '*' && b == '/') return true;
    else return false;
}

string error = "";
vector<pair<type, string> > lex(string s) {
    type state = S;
    char c;
    string tmp = "";
    vector<pair<type, string> > res;
    for (int i = 0; i < s.length(); ++i) {
        c = s[i];
        if (c == '\n') res.push_back({NL, ""});
        if (c == '\0') {
            if (tmp != "") res.push_back({state, tmp});
            break;
        }
        if (c == '"') { // handling string constant
            if (tmp != "") {
                res.push_back({state, tmp});
                tmp = "";
                state = S;
            }
            ++i;
            bool end_quote = false;
            while (i < s.length()) {
                if (s[i] == '"') {
                    end_quote = true;
                    break;
                }
                tmp += s[i];
                ++i;
            }
            if (!end_quote) throw LEXER_ERROR;
            res.push_back({C, tmp});
            tmp = "";
            continue;
        } else if (c == '/' && s[i+1] == '/') { // single-line comment
            if (tmp != "") {
                res.push_back({state, tmp});
                tmp = "";
            }
            state = S;
            while (++i, i < s.length() && s[i] != '\n')res.push_back({NL, ""});
            continue;
        } else if (c == '/' && s[i+1] == '*') { // multi-line coment
            //cout << "here" << endl;
            if (tmp != "") {
                res.push_back({state, tmp});
                tmp = "";
            }
            state = S;
            while (++i, i < s.length()-1 && !is_end_comment(s[i], s[i+1]));
            ++i;
            continue;
        }
        if (state == S) {
            if (is_separator(c)) { // for separators;
                tmp = c;
                if (c != ' ' && c != '\n') res.push_back({S, tmp});
                tmp = "";
                continue;
            } else if (is_operator(c)) { // for operators
                tmp += c;
                if (is_operator(s[i+1])) {
                    tmp += s[i+1]; ++i;
                    if (is_double_op(tmp)) res.push_back({O, tmp});
                    else {
                        error = tmp;
                        throw LEXER_ERROR;
                    }
                } else res.push_back({O, tmp});
                tmp = "";
            } else if (is_digit(c)) {
                tmp += c;
                state = N;
            } else if (is_alpha(c)) {
                tmp += c;
                state = A;
            }
        } else if (state == N) {
            if (is_digit(c)) {
                tmp += c;
            } else if (c == '.') {
                tmp += c;
                state = F;
            } else if (is_separator(c)) {
                res.push_back({N, tmp});
                tmp = c;
                if (c != ' ' && c != '\n') res.push_back({S, tmp});
                tmp = "";
                state = S;
            } else if (is_operator(c)) {
                if (is_operator(c)) { // for operators
                    res.push_back({N, tmp});
                    tmp = "";
                    tmp += c;
                    if (is_operator(s[i+1])) {
                        tmp += s[i+1]; ++i;
                        if (is_double_op(tmp)) res.push_back({O, tmp});
                        else {
                            error = tmp;
                            throw LEXER_ERROR;
                        }
                    } else res.push_back({O, tmp});
                    tmp = "";
                    state = S;
                }
            } else {
                error = tmp + c;
                throw LEXER_ERROR;
            }
        } else if (state == F) {
            if (is_digit(c)) {
                tmp += c;
            } else if (is_separator(c)) {
                res.push_back({F, tmp});
                tmp = c;
                if (c != ' ' && c != '\n') res.push_back({S, tmp});
                tmp = "";
                state = S;
            } else if (is_operator(c)) { // for operators
                res.push_back({F, tmp});
                tmp = "";
                tmp += c;
                if (is_operator(s[i+1])) {
                    tmp += s[i+1]; ++i;
                    if (is_double_op(tmp)) res.push_back({O, tmp});
                    else {
                        error = tmp;
                        throw LEXER_ERROR;
                    }
                } else res.push_back({O, tmp});
                tmp = "";
                state = S;
            } else {
                error = tmp + c;
                throw LEXER_ERROR;
            }
        }
        else if (state == A) {
            if (is_alpha(c) || is_digit(c)) {
                tmp += c;
            } else if (is_separator(c)) {
                if (is_reserved(tmp)) res.push_back({R, tmp});
                else res.push_back({A, tmp});
                tmp = c;
                if (c != ' ' && c != '\n') res.push_back({S, tmp});
                tmp = "";
                state = S;
            } else if (is_operator(c)) { // for operators
                res.push_back({A, tmp});
                tmp = c;
                if (is_operator(s[i+1])) {
                    tmp += s[i+1]; ++i;
                    if (is_double_op(tmp)) res.push_back({O, tmp});
                    else {
                        error = tmp;
                        throw LEXER_ERROR;
                    }
                } else res.push_back({O, tmp});
                tmp = "";
                state = S;
            }
        }
    }
    return res;
}

int main(int argc, char** argv)
{
    ifstream fin(argv[1]);
    ofstream fout("out.txt");
    string all = "";
    char c;
    while (!fin.eof()) {
        c = fin.get();
        all += c;
    }
    fin.close();

    all += " \0";
    //lex(all);
    try {
    vector<pair<type, string> > vc = lex(all);
    for (auto p : vc) {
            cout << p.first << " : " << p.second << endl;
            fout << p.first << " " << p.second << endl;
    }
    } catch (int e) {
        if (e == LEXER_ERROR) cout << "Unexpected symbol: " << error << endl;
        else cout << "Unknown exception" << endl;
        fout << "error";
    }

    fout.close();

    return 0;
}
