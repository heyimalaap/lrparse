#include <iostream>
#include <iomanip>
#include <string>
#include <stack>
#include <map>
#include <utility>
#include <vector>
#include <sstream>
#include <algorithm>
#include <functional>

using namespace std;

// FOR TREE DRAWING
class TextBox {
public:
    struct xy {
        TextBox& tb;
        int x, y;

        xy operator<< (const string& s) const {
            return {tb, x + tb.puts(x, y, s), y};
        }

        xy operator<< (const int i) const {
            string s = to_string(i);
            return {tb, x + tb.puts(x, y, s), y};
        }

        xy operator<< (const char c) const {
            tb.putc(x, y, c);
            return {tb, x + 1, y};
        }

        xy operator<< (const TextBox& _tb) const {
            tb.puttb(x, y, _tb);
            return {tb, x + _tb.width(), y};
        }
    };

    string str() const {
        stringstream ss;
        for (auto& s : m_buffer) {
            //ss << s << '\n';
            for (char c : s) {
                if (c > 0b1111)
                    ss << c;
                else {
                    switch (static_cast<int>(c) & 0b1111) {
                        case 0b01:
                            ss << '-';
                            break;
                        case 0b10:
                            ss << '|';
                            break;
                        case 0b11:
                            ss << '.';
                            break;
                        default:
                            ss << c;
                    }
                }
            }
            ss << '\n';
        }
        return ss.str();
    }
    
    TextBox::xy operator() (int x, int y) const {
        return {const_cast<TextBox&>(*this), x, y};
    }
    

    int width() const {
        int w = -1;
        for (auto& s : m_buffer) 
            w = max(w, static_cast<int>(s.size()));
        return w;
    }

    int height() const {
        return m_buffer.size();
    }

    void hline(int x, int y, int width) {
        if (y >= m_buffer.size())
            m_buffer.resize(y+1);
        if (x+width >= m_buffer[y].size())
            m_buffer[y].resize(x+width, ' ');

        for (int i = 0; i < width; i++) {
            if (m_buffer[y][x+i] > 0b1111)
                m_buffer[y][x+i] = 0;
            m_buffer[y][x+i] |= 0b1;
        }
    }

    void vline(int x, int y, int height) {
        if (y+height >= m_buffer.size())
            m_buffer.resize(y+height);
        for (int i = 0; i < height; i++) {
            if (x >= m_buffer[y+i].size())
                m_buffer[y+i].resize(x+1, ' ');
            if (m_buffer[y+i][x] > 0b1111)
                m_buffer[y+i][x] = 0;
            m_buffer[y+i][x] |= 0b10;
        }
    }

private:

    void putc(int x, int y, char c) {
        if (y >= m_buffer.size())
            m_buffer.resize(y+1);
        if (x >= m_buffer[y].size())
            m_buffer[y].resize(x+1, ' ');
        m_buffer[y][x] = c;
    }
    
    int puts(int x, int y, const string& s) {
        if (s.empty()) return x;
        for (int i = 0; i < s.size(); i++)
            putc(x+i, y, s[i]);
        return x + s.size();
    }
    
    void puttb(int x, int y, const TextBox& tb) {
        for (int i = 0; i < tb.height(); i++) {
            puts(x, y+i, tb.m_buffer[i]);
        }
    }

private:
    vector<string> m_buffer;
};


struct Tree {
    struct TreeNode {
        string data;
        vector<TreeNode> children;
        
        TreeNode(string _data) : data(_data)  {}

        void add_child(string c) {
            children.emplace_back(c);
        }
    };

	bool rightmost_add(string& lhs, string& rhs) {
		bool added = false;
		_rightmost_add(root, lhs, rhs, added);
		return added;
	}

