#include "buildhfm.h"
#include <fstream>
#include <vector>
#include <iostream>

namespace {
    // 树1：先序 flag + weight + 叶子 ch
    void saveNode1(std::ostream& out, node* t) {
        unsigned char flag = (t->left == nullptr) ? 1 : 0;
        out.write(reinterpret_cast<const char*>(&flag), sizeof(flag));
        out.write(reinterpret_cast<const char*>(&t->weight), sizeof(t->weight));
        if (flag == 1) {
            out.write(reinterpret_cast<const char*>(&t->ch), sizeof(t->ch));
        }
        else {
            saveNode1(out, t->left);
            saveNode1(out, t->right);
        }
    }

    node* loadNode1(std::istream& in) {
        unsigned char flag;
        unsigned long long w;
        in.read(reinterpret_cast<char*>(&flag), sizeof(flag));
        in.read(reinterpret_cast<char*>(&w), sizeof(w));
        if (!in) return nullptr;

        node* t = new node;
        t->ch = 0;
        t->weight = w;
        t->left = nullptr;
        t->right = nullptr;

        if (flag == 1) {
            in.read(reinterpret_cast<char*>(&t->ch), sizeof(t->ch));
        }
        else {
            t->left = loadNode1(in);
            t->right = loadNode1(in);
        }
        return t;
    }

    // 树2：先序 flag + 叶子 ch
    void saveNode2(std::ostream& out, node* t) {
        unsigned char flag = (t->left == nullptr) ? 1 : 0;
        out.write(reinterpret_cast<const char*>(&flag), sizeof(flag));
        if (flag == 1) {
            out.write(reinterpret_cast<const char*>(&t->ch), sizeof(t->ch));
        }
        else {
            saveNode2(out, t->left);
            saveNode2(out, t->right);
        }
    }

    node* loadNode2(std::istream& in) {
        unsigned char flag;
        in.read(reinterpret_cast<char*>(&flag), sizeof(flag));
        if (!in) return nullptr;

        node* t = new node;
        t->ch = 0;
        t->weight = 0;
        t->left = nullptr;
        t->right = nullptr;

        if (flag == 1) {
            in.read(reinterpret_cast<char*>(&t->ch), sizeof(t->ch));
        }
        else {
            t->left = loadNode2(in);
            t->right = loadNode2(in);
        }
        return t;
    }
}

bool saveTree1(node* root, const std::string& filename) {
    if (!root) return false;
    std::ofstream out(filename, std::ios::binary);
    if (!out) return false;
    saveNode1(out, root);
    return out.good();
}

node* loadTree1(const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    if (!in) return nullptr;
    return loadNode1(in);
}

bool saveTree2(node* root, const std::string& filename) {
    if (!root) return false;
    std::ofstream out(filename, std::ios::binary);
    if (!out) return false;
    saveNode2(out, root);
    return out.good();
}

node* loadTree2(const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    if (!in) return nullptr;
    return loadNode2(in);
}

// 兼容接口不需要了可以删掉，或者这样保留
bool saveTree(node* root, const std::string& filename) {
    // 这里因为不知道 n，建议直接移除该函数，或者在头文件去声明。
    return false;
}

node* loadTree(const std::string& filename) {
    return loadTree1(filename);
}

void freeTree(node* t) {
    if (!t) return;
    freeTree(t->left);
    freeTree(t->right);
    delete t;
}