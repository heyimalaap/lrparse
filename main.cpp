#include <iostream>
#include <string>

using namespace std;

enum class Token {
	ID, PLUS, MULT, BRACKET_OPEN, BRACKET_CLOSE, EOI, ERR
};

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
			case '+':
				cur = lookahead + 1;
				return Token::PLUS;
			case '*':
				cur = lookahead + 1;
				return Token::MULT;
			case '(':
				cur = lookahead + 1;
				return Token::BRACKET_OPEN;
			case ')':
				cur = lookahead + 1;
				return Token::BRACKET_CLOSE;
			case '\0':
				cur = lookahead + 1;
				return Token::EOI;
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

private:
	void error() {
		cout << "Lex error: " << endl << input_buffer << endl;
		for(int i = 0; i < cur; i++) cout << ' ';
		cout << "^ error occured while trying to lex." << endl;
	}

	int cur = 0;
	string input_buffer;
};

int main() {
}
