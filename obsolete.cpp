/** SExpParser
 *
 */

namespace SExp {
	using namespace std;
	struct Data {
		enum {
			Atom, Int, Float, Bool
		} type;
		union {
			string atom;
			long long inte;
			double floa;
		};
		Data(string a) : atom(a), type(Atom) {}
		Data(long long a) : inte(a), type(Int) {}
		Data(double a) : floa(a), type(Float) {}
		Data(bool a) : inte(a), type(Bool) {}
		friend ostream &operator<<(ostream &t, Data a) {
			switch (a.type) {
				case Atom:
					t << a.atom;
					break;
				case Int:
					t << a.inte;
					break;
				case Float:
					t << a.floa;
					break;
				case Bool:
					t << (a.inte ? "true" : "false");
					break;
			}
		}
		Data(const Data &t) {
			switch (t.type) {
				case Atom:
					atom = t.atom, type = Atom;
					break;
				case Int:
					inte = t.inte, type = Int;
					break;
				case Float:
					floa = t.floa, type = Float;
					break;
				case Bool:
					inte = t.inte, type = Bool;
					break;
			}
		}
		`
		~Data() {
			if (type == Atom)
				atom.~string();
		}
	};
	struct SExp;
	using SExpPtr = unique_ptr<SExp>;
	using rtype = variant<SExpPtr, Data>;
	ostream &operator<<(ostream &a, rtype &b);
	struct SExp {
		vector <rtype> child;
		friend ostream &operator<<(ostream &a, SExp &b) {
			if (b.child.size() == 0)
				return a << "()";
			for (int i = 0; i < b.child.size(); ++i)
				a << (i ? " " : "(") << b.child[i];
		}
	};
	ostream &operator<<(ostream &a, rtype &b) {
		if (holds_alternative<Data>(b))
			return a << get<Data>(b);
		return a << *get<SExpPtr>(b);
	}
	using namespace Automaton;
	struct Res {
		rtype p;
		bool failed;
		token *t;
		operator bool() { return failed; }
	} fail = {NULL, 1, NULL};
	Res rtn(rtype a, token *n) { return (Res) {move(a), 0, n}; }
	using parserFn = function<Res(token * , token * )>;
	using composerFn = function<rtype(rtype, rtype)>;
	rtype push(rtype vec, rtype item) { get<SExpPtr>(vec)->child.push_back(item); }
	parserFn Eat(int type, char ch) {
		return [type, ch](token *s, token *t) {
			if (s == t)
				return move(fail);
			if (s->type == type && *(s->start) == ch)
				return rtn(NULL, t);
			return move(fail);
		};
	}
	Res repbody(parserFn a, composerFn composer, rtype reduce, token *s, token *t) {
		if (Res g = a(s, t))
			return g;
		else {
			reduce = move(composer(move(reduce), move(g.p)));
			if (Res k = repbody(a, composer, move(reduce), g.t, t))
				return g;
			else
				return k;
		}
	}
	parserFn Rep(parserFn a, composerFn composer, function<rtype()> reduceFn) {
		return [a, composer, reduceFn](token *start, token *end) {
			return repbody(a, composer, reduceFn(), start, end);
		};
	}
	parserFn Sel(parserFn a, parserFn b) {
		return [a, b](token *start, token *end) {
			if (Res g = a(start, end))
				return b(start, end);
			else
				return g;
		};
	}
	parserFn Seq(parserFn a, parserFn b, composerFn c) {
		return [a, b, c](token *start, token *end) {
			if (Res g = a(start, end))
				return move(fail);
			else if (Res t = b(start, end))
				return move(fail);
			else
				return rtn(c(move(g.p), move(t.p)), t.t);
		};
	}
	Res ParseInt(token *s, token *t) {
		if (s->type != _num)
			return move(fail);
		string c(s->start, s->end);
		return rtn(Data(stoll(c)), s + 1);
	}
	Res ParseFloat(token *s, token *t) {
		if (s->type != _flo)
			return move(fail);
		string c(s->start, s->end);
		return rtn(Data(stod(c)), s + 1);
	}
	const char *port(const char *z, int len) {
		char *c = new char[len + 1];
		memcpy(c, z, len + 1);
		return c;
	}
	Res ParseIdent(token *s, token *t) {
		if (s->type != _ide)
			return move(fail);
		string c(s->start, s->end);
		if (c == "false")
			return rtn(Data(false), s + 1);
		if (c == "true")
			return rtn(Data(true), s + 1);
		return rtn(Data(port(c.data(), c.size())), s + 1);
	}
	const parserFn parseValue = Sel(ParseInt, Sel(ParseFloat, ParseIdent));
	Res ParseList(token *s, token *t);
	rtype second(rtype a, rtype b) { return move(b); }
	rtype first(rtype a, rtype b) { return move(a); }
	parserFn operator|(parserFn a, parserFn b) { return Sel(a, b); }
	parserFn operator+(parserFn a, parserFn b) { return Seq(a, b, first); }
	parserFn operator-(parserFn a, parserFn b) { return Seq(a, b, second); }
	Res ParseAll(token *s, token *t) {
		return (parseValue | (Eat(_bra, '(') - (ParseList + Eat(_bra, ')'))))(s, t);
	}
	parserFn parseList = Rep(ParseAll, push, []() { return make_unique<SExp>(); });
	Res ParseList(token *s, token *t) { return parseList(s, t); }
	vector <rtype> Parse(FILE *f) { string c = istream(f).re; }
} // namespace SExp

struct type;
struct type_id {
	unsigned int hash;
	string type_name;
	type *type;
	bool operator<(type_id &r) {
		return hash < r.hash || (hash == r.hash && type_name < r.type_name);
	}
	type_id(string p) : type_name(p) { hash = Hashing::Hasher(p); }
	friend ostream &operator<<(ostream &a, type_id b) { return a << b.type_name; }
};
struct typeclass;
enum Typing {
	Default,
	Variant,
	Product,
	Arrow,
	Primitive
	Variable,
	Constraint
};
#define regtype(type)                                                          \
  virtual Typing get_type() { return type; }
struct type {
	type_id *name;
	regtype(Default)
};
struct variant_type {
	vector <pair<string, type *>> subs;
	regtype(Variant)
};
struct product_type {
	vector<type *> subs;
	regtype(Product)
};
struct arrow_type {
	type *from, *to;
	regtype(Arrow)
};
struct type_variable {
	typeclass *r;
	regtype(Variable)
};
struct type_constraint {
	typeclass *r;
	vector<type *> subs;
	regtype(Constraint)
};
struct primitive_type {
	regtype(Primitive)
};
struct type_instance;
struct typeclass {
	set <type_id> types;
	map<pair < type_id, type_id>, type_instance *> impls;
};
struct value;
struct expre;
struct type_instance {
	type_id *type;
	expre *toexpr;
};
struct expre {
	value *evaluated;
	expre() : evaluated(NULL) {}
};
struct value {
	regtype(Default);
};
struct context;
struct value_arrow {
	context *assoc_ctx;
};