#include "buildhfm.h"
#include <queue>
#include <vector>

bool Cmp::operator()(node* a, node* b) const {
    if (a->weight != b->weight) return a->weight > b->weight;
    return a->ch > b->ch;
}

node* buildhfm(const std::array<unsigned long long, 256>& freq) {
    std::priority_queue<node*, std::vector<node*>, Cmp> pq;

    for (int i = 0; i < 256; ++i) {
        if (freq[i] == 0) continue;
        node* t = new node;
        t->ch = static_cast<unsigned char>(i);
        t->weight = freq[i];
        t->left = nullptr;
        t->right = nullptr;
        pq.push(t);
    }

    if (pq.empty()) return nullptr;
    if (pq.size() == 1) return pq.top();

    while (pq.size() > 1) {
        node* a = pq.top(); pq.pop();
        node* b = pq.top(); pq.pop();

        node* parent = new node;
        parent->ch = 0;
        parent->weight = a->weight + b->weight;
        parent->left = a;
        parent->right = b;
        pq.push(parent);
    }
    return pq.top();
}