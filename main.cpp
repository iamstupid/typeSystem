#include <iostream>
#include "SExpParser.h"
#include "Types.h"
using namespace std;
int main() {
	Automaton::init();
	auto c = SExp::parseFile(stdin);
	for (auto &g:c) {
		SExp::print(g, 0);
		cout << endl;
	}
	return 0;
}