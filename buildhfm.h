#pragma once
#include <array>
#include <string>
#include <vector>
#include <ostream>

struct node {
    unsigned char ch;
    unsigned long long weight;
    node* left;
    node* right;
};

struct Cmp {
    bool operator()(node* a, node* b) const;
};

node* buildhfm(const std::array<unsigned long long, 256>& freq);

// 树1：先写叶节点个数 n（4字节），再先序写 flag + weight + ch
bool saveTree1(node* root, const std::string& filename);
node* loadTree1(const std::string& filename);

// 树2：在树1基础上精简，先序写 flag + 叶子 ch，不写 n、不写 weight
bool saveTree2(node* root, const std::string& filename);
node* loadTree2(const std::string& filename);

// 兼容原接口，等价于树1
bool saveTree(node* root, const std::string& filename);
node* loadTree(const std::string& filename);

void freeTree(node* t);

bool encodeFile(node* root, const std::string& inputFile, const std::string& outputFile);
bool decodeFile(node* root, const std::string& codeFile, const std::string& textFile);

// 打印编码位串。原版输出终端 + CodePrint1.txt；重载可指定文件名并控制终端输出
void printCodeFile(const std::string& codeFile, const std::string& printFile, bool toConsole);

// 打印树。showWeight=false 时不显示权值（树2用）
void printTree(node* root, int level, std::ostream& out, bool showWeight = true);

// 规范哈夫曼（树3）
struct CanonicalCode {
    unsigned char ch;
    unsigned char len;
};

bool buildCanonicalCodes(node* root, std::vector<CanonicalCode>& table);
bool saveCanonicalDict(const std::vector<CanonicalCode>& table, const std::string& filename);
bool loadCanonicalDict(std::vector<CanonicalCode>& table, const std::string& filename);
bool encodeCanonicalFile(const std::vector<CanonicalCode>& table,
    const std::string& inputFile,
    const std::string& outputFile);
bool decodeCanonicalFile(const std::vector<CanonicalCode>& table,
    const std::string& codeFile,
    const std::string& textFile);

// 三方案对比
void compareThreeModes(const std::string& inputFile);