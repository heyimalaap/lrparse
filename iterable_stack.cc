#include <iostream>
#include <stack>
#include <deque>

template <class T, class Container = std::deque<T>>
class printable_stack : public std::stack<T, Container> {
	friend std::ostream& operator<<(std::ostream& os, const printable_stack<T, Container>& stk) {
		os << "[";
		for (auto i : stk.c)
			os << " " << i;
		os << " ]";
		return os;
	}
};

int main() {
	printable_stack<int> stk;
	stk.push(1);
	stk.push(2);
	stk.push(3);
	
	std::cout << "\x1b(0jklmnqtuvwx\x1b(B" << std::endl;

	std::cout << stk << std::endl;
}
