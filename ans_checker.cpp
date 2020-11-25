#include <bits/stdc++.h>

using namespace std;

int main()
{
    ifstream f1("C:\\Users\\roman_000\\Desktop\\Folder for answerf1.txt\\A.txt");
    ifstream f2("C:\\Users\\roman_000\\Desktop\\Folder for answerf1.txt\\out.txt");

    string s1, s2;
    while (getline(f1, s1) && getline(f2,s2)) {
        //f1 >> s1;
        //f2 >> s2;
        cout << s1 << " " << s2 << endl;
        if (s1 != s2) {
            cout << s1 << " != " << s2 << endl;
            return 0;
        }
        cout << s1 << endl;
    }
    cout << "ok" << endl;

    return 0;
}
