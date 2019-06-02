#ifndef TYPESYSTEM_SEXPPARSER_H
#define TYPESYSTEM_SEXPPARSER_H
#include <iostream>
#include <string>
#include <vector>
#include <variant>
#include "lexer.h"
#include <cassert>
#include <functional>
#include <memory>
#include <cstring>
namespace SExp {
	using namespace std;
	using namespace Automaton;
	using Data=variant<string, long long, double, bool>;
	struct SList;
	using SPtr = unique_ptr<SList>;
	typedef variant<SPtr, Data> Item;
	struct SList {
		vector<Item> items;
	};
	Data parseValue(token *s) {
		string q(s->start, s->end);
		switch (s->type) {
			case _ide:
				if (q == "true") return Data(true);
				if (q == "false") return Data(false);
				return Data(q);
			case _num:
				return Data(stoll(q));
			case _flo:
				return Data(stod(q));
			default:
				assert(0);
		}
	}
	pair<token *, Item> parse(token *s, token *t) {
		using rtn = pair<token *, Item>;
		if (s->type == _lbra) {
			SPtr ls = make_unique<SList>();
			for (++s; s < t;) {
				if (s->type == _rbra) return rtn(s, move(ls));
				else {
					pair<token *, Item> ret = parse(s, t);
					s = ret.first + 1;
					ls->items.push_back(move(ret.second));
				}
			}
			return rtn(t, move(ls));
		} else return rtn(s, parseValue(s));
	}
	struct fileReader {
		char *buf, *bufend, *readend;
		int len, siz;
		void extend() {
			char *g = new char[siz * 2];
			memcpy(g, buf, siz);
			delete[]buf;
			buf = g;
		}
		fileReader(FILE *t) {
			siz = 1 << 18;
			buf = new char[siz];
			bufend = buf + siz;
			readend = buf;
			while (1) {
				int cnt = fread(readend, 1, bufend - readend, t);
				len = readend + cnt - buf;
				if (cnt == bufend - readend)extend(); else break;
				readend = buf + len;
				bufend = buf + siz;
			}
			readend = buf + len + 1;
			*readend = 0;
		}
	};
	vector<Item> parseFile(FILE *t) {
		vector<Item> q;
		fileReader fr(t);
		vector<token> tr = tokenize(fr.buf, fr.readend);
		token *st = tr.data();
		token *ed = st + tr.size();
		while (st < ed) {
			pair<token *, Item> g = parse(st, ed);
			assert(g.first != ed);
			st = g.first + 1;
			q.push_back(move(g.second));
		}
		return q;
	}
	void pi(int indent) { for (int i = 0; i < indent; ++i)cout << " "; }
	void print(const Item &t, int indent) {
		if (t.index() == 0) {
			cout << endl;
			pi(indent);
			cout << "(";
			vector<Item> &v = get<SPtr>(t)->items;
			for (const Item &q:v)
				print(q, indent + 1);
			cout << ")" << endl;
		} else {
			Data u = get<Data>(t);
			switch (u.index()) {
				case 0:
					cout << get<string>(u);
					break;
				case 1:
					cout << get<long long>(u);
					break;
				case 2:
					cout << get<double>(u);
					break;
				case 3:
					cout << (get<bool>(u) ? "true" : "false");
					break;
			}
			cout << " ";
		}
	}
}
#endif //TYPESYSTEM_SEXPPARSER_H
