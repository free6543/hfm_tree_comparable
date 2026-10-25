#include "buildhfm.h"
#include <fstream>
#include <iostream>

bool decodeFile(node* root, const std::string& codeFile, const std::string& textFile) {
    if (!root) return false;

    std::ifstream fin(codeFile, std::ios::binary);
    std::ofstream fout(textFile, std::ios::binary);

    if (!fin) {
        std::cerr << "Error: Cannot open " << codeFile << "\n";
        return false;
    }
    if (!fout) {
        std::cerr << "Error: Cannot create " << textFile << "\n";
        return false;
    }

    unsigned long long bitCount = 0;
    fin.read(reinterpret_cast<char*>(&bitCount), sizeof(bitCount));
    if (!fin) {
        std::cerr << "Error: Invalid CodeFile header.\n";
        return false;
    }

    if (root->left == nullptr) {
        for (unsigned long long i = 0; i < bitCount; ++i) {
            fout.put(static_cast<char>(root->ch));
        }
        fin.close();
        fout.close();
        return true;
    }

    node* cur = root;
    unsigned long long cnt = 0;
    char ch;
    while (cnt < bitCount && fin.get(ch)) {
        unsigned char byte = static_cast<unsigned char>(ch);
        for (int i = 7; i >= 0 && cnt < bitCount; --i, ++cnt) {
            int bit = (byte >> i) & 1;
            if (!cur) { fin.close(); fout.close(); return false; }
            cur = bit ? cur->right : cur->left;
            if (cur && cur->left == nullptr) {
                fout.put(static_cast<char>(cur->ch));
                cur = root;
            }
        }
    }
    fin.close();
    fout.close();
    return true;
}