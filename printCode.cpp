#include "buildhfm.h"
#include <fstream>
#include <iostream>

void printCodeFile(const std::string& codeFile, const std::string& printFile, bool toConsole) {
    std::ifstream fin(codeFile, std::ios::binary);
    if (!fin) {
        std::cerr << "Error: Cannot open " << codeFile << "\n";
        return;
    }
    std::ofstream fout(printFile);
    if (!fout) {
        std::cerr << "Error: Cannot create " << printFile << "\n";
        return;
    }
    unsigned long long bitCount = 0;
    fin.read(reinterpret_cast<char*>(&bitCount), sizeof(bitCount));
    if (!fin) {
        std::cerr << "Error: Invalid CodeFile header.\n";
        return;
    }
    if (toConsole) {
        std::cout << "\n===== CodeFile Content (50 codes per line) =====\n";
    }
    unsigned long long done = 0;
    int count = 0;
    char ch;
    while (done < bitCount && fin.get(ch)) {
        unsigned char byte = static_cast<unsigned char>(ch);
        for (int i = 7; i >= 0 && done < bitCount; --i, ++done) {
            char bit = ((byte >> i) & 1) ? '1' : '0';
            if (toConsole) std::cout << bit;
            fout << bit;
            if (++count % 50 == 0) {
                if (toConsole) std::cout << "\n";
                fout << "\n";
            }
        }
    }
    if (count % 50 != 0) {
        if (toConsole) std::cout << "\n";
        fout << "\n";
    }
    if (toConsole) {
        std::cout << "================================================\n";
        std::cout << "(" << printFile << " has been generated.)\n";
    }
    fin.close();
    fout.close();
}