#include "buildhfm.h"
#include <fstream>
#include <iostream>
#include <iomanip>

namespace {
    unsigned long long fileSize(const std::string& f) {
        std::ifstream in(f, std::ios::binary | std::ios::ate);
        if (!in) return 0;
        return static_cast<unsigned long long>(in.tellg());
    }

    void printChar(unsigned char ch) {
        if (ch == ' ')       std::cout << "'\\s'";
        else if (ch == '\n') std::cout << "'\\n'";
        else if (ch == '\r') std::cout << "'\\r'";
        else if (ch == '\t') std::cout << "'\\t'";
        else if (ch >= 32 && ch <= 126) std::cout << "'" << static_cast<char>(ch) << "'";
        else std::cout << "0x" << std::hex << static_cast<int>(ch) << std::dec;
    }
}

void compareThreeModes(const std::string& inputFile) {
    std::array<unsigned long long, 256> freq{};
    std::ifstream fin(inputFile, std::ios::binary);
    if (!fin) {
        std::cerr << "Error: Cannot open " << inputFile << "\n";
        return;
    }

    unsigned long long total = 0;
    char c;
    while (fin.get(c)) {
        unsigned char uc = static_cast<unsigned char>(c);
        freq[uc]++;
        total++;
    }
    fin.close();

    unsigned int leafCount = 0;
    for (int i = 0; i < 256; ++i) {
        if (freq[i] != 0) leafCount++;
    }

    std::cout << "\n===== 字符统计 =====\n";
    std::cout << "字符集大小 n = " << leafCount << "\n";
    std::cout << "总字符数     = " << total << "\n";
    for (int i = 0; i < 256; ++i) {
        if (freq[i] == 0) continue;
        printChar(static_cast<unsigned char>(i));
        std::cout << " : " << freq[i] << "\n";
    }

    node* root = buildhfm(freq);
    if (!root) {
        std::cerr << "Error: input file is empty.\n";
        return;
    }

    // ================= 方案1 =================
    saveTree1(root, "hfmTree1.bin");
    encodeFile(root, inputFile, "CodeFile1.bin");
    printCodeFile("CodeFile1.bin", "CodePrint1.txt", false);
    std::ofstream fout1("TreePrint1.txt");
    printTree(root, 1, fout1, true);
    node* r1 = loadTree1("hfmTree1.bin");
    decodeFile(r1, "CodeFile1.bin", "TextFile1.txt");
    freeTree(r1);

    // ================= 方案2 =================
    saveTree2(root, "hfmTree2.bin");
    encodeFile(root, inputFile, "CodeFile2.bin");
    printCodeFile("CodeFile2.bin", "CodePrint2.txt", false); 
    std::ofstream fout2("TreePrint2.txt");
    printTree(root, 1, fout2, false);
    node* r2 = loadTree2("hfmTree2.bin");
    decodeFile(r2, "CodeFile2.bin", "TextFile2.txt");
    freeTree(r2);

    // ================= 方案3 =================
    std::vector<CanonicalCode> table;
    buildCanonicalCodes(root, table);
    saveCanonicalDict(table, "hfmTree3.bin");
    encodeCanonicalFile(table, inputFile, "CodeFile3.bin");
    printCodeFile("CodeFile3.bin", "CodePrint3.txt", false);
    decodeCanonicalFile(table, "CodeFile3.bin", "TextFile3.txt");

    freeTree(root);

    unsigned long long tree1 = fileSize("hfmTree1.bin");
    unsigned long long tree2 = fileSize("hfmTree2.bin");
    unsigned long long tree3 = fileSize("hfmTree3.bin");
    unsigned long long code1 = fileSize("CodeFile1.bin");
    unsigned long long code2 = fileSize("CodeFile2.bin");
    unsigned long long code3 = fileSize("CodeFile3.bin");
    unsigned long long inputSize = fileSize(inputFile);

    std::cout << "文件大小\n";
    std::cout << "=======================================================\n";
    std::cout << "  hfmTree1.bin : " << std::setw(6) << tree1 << " 字节 (带 n / flag / weight / ch)\n";
    std::cout << "  hfmTree2.bin : " << std::setw(6) << tree2 << " 字节 (带 flag / ch)\n";
    std::cout << "  hfmTree3.bin : " << std::setw(6) << tree3 << " 字节 (规范字典 n / ch / len)\n";

    std::cout << "\n=======================================================\n";
    std::cout << "   编码文件大小 (一致性验证)\n";
    std::cout << "=======================================================\n";
    std::cout << "  CodeFile1.bin : " << std::setw(6) << code1 << " 字节\n";
    std::cout << "  CodeFile2.bin : " << std::setw(6) << code2 << " 字节\n";
    std::cout << "  CodeFile3.bin : " << std::setw(6) << code3 << " 字节\n";
    std::cout << "  -----------------------------------------------------\n";
    if (code1 == code2 && code2 == code3) {
        std::cout << "  CodeFile.bin1 == CodeFile.bin2 == CodeFile.bin3 (完全一致)\n";
    }
    else {
        std::cout << "  警告：CodeFile 大小不一致！\n";
    }

    std::cout << "\n=======================================================\n";
    std::cout << "   压缩比例 = 总大小 / 原文件大小\n";
    std::cout << "   原文件 (input.txt) : " << inputSize << " 字节\n";

    auto printRatio = [](const char* name, unsigned long long tree, unsigned long long code, unsigned long long inputSize) {
        unsigned long long totalSize = tree + code;
        double ratio = inputSize > 0 ? (100.0 * totalSize / inputSize) : 0.0;
        std::cout << "  " << name << " (hfmTree + CodeFile) = " << std::setw(6) << totalSize
            << " 字节, 压缩比例: " << std::fixed << std::setprecision(2) << ratio << "%\n";
        };

    printRatio("方案1", tree1, code1, inputSize);
    printRatio("方案2", tree2, code2, inputSize);
    printRatio("方案3", tree3, code3, inputSize);
    std::cout << "   已生成文件清单\n";
    std::cout << "=======================================================\n";
    std::cout << "  方案1: hfmTree1.bin  CodeFile1.bin  TextFile1.txt  CodePrint1.txt  TreePrint1.txt\n";
    std::cout << "  方案2: hfmTree2.bin  CodeFile2.bin  TextFile2.txt  CodePrint2.txt  TreePrint2.txt\n";
    std::cout << "  方案3: hfmTree3.bin  CodeFile3.bin  TextFile3.txt  CodePrint3.txt  \n";
}