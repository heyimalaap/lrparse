#include <iostream>
#include <string>
#include <stack>
#include <map>
#include <utility>

using namespace std;

#define TOKEN_CASE(chr, tkn) \
	case chr: \
		cur = lookahead + 1; \
		return tkn;


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
		Token a = lex.next();
		while (true) {
			if (a == Token::ERR) return false;
			int s = parse_stack.top();
			if (action_map[{s, a}]->type == Action::Shift) {
				ShiftAction* sa = reinterpret_cast<ShiftAction*>(action_map[{s, a}]);
				parse_stack.push(sa->shift_state);
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
				cout << "Using production " << ra->production_lhs << " -> " << ra->production_rhs << endl;
			} else if (action_map[{s, a}]->type == Action::Accept) {
				return true;
			} else {
				error(a, lex);
				return false;
			}
		}
	}
private:
	stack<int> parse_stack;
	map<pair<int, Token>, Action*> action_map;
	map<pair<int, string>, int> goto_map;

	void error(Token cur_token, Lexer& lex) {
		cout << "Encountered error while parsing : Unexpected token " << token_to_str(cur_token) << endl;
		lex.display_current_state("Parse error");
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
	action_map[{4, Token::PLUS}]  = ERR_ACTN     ; action_map[{4, Token::MULT}]  = ERR_ACTN     ; action_map[{4, Token::BRACKET_OPEN}]  = SHFT_ACTN(11); action_map[{4, Token::BRACKET_CLOSE}]  = ERR_ACTN     ; action_map[{4, Token::ID}]  = SHFT_ACTN(11); action_map[{4, Token::EOI}]  = ERR_ACTN    ;
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
