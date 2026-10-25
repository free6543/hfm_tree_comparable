#include "buildhfm.h"
#include <fstream>
#include <iostream>
#include <string>

// 生成码表
static void generateCodes(node* root, std::string cur, std::string table[256]) {
    if (!root) return;

    if (root->left == nullptr) {
        if (cur.empty()) cur = "0";//防止仅一个字符情况崩溃
        table[root->ch] = cur;
        return;
    }
    generateCodes(root->left, cur + "0", table);
    generateCodes(root->right, cur + "1", table);
}

bool encodeFile(node* root, const std::string& inputFile, const std::string& outputFile) {
    std::string table[256];
    generateCodes(root, "", table);

    std::ifstream in(inputFile, std::ios::binary);
    if (!in) {
        std::cerr << "Error: Cannot open " << inputFile << "\n";
        return false;
    }

    std::ofstream out(outputFile, std::ios::binary);
    if (!out) {
        std::cerr << "Error: Cannot create " << outputFile << "\n";
        return false;
    }
    //8字节bitCount
    unsigned long long bitCount = 0;
    out.write(reinterpret_cast<const char*>(&bitCount), sizeof(bitCount));

    unsigned char buffer = 0; 
    int bitPos = 0;           // 当前buffer有效位数
    char c;

    //边读边转bit
    while (in.get(c)) {
        unsigned char uc = static_cast<unsigned char>(c);
        for (char bitChar : table[uc]) {
            buffer <<= 1;
            buffer += (bitChar == '1' ? 1 : 0);
            bitPos++;
            bitCount++;

            if (bitPos == 8) {
                out.put(static_cast<char>(buffer)); // 满 8 位就写进文件
                buffer = 0;
                bitPos = 0;
            }
        }
    }
    // 3. 处理末尾不足 8 位的情况
    if (bitPos > 0) {
        buffer <<= (8 - bitPos); // 右侧补 0
        out.put(static_cast<char>(buffer));
    }
    //更新 bitCount
    out.seekp(0, std::ios::beg);
    out.write(reinterpret_cast<const char*>(&bitCount), sizeof(bitCount));
    in.close();
    out.close();
    return true;
}