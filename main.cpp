#include "buildhfm.h"
#include <fstream>
#include <iostream>
#include <array>
#include <iomanip>
using namespace std;

void printTree(node* root, int level, ostream& out, bool showWeight) {
    if (!root) return;
    printTree(root->right, level + 1, out, showWeight);

    if (level == 2)  out << "    ";
    else if (level != 1) {
        for (int i = 0; i < 4 * (level - 2); i++) out << " ";
        out << "|-- ";
    }

    if (root->left == nullptr) {
        if (root->ch == ' ')       out << "'\\s'";
        else if (root->ch == '\n') out << "'\\n'";
        else if (root->ch == '\r') out << "'\\r'";
        else if (root->ch == '\t') out << "'\\t'";
        else if (root->ch >= 32 && root->ch <= 126) out << "'" << (char)root->ch << "'";
        else out << "0x" << hex << (int)root->ch << dec;
        if (showWeight) out << ":" << root->weight;
        out << "\n";
    }
    else {
        if (showWeight) out << "[" << root->weight << "]\n";
        else            out << "*\n";
    }
    printTree(root->left, level + 1, out, showWeight);
}

void printChar(unsigned char ch) {
    if (ch == ' ')       cout << "'\\s'";
    else if (ch == '\n') cout << "'\\n'";
    else if (ch == '\r') cout << "'\\r'";
    else if (ch == '\t') cout << "'\\t'";
    else if (ch >= 32 && ch <= 126) cout << "'" << (char)ch << "'";
    else cout << "0x" << hex << (int)ch << dec;
}

// 1. 初始化
void initSystem() {
    array<unsigned long long, 256> freq{};
    ifstream fin("input.txt", ios::binary);
    if (!fin) {
        cerr << "Error: Cannot open input.txt!\n";
        return;
    }

    char c;
    while (fin.get(c)) {
        unsigned char uc = static_cast<unsigned char>(c);
        freq[uc]++;
    }
    fin.close();

    unsigned int n = 0;
    for (int i = 0; i < 256; ++i) {
        if (freq[i] != 0) n++;
    }
    if (!n) {
        cerr << "Error: input.txt is empty!\n";
        return;
    }
    cout << "\n===== 字符统计 =====\n";
    cout << "字符集大小（叶结点大小） n = " << n << "\n";
    for (int i = 0; i < 256; ++i) {
        if (freq[i] == 0) continue;
        printChar(static_cast<unsigned char>(i));
        cout << " : " << freq[i] << "\n";
    }

    node* root = buildhfm(freq);
    if (saveTree1(root, "hfmTree1.bin"))
        cout << "Success: Huffman tree1 saved to hfmTree.bin1 (n = " << n << ").\n";
    else
        cerr << "Error: Failed to save Huffman tree1!\n";

    freeTree(root);
}

// 2. 编码（树1）
void encodeSystem() {
    node* root = loadTree1("hfmTree1.bin");
    if (!root) {
        cout << "Error: hfmTree1.bin not found! Please run Option 1 first.\n";
        return;
    }
    if (encodeFile(root, "input.txt", "CodeFile1.bin")) {
        cout << "Success: Encoding complete. Result saved in CodeFile1.bin.\n";
    }
    else {
        cerr << "Error: Encoding failed!\n";
    }
    freeTree(root);
}

// 3. 解码（树1）
void decodeSystem() {
    node* root = loadTree1("hfmTree1.bin");
    if (!root) {
        cout << "Error: hfmTree1.bin not found! Please run Option 1 first.\n";
        return;
    }
    if (decodeFile(root, "CodeFile1.bin", "TextFile1.txt")) {
        cout << "Success: Decoding complete. Result saved in TextFile1.txt.\n";
    }
    else {
        cerr << "Error: Decoding failed!\n";
    }
    freeTree(root);
}

// 4. 打印编码文件（终端 + CodePrint1.txt）
void printCodeSystem() {
    printCodeFile("CodeFile1.bin", "CodePrint1.txt", true);
}

// 5. 打印哈夫曼树（终端 + TreePrint1.txt）
void printTreeSystem() {
    node* root = loadTree1("hfmTree1.bin");
    if (!root) {
        cout << "Error: hfmTree1.bin not found! Please run Option 1 first.\n";
        return;
    }

    cout << "\n===== Huffman Tree Structure =====\n";
    printTree(root, 1, cout, true);
    cout << "==================================\n";

    ofstream fout("TreePrint1.txt");
    if (fout) {
        printTree(root, 1, fout, true);
        fout.close();
        cout << "(TreePrint1.txt has been generated.)\n";
    }
    else {
        cerr << "Error: Cannot create TreePrint1.txt!\n";
    }

    freeTree(root);
}

// 6. 三方案对比
void compareSystem() {
    compareThreeModes("input.txt");
}

int main() {
    int choice = -1;
    while (true) {
        system("cls");
        system("color 3f");
        cout << "\n==========================================\n";
        cout << "   Huffman Encoder / Decoder\n";
        cout << "==========================================\n";
        cout << "  1. Initialize (Build Tree1)\n";
        cout << "  2. Encode  (Text -> CodeFile1.bin)\n";
        cout << "  3. Decode  (CodeFile1.bin -> TextFile1.txt)\n";
        cout << "  4. Print CodeFile1.bin (50 chars/line)\n";
        cout << "  5. Print Huffman Tree1\n";
        cout << "  6. Compare Three Modes (1/2/3)\n";
        cout << "  0. Exit\n";
        cout << "==========================================\n";
        cout << "Enter your choice: [ ]\b\b";
        cin >> choice;
        switch (choice) {
        case 1: initSystem(); system("pause");
            break;
        case 2: encodeSystem(); system("pause");
            break;
        case 3: decodeSystem(); system("pause");
            break;
        case 4: printCodeSystem(); system("pause");
            break;
        case 5: printTreeSystem(); system("pause");
            break;
        case 6: compareSystem(); system("pause");
            break;
        case 0: cout << "Exiting system. Goodbye!\n"; system("pause");
            return 0;
        default: cout << "Invalid choice. Try again.\n";
            system("pause");
        }
    }
    return 0;
}