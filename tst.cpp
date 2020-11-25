#include <iostream>
#include <conio.h>
#include <windows.h>

using namespace std;
char c;
string str;
int i;
void s();
void e();
void k();
void gc() {
    //cout << (i<str.length()) << endl;
    if (i < str.length()) c = str[i++];
    else c = '#', cout << "eof" << endl;
    //cout << c << endl;
}
void s() {
    e();
    if (c == '+' || c == '-') {gc(); s();}
}
void e() {
    k();
    if (c == '*' || c == '/') {gc(); e();}
}
void k() {
    if (c == 'a') {
        gc();
        return;
    } else if (c == '(') {
        gc();
        s();
        cout << c << " ";
        if (c != ')') throw c;
        gc();
        cout << "here" << endl;
    } else throw c;
}


int main()
{
/*try {
        i = 0;
        str = "";
        c = 0;
        cin >> str;
        gc();
        s();
        if (c != '#') throw c;
        cout << i << endl;
        cout << "ok" << endl;
    } catch (char e) {
        cout << "error" << endl;
        cout << e << endl;
    }**/
    int a, b;
    a+b=5+3;

    return 0;
}
