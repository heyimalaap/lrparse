#pragma once

#include <vector>
#include <ostream>
#include "TextBox.h"

template<typename T>
struct Tree {
    struct TreeNode {
        T data;
        std::vector<TreeNode> children;
        
        TreeNode(T _data) : data(_data)  {}

        void add_child(T c) {
            children.emplace_back(c);
        }
    };

    TreeNode root;

    Tree(T _root) : root(_root) {}
    
    friend std::ostream& operator<< (std::ostream& os, Tree<T>& tree) {
        TextBox tb = create_tree_textbox(tree.root);
        os << tb.str();
        return os;
    }
private:
    static TextBox create_tree_textbox(const TreeNode& node) {
        TextBox tb;
        constexpr int padding = 2;
        tb(0, 0) << node.data;
        if (node.children.empty())
            return tb;

        std::vector<TextBox> child_tbs;
        for (auto child : node.children)
            child_tbs.push_back(create_tree_textbox(child));
        
        tb.vline(0, 1, 1);
        int i = 0;
        for (auto child : child_tbs) {
            tb.vline(i, 2, 2);
            tb(i, 4) << child;
            i += child.width() + padding;
        }
        tb.hline(0, 2, i - child_tbs[child_tbs.size() - 1].width() - padding + 1);

        return tb;
    }
};
