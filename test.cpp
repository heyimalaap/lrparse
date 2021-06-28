#include <iostream>
#include "Tree.h"

int main() {
    Tree<int> my_tree(0);
    my_tree.root.add_child(1);
    my_tree.root.children[0].add_child(2);
    my_tree.root.children[0].add_child(3);
    my_tree.root.children[0].add_child(4);
    my_tree.root.add_child(5);
    my_tree.root.children[1].add_child(6);
    my_tree.root.children[1].add_child(7);
    my_tree.root.children[1].add_child(8);
    my_tree.root.children[1].add_child(9);
    my_tree.root.children[1].children[2].add_child(10);
    my_tree.root.children[1].children[2].add_child(11);
    my_tree.root.children[1].children[2].add_child(12);

    std::cout << my_tree << std::endl;
}