	void _rightmost_add(TreeNode& n, string& lhs, string& rhs, bool& added) {
		if (added) return;

		if (n.children.size() == 0) {
			if (n.data == lhs) {
				if (rhs != "id") {
					for (char c : rhs) n.children.emplace_back(string(1, c));
				} else {
					n.children.emplace_back("id");
				}
				added = true;
			}
		}

		for (auto i = n.children.rbegin(); i < n.children.rend(); i++) {
			_rightmost_add(*i, lhs, rhs, added);
		}
	}

    TreeNode root;

    Tree(string _root) : root(_root) {}
    
    friend ostream& operator<< (ostream& os, Tree& tree) {
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

        vector<TextBox> child_tbs;
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

// TREE DRAWING END


#define TOKEN_CASE(chr, tkn) \
	case chr: \
		cur = lookahead + 1; \
		return tkn;

template <class T, class Container = deque<T>>
class printable_stack : public stack<T, Container> {
	friend ostream& operator<<(ostream& os, const printable_stack<T, Container>& stk) {
		stringstream ss;
		ss << "[";
		for (auto i : stk.c)
			ss << " " << i;
		ss << " ]";
		os << ss.str();
		return os;
	}
};

enum class Token {
	ID, PLUS, MULT, BRACKET_OPEN, BRACKET_CLOSE, EOI, ERR
};

string token_to_str(const Token& token) {
	switch(token) {
		case Token::ID:
			return "<ID>";
		case Token::PLUS:
			return "<PLUS>";
		case Token::MULT:
			return "<MULT>";
		case Token::BRACKET_OPEN:
			return "<BRACKET_OPEN>";
		case Token::BRACKET_CLOSE:
			return "<BRACKET_CLOSE>";
		case Token::EOI:
			return "<$>";
		case Token::ERR:
			return "<ERROR>";
		default:
			return "unrecogonized token";
	}
}

class Lexer {
public:
	Lexer(const string input) {
		input_buffer = input;
	}
	
	Token next() {
		if (cur >= input_buffer.size()) {
			// No more input to read, return end of input token
			return Token::EOI;
		}
		
		// Ignore whitespace
		while (input_buffer[cur] == ' ' || input_buffer[cur] == '\t') cur++;

		int lookahead = cur;
		switch(input_buffer[lookahead]) {
			TOKEN_CASE('+', Token::PLUS)
			TOKEN_CASE('*', Token::MULT)
			TOKEN_CASE('(', Token::BRACKET_OPEN)
			TOKEN_CASE(')', Token::BRACKET_CLOSE)
			TOKEN_CASE('\0', Token::EOI)
			case 'i':
				if (input_buffer[++lookahead] == 'd') {
					cur = lookahead + 1;
					return Token::ID;
				}
				error();
				return Token::ERR;
			default:
				error();
				return Token::ERR;
		}
	}

	friend ostream& operator<<(ostream& os, const Lexer& lex) {
		stringstream ss;
		for (int i = (lex.input_buffer[lex.cur] == ' ')? lex.cur + 1 : lex.cur; i < lex.input_buffer.size(); i++) ss << lex.input_buffer[i];
		ss << " $";
		os << ss.str();
		return os;
	}
	
	void display_current_state(const string& msg) {
		cout << input_buffer << endl;
		for(int i = 0; i < cur; i++) cout << ' ';
		cout << "^ " << msg << endl;
	}

private:
	void error() {
		cout << "Lex error: " << endl << input_buffer << endl;
		for(int i = 0; i < cur; i++) cout << ' ';
		cout << "^ error occured while trying to lex." << endl;
	}

	int cur = 0;
	string input_buffer;
};

class Action {
public:
	enum ActionType {
		Shift,
		Reduce,
		Accept,
		Error
	};

	ActionType type;
};

class ShiftAction : public Action {
public:
	ShiftAction(int shiftState)
		:shift_state(shiftState)  {
		type = Action::Shift;
	}
	int shift_state;
};

class ReduceAction : public Action {
public:
	ReduceAction(const string& productionLhs, const string& productionRhs)
		:production_lhs(productionLhs), production_rhs(productionRhs), pop_amt(productionRhs == "id" ? 1 : productionRhs.size())  {
		type = Action::Reduce;
	}
	int pop_amt;
	string production_lhs;
	string production_rhs;
};

class Parser {
public:
	Parser(map<pair<int, Token>, Action*> actionMap, map<pair<int, string>, int> gotoMap)
		:action_map(actionMap), goto_map(gotoMap) {
		parse_stack.push(0);
	}

	bool parse(const string& input) {
		Lexer lex(input);
		cout << left << setw(25) << "Stack"     << setw(25) << "Current Token" << setw(25) << "Input" << setw(25) << "Action" << endl;
		cout << left << setw(25) << parse_stack << setw(25) << "- "            << setw(25) << lex     << setw(25) << "-"<< endl;
		Token a = lex.next();
		while (true) {
			if (a == Token::ERR) return false;
			int s = parse_stack.top();
			if (action_map[{s, a}]->type == Action::Shift) {
				ShiftAction* sa = reinterpret_cast<ShiftAction*>(action_map[{s, a}]);
				parse_stack.push(sa->shift_state);
				cout << left << setw(25) << parse_stack << setw(25) << token_to_str(a) << setw(25) << lex << setw(25) << "Shift to " + to_string(sa->shift_state) << endl;
				a = lex.next();
			} else if (action_map[{s, a}]->type  == Action::Reduce) {
				ReduceAction* ra = reinterpret_cast<ReduceAction*>(action_map[{s, a}]);
				for (int i = 0; i < ra->pop_amt; i++) parse_stack.pop();
				int t = parse_stack.top();
				if (goto_map[{t, ra->production_lhs}] == -1) {
					error(a, lex);
					return false;
				}
				parse_stack.push(goto_map[{t, ra->production_lhs}]);
				cout << left << setw(25) << parse_stack << setw(25) << token_to_str(a) << setw(25) << lex << setw(25) << "Reduce by " + ra->production_lhs + " -> " + ra->production_rhs << endl;
				//cout << "Using production " << ra->production_lhs << " -> " << ra->production_rhs << endl;
				production_stack.push({ra->production_lhs, ra->production_rhs});
			} else if (action_map[{s, a}]->type == Action::Accept) {
				cout << left << setw(25) << parse_stack << setw(25) << token_to_str(a) << setw(25) << lex << setw(25) << "Accepted" << endl;
				Tree pt = create_parse_tree();
				cout << "\nThe parse tree for the string is : \n" << pt << "\n";
				return true;
			} else {
				error(a, lex);
				return false;
			}
		}
	}
private:
	printable_stack<int> parse_stack;
	map<pair<int, Token>, Action*> action_map;
	map<pair<int, string>, int> goto_map;
	stack<pair<string, string>> production_stack;

	void error(Token cur_token, Lexer& lex) {
		cout << "Encountered error while parsing : Unexpected token " << token_to_str(cur_token) << endl;
		lex.display_current_state("Parse error");
	}

	void print_state() {
		cout << "This is test string lol this string is supposed to be very big lol lets see";
		cout << "lol " << endl;
	}

	Tree create_parse_tree() {
		Tree parse_tree("E");
		while (!production_stack.empty()) {
			auto prod = production_stack.top(); production_stack.pop();
			parse_tree.rightmost_add(prod.first, prod.second);
		}
		return parse_tree;
	}
};

#define ERR_ACTN new Action {.type = Action::Error}
#define ACC_ACTN new Action {.type = Action::Accept}
#define SHFT_ACTN(i) new ShiftAction(i)
#define REDC_ACTN(i) new ReduceAction(productions[i][0], productions[i][1])

int main() {
	string productions[7][2] = {
		{"E'", "E"},
		{"E" , "E+T"},
		{"E" , "T"},
		{"T" , "T*F"},
		{"T" , "F"},
		{"F" , "(E)"},
		{"F" , "id"}
	};

	map<pair<int, Token>, Action*> action_map;
	map<pair<int, string>, int> goto_map;

	// initialize goto_map
	for (int i = 0; i < 7; i++)
		for (int j = 0; j <= 21; j++)
			goto_map[{j, productions[i][0]}] = -1;

	// Define parse table : Action
	action_map[{0, Token::PLUS}]  = ERR_ACTN     ; action_map[{0, Token::MULT}]  = ERR_ACTN     ; action_map[{0, Token::BRACKET_OPEN}]  = SHFT_ACTN(4) ; action_map[{0, Token::BRACKET_CLOSE}]  = ERR_ACTN     ; action_map[{0, Token::ID}]  = SHFT_ACTN(5) ; action_map[{0, Token::EOI}]  = ERR_ACTN    ;
	action_map[{1, Token::PLUS}]  = SHFT_ACTN(6) ; action_map[{1, Token::MULT}]  = ERR_ACTN     ; action_map[{1, Token::BRACKET_OPEN}]  = ERR_ACTN     ; action_map[{1, Token::BRACKET_CLOSE}]  = ERR_ACTN     ; action_map[{1, Token::ID}]  = ERR_ACTN     ; action_map[{1, Token::EOI}]  = ACC_ACTN    ;
	action_map[{2, Token::PLUS}]  = REDC_ACTN(2) ; action_map[{2, Token::MULT}]  = SHFT_ACTN(7) ; action_map[{2, Token::BRACKET_OPEN}]  = ERR_ACTN     ; action_map[{2, Token::BRACKET_CLOSE}]  = ERR_ACTN     ; action_map[{2, Token::ID}]  = ERR_ACTN     ; action_map[{2, Token::EOI}]  = REDC_ACTN(2);
	action_map[{3, Token::PLUS}]  = REDC_ACTN(4) ; action_map[{3, Token::MULT}]  = REDC_ACTN(4) ; action_map[{3, Token::BRACKET_OPEN}]  = ERR_ACTN     ; action_map[{3, Token::BRACKET_CLOSE}]  = ERR_ACTN     ; action_map[{3, Token::ID}]  = ERR_ACTN     ; action_map[{3, Token::EOI}]  = REDC_ACTN(4);
	action_map[{4, Token::PLUS}]  = ERR_ACTN     ; action_map[{4, Token::MULT}]  = ERR_ACTN     ; action_map[{4, Token::BRACKET_OPEN}]  = SHFT_ACTN(11); action_map[{4, Token::BRACKET_CLOSE}]  = ERR_ACTN     ; action_map[{4, Token::ID}]  = SHFT_ACTN(12); action_map[{4, Token::EOI}]  = ERR_ACTN    ;
	action_map[{5, Token::PLUS}]  = REDC_ACTN(6) ; action_map[{5, Token::MULT}]  = REDC_ACTN(6) ; action_map[{5, Token::BRACKET_OPEN}]  = ERR_ACTN     ; action_map[{5, Token::BRACKET_CLOSE}]  = ERR_ACTN     ; action_map[{5, Token::ID}]  = ERR_ACTN     ; action_map[{5, Token::EOI}]  = REDC_ACTN(6);
	action_map[{6, Token::PLUS}]  = ERR_ACTN     ; action_map[{6, Token::MULT}]  = ERR_ACTN     ; action_map[{6, Token::BRACKET_OPEN}]  = SHFT_ACTN(4) ; action_map[{6, Token::BRACKET_CLOSE}]  = ERR_ACTN     ; action_map[{6, Token::ID}]  = SHFT_ACTN(5) ; action_map[{6, Token::EOI}]  = ERR_ACTN    ;
	action_map[{7, Token::PLUS}]  = ERR_ACTN     ; action_map[{7, Token::MULT}]  = ERR_ACTN     ; action_map[{7, Token::BRACKET_OPEN}]  = SHFT_ACTN(4) ; action_map[{7, Token::BRACKET_CLOSE}]  = ERR_ACTN     ; action_map[{7, Token::ID}]  = SHFT_ACTN(5) ; action_map[{7, Token::EOI}]  = ERR_ACTN    ;
	action_map[{8, Token::PLUS}]  = SHFT_ACTN(16); action_map[{8, Token::MULT}]  = ERR_ACTN     ; action_map[{8, Token::BRACKET_OPEN}]  = ERR_ACTN     ; action_map[{8, Token::BRACKET_CLOSE}]  = SHFT_ACTN(15); action_map[{8, Token::ID}]  = ERR_ACTN     ; action_map[{8, Token::EOI}]  = ERR_ACTN    ;
	action_map[{9, Token::PLUS}]  = REDC_ACTN(2) ; action_map[{9, Token::MULT}]  = SHFT_ACTN(17); action_map[{9, Token::BRACKET_OPEN}]  = ERR_ACTN     ; action_map[{9, Token::BRACKET_CLOSE}]  = REDC_ACTN(2) ; action_map[{9, Token::ID}]  = ERR_ACTN     ; action_map[{9, Token::EOI}]  = ERR_ACTN    ;
	action_map[{10, Token::PLUS}] = REDC_ACTN(4) ; action_map[{10, Token::MULT}] = REDC_ACTN(4) ; action_map[{10, Token::BRACKET_OPEN}] = ERR_ACTN     ; action_map[{10, Token::BRACKET_CLOSE}] = REDC_ACTN(4) ; action_map[{10, Token::ID}] = ERR_ACTN     ; action_map[{10, Token::EOI}] = ERR_ACTN    ;
	action_map[{11, Token::PLUS}] = ERR_ACTN     ; action_map[{11, Token::MULT}] = ERR_ACTN     ; action_map[{11, Token::BRACKET_OPEN}] = SHFT_ACTN(11); action_map[{11, Token::BRACKET_CLOSE}] = ERR_ACTN     ; action_map[{11, Token::ID}] = SHFT_ACTN(12); action_map[{11, Token::EOI}] = ERR_ACTN    ;
	action_map[{12, Token::PLUS}] = REDC_ACTN(6) ; action_map[{12, Token::MULT}] = REDC_ACTN(6) ; action_map[{12, Token::BRACKET_OPEN}] = ERR_ACTN     ; action_map[{12, Token::BRACKET_CLOSE}] = REDC_ACTN(6) ; action_map[{12, Token::ID}] = ERR_ACTN     ; action_map[{12, Token::EOI}] = ERR_ACTN    ;
	action_map[{13, Token::PLUS}] = REDC_ACTN(1) ; action_map[{13, Token::MULT}] = SHFT_ACTN(7) ; action_map[{13, Token::BRACKET_OPEN}] = ERR_ACTN     ; action_map[{13, Token::BRACKET_CLOSE}] = ERR_ACTN     ; action_map[{13, Token::ID}] = ERR_ACTN     ; action_map[{13, Token::EOI}] = REDC_ACTN(1);
	action_map[{14, Token::PLUS}] = REDC_ACTN(3) ; action_map[{14, Token::MULT}] = REDC_ACTN(3) ; action_map[{14, Token::BRACKET_OPEN}] = ERR_ACTN     ; action_map[{14, Token::BRACKET_CLOSE}] = ERR_ACTN     ; action_map[{14, Token::ID}] = ERR_ACTN     ; action_map[{14, Token::EOI}] = REDC_ACTN(3);
	action_map[{15, Token::PLUS}] = REDC_ACTN(5) ; action_map[{15, Token::MULT}] = REDC_ACTN(5) ; action_map[{15, Token::BRACKET_OPEN}] = ERR_ACTN     ; action_map[{15, Token::BRACKET_CLOSE}] = ERR_ACTN     ; action_map[{15, Token::ID}] = ERR_ACTN     ; action_map[{15, Token::EOI}] = REDC_ACTN(5);
	action_map[{16, Token::PLUS}] = ERR_ACTN     ; action_map[{16, Token::MULT}] = ERR_ACTN     ; action_map[{16, Token::BRACKET_OPEN}] = SHFT_ACTN(11); action_map[{16, Token::BRACKET_CLOSE}] = ERR_ACTN     ; action_map[{16, Token::ID}] = SHFT_ACTN(12); action_map[{16, Token::EOI}] = ERR_ACTN    ;
	action_map[{17, Token::PLUS}] = ERR_ACTN     ; action_map[{17, Token::MULT}] = ERR_ACTN     ; action_map[{17, Token::BRACKET_OPEN}] = SHFT_ACTN(11); action_map[{17, Token::BRACKET_CLOSE}] = ERR_ACTN     ; action_map[{17, Token::ID}] = SHFT_ACTN(12); action_map[{17, Token::EOI}] = ERR_ACTN    ;
	action_map[{18, Token::PLUS}] = SHFT_ACTN(16); action_map[{18, Token::MULT}] = ERR_ACTN     ; action_map[{18, Token::BRACKET_OPEN}] = ERR_ACTN     ; action_map[{18, Token::BRACKET_CLOSE}] = SHFT_ACTN(21); action_map[{18, Token::ID}] = ERR_ACTN     ; action_map[{18, Token::EOI}] = ERR_ACTN    ;
	action_map[{19, Token::PLUS}] = REDC_ACTN(1) ; action_map[{19, Token::MULT}] = SHFT_ACTN(17); action_map[{19, Token::BRACKET_OPEN}] = ERR_ACTN     ; action_map[{19, Token::BRACKET_CLOSE}] = REDC_ACTN(1) ; action_map[{19, Token::ID}] = ERR_ACTN     ; action_map[{19, Token::EOI}] = ERR_ACTN    ;
	action_map[{20, Token::PLUS}] = REDC_ACTN(3) ; action_map[{20, Token::MULT}] = REDC_ACTN(3) ; action_map[{20, Token::BRACKET_OPEN}] = ERR_ACTN     ; action_map[{20, Token::BRACKET_CLOSE}] = REDC_ACTN(3) ; action_map[{20, Token::ID}] = ERR_ACTN     ; action_map[{20, Token::EOI}] = ERR_ACTN    ;
	action_map[{21, Token::PLUS}] = REDC_ACTN(5) ; action_map[{21, Token::MULT}] = REDC_ACTN(5) ; action_map[{21, Token::BRACKET_OPEN}] = ERR_ACTN     ; action_map[{21, Token::BRACKET_CLOSE}] = REDC_ACTN(5) ; action_map[{21, Token::ID}] = ERR_ACTN     ; action_map[{21, Token::EOI}] = ERR_ACTN    ;

	// Define parse table : Goto
	goto_map[{0, "E"}] = 1; goto_map[{0, "T"}] = 2; goto_map[{0, "F"}] = 3;
	goto_map[{4, "E"}] = 8; goto_map[{4, "T"}] = 9; goto_map[{4, "F"}] = 10;
	goto_map[{6, "T"}] = 13; goto_map[{6, "F"}] = 3;
	goto_map[{7, "F"}] = 14;
	goto_map[{11, "E"}] = 18; goto_map[{11, "T"}] = 9; goto_map[{11, "F"}] = 10;
	goto_map[{16, "T"}] = 19; goto_map[{16, "F"}] = 10;
	goto_map[{17, "F"}] = 20;

	// Create parser
	Parser parser(action_map, goto_map);
	string input;
	cout << "Enter string to parse :";
	getline(cin, input);
	parser.parse(input);
}
