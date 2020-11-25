#include <bits/stdc++.h>

using namespace std;
enum val {Int, Double, String, Bool, Op, Type, Func, Name};
int main()
{
    string s;
    ifstream fin("plz.txt");
    vector<pair<string, val>> plz;
    while (!fin.eof()) {
        fin >> s;
        int t;
        fin >> t;
        val k = static_cast<val>(t);
        plz.push_back({s, k});
    }

    string ans = "";
    vector<string> stk;
    for (int i = 0; i < plz.size(); ++i) {
        cout << plz[i].first << endl;
        if (plz[i].second == Op) {
            if (plz[i].first == ";") {
                //stk.back() += ";\n";
                continue;
            }
            string b = stk.back();
            stk.pop_back();
            string a = stk.back();
            stk.pop_back();
            stk.push_back("(" + a + plz[i].first + b + ")");
        } else stk.push_back(plz[i].first);
    }
    cout << stk.back();
    fin.close();
    return 0;
}
