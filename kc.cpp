#include <windows.h>
#include <iostream>

using namespace std;

int main(int argc, char** argv)
{
    if (argc != 2) {cout << "invalid coomand-line arguments" << endl; return 1;}
    string lexer_command = "lexer.exe ";
    lexer_command += argv[1];
    //cout << filename;
    system(lexer_command.c_str());
    system("synt.exe");

    return 0;
}
