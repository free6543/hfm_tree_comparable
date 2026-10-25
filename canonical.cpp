#include "buildhfm.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <unordered_map>

namespace {
    // 收集叶子：map<depth, vector<ch>>
    void collectLeaves(node* t, int depth, std::map<int, std::vector<unsigned char>>& mp) {
        if (!t) return;
        if (t->left == nullptr) {
            mp[depth].push_back(t->ch);
            return;
        }
        collectLeaves(t->left, depth + 1, mp);
        collectLeaves(t->right, depth + 1, mp);
    }

    //计算字符对应编码
    std::vector<std::string> generate(const std::vector<CanonicalCode>& table) {
        std::vector<std::string> codes(table.size());
        unsigned long long code = 0;
        long long prev_len = 0;
        for (size_t i = 0; i < table.size(); ++i) {
            long long len = table[i].len;
            if (i > 0) {
                code = (code + 1) << (len - prev_len);
            }
            std::string cur(len, '0');
            unsigned long long temp = code;
            for (long long k = len - 1; k >= 0; --k) {
                cur[k] = (temp & 1) ? '1' : '0';
                temp >>= 1;
            }
            codes[i] = cur;
            prev_len = len;
        }
        return codes;
    }
}

bool buildCanonicalCodes(node* root, std::vector<CanonicalCode>& table) {
    if (!root) return false;
    std::map<int, std::vector<unsigned char>> mp;
    collectLeaves(root, 0, mp); 
    table.clear();
    for (auto& kv : mp) {
        std::vector<unsigned char>& v = kv.second;
        std::sort(v.begin(), v.end()); 
        for (auto ch : v) {
            CanonicalCode c;
            c.ch = ch;
            c.len = static_cast<unsigned char>(kv.first);
            table.push_back(c);
        }
    }
    if (table.empty()) return false;
    if (table.size() == 1) {
        table[0].len = 1;
        return true;
    }
    return true;
}

bool saveCanonicalDict(const std::vector<CanonicalCode>& table, const std::string& filename) {
    std::ofstream out(filename, std::ios::binary);
    if (!out) return false;
    unsigned int n = static_cast<unsigned int>(table.size());
    out.write(reinterpret_cast<const char*>(&n), sizeof(n));
    for (const auto& c : table) {
        out.write(reinterpret_cast<const char*>(&c.ch), sizeof(c.ch));
        out.write(reinterpret_cast<const char*>(&c.len), sizeof(c.len));
    }
    return out.good();
}

bool loadCanonicalDict(std::vector<CanonicalCode>& table, const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    if (!in) return false;

    unsigned int n = 0;
    in.read(reinterpret_cast<char*>(&n), sizeof(n));
    if (!in) return false;

    table.clear();
    for (unsigned int i = 0; i < n; ++i) {
        CanonicalCode c;
        in.read(reinterpret_cast<char*>(&c.ch), sizeof(c.ch));
        in.read(reinterpret_cast<char*>(&c.len), sizeof(c.len));
        if (!in) return false;
        table.push_back(c);
    }

    if (table.empty()) return false;

    if (table.size() == 1) {
        table[0].len = 1;
    }

    return true;
}

bool encodeCanonicalFile(const std::vector<CanonicalCode>& table,
    const std::string& inputFile,
    const std::string& outputFile) {
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
    //推导字符对应编码
    std::vector<std::string> codes = generate(table);
    std::unordered_map<unsigned char, std::string> dict;
    for (size_t i = 0; i < table.size(); ++i) {
        dict[table[i].ch] = codes[i];
    }
    //先占位
    unsigned long long bitCount = 0;
    out.write(reinterpret_cast<const char*>(&bitCount), sizeof(bitCount));
    unsigned char buffer = 0;
    int bitPos = 0;
    char c;
    while (in.get(c)) {
        unsigned char uc = static_cast<unsigned char>(c);
        auto it = dict.find(uc);
        if (it == dict.end()) {
            std::cerr << "Error: char not in canonical table: 0x" << std::hex << (int)uc << "\n";
            return false;
        }
        for (char bitChar : it->second) {
            buffer <<= 1;
            buffer += (bitChar == '1' ? 1 : 0);
            bitPos++;
            bitCount++;
            if (bitPos == 8) {
                out.put(static_cast<char>(buffer));
                buffer = 0;
                bitPos = 0;
            }
        }
    }
    if (bitPos > 0) {
        buffer <<= (8 - bitPos);
        out.put(static_cast<char>(buffer));
    }
    out.seekp(0, std::ios::beg);
    out.write(reinterpret_cast<const char*>(&bitCount), sizeof(bitCount));
    in.close();
    out.close();
    return true;
}

bool decodeCanonicalFile(const std::vector<CanonicalCode>& table,
    const std::string& codeFile,
    const std::string& textFile) {
    if (table.empty()) return false;
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
    if (table.size() == 1) {
        for (unsigned long long i = 0; i < bitCount; ++i) {
            fout.put(static_cast<char>(table[0].ch));
        }
        fin.close();
        fout.close();
        return true;
    }
    std::vector<std::string> codes = generate(table);
    std::unordered_map<std::string, unsigned char> dict;
    for (size_t i = 0; i < table.size(); ++i) {
        dict[codes[i]] = table[i].ch;
    }
    std::string cur;
    unsigned long long done = 0;
    char ch;
    while (done < bitCount && fin.get(ch)) {
        unsigned char byte = static_cast<unsigned char>(ch);
        for (int i = 7; i >= 0 && done < bitCount; --i, ++done) {
            cur += ((byte >> i) & 1) ? '1' : '0';
            auto it = dict.find(cur);
            if (it != dict.end()) {
                fout.put(static_cast<char>(it->second));
                cur.clear();
            }
        }
    }
    fin.close();
    fout.close();
    return true;
}