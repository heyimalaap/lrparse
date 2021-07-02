
#include <string>
#include <iostream>
#include <utility>
#include <vector>
#include <set>
#include <algorithm>

using namespace std;
typedef pair<string, string> production;

struct Item {
	Item(string l, string r, string la = "$") : lhs(l), rhs(r), dot_idx(0), lookahead(la) {}
	Item(string l, string r, int di, string la = "$") : lhs(l), rhs(r), dot_idx(di), lookahead(la) {}
    Item(production p, string la = "$") : lhs(p.first), rhs(p.second), dot_idx(0), lookahead(la) {}
    Item(production p, int di, string la = "$") : lhs(p.first), rhs(p.second), dot_idx(di), lookahead(la) {}

    bool operator==(const Item& other) const {
        return (lhs == other.lhs) && (rhs == other.rhs) && (dot_idx == other.dot_idx) && (lookahead == other.lookahead);
    }

    bool operator<(const Item& other) const {
        return (lhs < other.lhs) && (rhs < other.rhs) && (dot_idx < other.dot_idx) && (lookahead < other.lookahead);
    }

    friend ostream& operator<<(ostream& os, const Item& i) {
        int idx = i.dot_idx;
        os << "[" << i.lhs << " -> " << i.rhs.substr(0, idx) << "." << i.rhs.substr(idx) << " , " << i.lookahead << "]";
        return os; 
    }

	string lhs;
	string rhs;
    string lookahead = "$";
	int dot_idx;
};


struct Grammar {

    Grammar(vector<production> p) : productions(p) {
        for (auto& i : p)
            non_terminals.insert(i.first);
        generate_lr1_items();
    }

    set<string> first(string s) {
        set<string> first_set;
        for (char c : s) {
            if (non_terminals.find(string(1, c)) == non_terminals.end()) {
                first_set.insert(c != 'i' ? string(1,c) : "id");
                break;
            } else {
                string nt = string(1, c);
                for (auto p : productions) if (p.first == nt && p.second[0] != nt[0]) {
                    auto tmp = first(p.second);
                    first_set.insert(tmp.begin(), tmp.end());
                }
            }
        }
        return first_set;
    }

    vector<Item> closure(vector<Item> I) {
        bool done = false;
        while (!done) {
            done = true;
            for (int i = 0; i < I.size(); i++) {
                Item item = I[i];
                if (non_terminals.find(item.rhs.substr(item.dot_idx, 1)) != non_terminals.end()) {
                    string nt = item.rhs.substr(item.dot_idx, 1);
                    string remaining = item.rhs.substr(item.dot_idx+1) + item.lookahead;
                    for (auto p : productions) if (p.first == nt) {
                        auto s = first(remaining);
                        for (auto k : s) {
                            auto u = Item(p, k);
                            bool exists = false;
                            for (auto& x : I) {
                                if (x.dot_idx == u.dot_idx && x.lhs == u.lhs && x.rhs == u.rhs && x.lookahead == u.lookahead) {
                                    exists = true;
                                    break;
                                }
                            }
                            if (!exists) {
                                I.push_back(u);
                                done = false;
                            }
                        }
                    }
                }
            }
        }
        return I;
    }

    vector<Item> Goto(vector<Item> I, string X) {
        vector<Item> J;
        for (auto i : I) {
            if (i.rhs[i.dot_idx] == X[0]) J.push_back(Item(i.lhs, i.rhs, X == "id" ? i.dot_idx + 2 : i.dot_idx+1, i.lookahead));
        }
        return closure(J);
    }

    bool is_in_item_set(vector<Item> set) {
        for (auto soi : item_set) {
            if (set == soi) return true;
        }
        return false;
    }

    void generate_lr1_items() {
        item_set.push_back(closure({Item(productions[0])}));
        vector<string> grammar_symbols = {"E", "T", "F", "+", "*", "id", "(", ")"};
        bool done = false;
        while (!done) {
            done = true;
            for (int i = 0; i < item_set.size(); i++) {
                for (auto symb : grammar_symbols) {
                    auto g = Goto(item_set[i], symb);
                    if (g.size() != 0 && !is_in_item_set(g)) {
                        goto_history.push_back({i, symb});
                        item_set.push_back(g);
                        done = false;
                    }
                }
            }
        }
        clean_item_set();
    }

    void clean_item_set() {
        for (auto& x : item_set) {
            vector<Item> cleaned;
            for (int i = 0; i < x.size(); i++) {
                vector<string> lookaheads;
                Item ni(x[i].lhs, x[i].rhs, x[i].dot_idx);
                for (auto i = x.begin(); i < x.end(); i++) {
                    if ((*i).lhs == ni.lhs && (*i).rhs == ni.rhs && (*i).dot_idx == ni.dot_idx) {
                        lookaheads.push_back((*i).lookahead);
                        x.erase(i);
                    }
                }
                sort(lookaheads.begin(), lookaheads.end());
                ni.lookahead = lookaheads[0];
                for (int i = 1; i < lookaheads.size(); i++)
                    ni.lookahead = ni.lookahead + "/" + lookaheads[i];
                
                cleaned.push_back(ni);
            }

            x = cleaned;
        }
    }

    vector<pair<int, string>> goto_history;
    set<string> non_terminals;
    vector<vector<Item>> item_set;
    vector<production> productions;
};

int main() {
    Grammar g({{"E'", "E"}, {"E", "E+T"}, {"E", "T"}, {"T", "T*F"}, {"T", "F"}, {"F", "(E)"}, {"F", "id"}});
}