#include <iostream>
#include <fstream>
#include <vector>

#define LEXER_ERROR -1

using namespace std;

enum tok_type {CONST, NAME, OPERATOR, SEPARATOR, RESERVED};

#define SEP_LEN 5
char separators[] = {',', ';', ' ', '\n', '\r'};

#define OP_LEN 9
char operators[] = {'+', '-', '*', '/', '|', '&', '=', '<', '>'};

bool is_separator(char c) {
    for (int i = 0; i < SEP_LEN; ++i) {
        if (c == separators[i]) return true;
    }
    return false;
}

bool is_alpha(char c) {
    if (c <= 'z' && c >= 'a' || c <= 'Z' && c >= 'A' || c == '_') return true;
    return false;
}

bool is_operator

class Lexer {
    string source;
    vector<pair<tok_type, string> > tokens;
    void tokenize() {
        char c;
        string tmp = "";
        int h = 0;
        for (int i = 0; i < source.length(); ++i) {
            c = source[i];
            if (h == 0) {
                if (c <= '9' && c >= '0') {
                    tmp += c;
                    h = 1;
                } else if (is_alpha(c)) {

                } else throw LEXER_ERROR;
            } else if (h == 1) {
                if (c <= '9' && c >= '0') tmp += c;
                else if (c == '.') {
                    tmp += c;
                    h = 2;
                } else if (is_separator(c)) {
                    tokens.push_back({CONST, tmp});
                    tmp = "";
                    h = 0;
                }
            } else if (h == 2) {
                if (c <= '9' && c >= '0') tmp += c;
                else if (is_separator(c)) {
                    tokens.push_back({CONST, tmp});
                    tmp = "";
                    h = 0;
                }
            }
        }
        if (tmp != "") {
            if (h == 1 || h == 2) {
                tokens.push_back({CONST, tmp});
            }
        }
    }
public:
    Lexer(string s): source(s) {
        tokenize();
    };
    void print_tokens() {
        for (int i = 0; i < tokens.size(); ++i) {
            cout << tokens[i].first << " : " << tokens[i].second << endl;
        }
    }
};

int main()
{
    ifstream fin("input.txt");
    ofstream fout("out.txt");

    char c;
    string source = "";
    while (fin.get(c)) {
        source += c;
    }

    Lexer lex(source);
    lex.print_tokens();

    fin.close();
    fout.close();


    return 0;
}
