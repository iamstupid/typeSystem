#ifndef TYPESYSTEM_TYPES_H
#define TYPESYSTEM_TYPES_H
#include "supportive.hpp"
namespace types {
	using namespace std;
	struct atomic_name {
		string name;
		unsigned int hash;
		atomic_name(const string &t) : name(t), hash(Hashing::Hasher(t)) {}
		atomic_name(const atomic_name &t) : name(t.name), hash(t.hash) {}
		bool operator<(const atomic_name &t) {
			return hash < t.hash || (hash == t.hash && name < t.name);
		}
		bool operator==(const atomic_name &t) {
			return hash == t.hash && name == t.name;
		}
		bool operator!=(const atomic_name &t) { return !(*this == t); }
		operator bool();
	} noname("");
	atomic_name::operator bool() { return *this != noname; }
	struct TypeClass;
	enum CType {
		None, Arr, Vari, Prod, Primit, Bind
	};
	struct Type {
		atomic_name name;
		const CType type;
		Type(CType type = None, atomic_name name = noname)
				: type(type), name(noname) {}
	};
	struct Type_Variant : public Type {
		map<atomic_name, Type *> subs;
		Type_Variant(atomic_name name = noname) : Type(Vari, name) {}
	};
	struct Type_Arrow : public Type {
		map<atomic_name, TypeClass *> poly;
		Type *From, *To;
		Type_Arrow(atomic_name name = noname) : Type(Arr, name) {}
	};
	struct Type_Product : public Type {
		vector<Type *> subs;
		Type_Product(atomic_name name = noname) : Type(Prod, name) {}
	};
	struct Type_Primitive : public Type {
		Type_Primitive(atomic_name name) : Type(Primit, name) {}
	};
	struct Type_Bind : public Type {
		set<atomic_name> subs;
		Type_Bind(atomic_name = noname) : Type(Bind, name) {}
	};
	bool compare_types(Type *a, Type *b);
	bool compare_types(Type_Arrow *a, Type_Arrow *b) {
		if (a->name == b->name)
			return 1;
		for (auto c : functional::zip(a->poly, b->poly))
			if (c.first != c.second)
				return 0;
		return compare_types(a->From, b->From) && compare_types(a->To, b->To);
	}
	bool compare_types(Type_Product *a, Type_Product *b) {
		if (a->name == b->name)
			return 1;
		if (a->subs.size() != b->subs.size())
			return 0;
		for (auto t : functional::zip(a->subs, b->subs))
			if (!compare_types(t.first, t.second))
				return 0;
		return 1;
	}
	bool compare_types(Type_Primitive *a, Type_Primitive *b) {
		return a->name == b->name;
	}
	bool compare_types(Type_Variant *a, Type_Variant *b) {
		return a->name == b->name;
	}
	bool compare_types(Type_Bind *a, Type_Bind *b) {
		if (a->name == b->name)
			return 1;
		return a->subs == b->subs;
	}
	bool compare_types(Type *a, Type *b) {
		if (a->type != b->type)
			return 0;
		if (a->name)
			if (a->name == b->name)
				return 1;
			else
				return 0;
		else if (b->name)
			return 0;
		switch (a->type) {
			case Arr:
				return compare_types((Type_Arrow *) a, (Type_Arrow *) b);
			case Vari:
				return compare_types((Type_Variant *) a, (Type_Variant *) b);
			case Prod:
				return compare_types((Type_Product *) a, (Type_Product *) b);
			case Primit:
				return compare_types((Type_Primitive *) a, (Type_Primitive *) b);
			case Bind:
				return compare_types((Type_Bind *) a, (Type_Bind *) b);
		}
	}
	Type *bind(Type *a, map<atomic_name, Type *> nameBinding);
	Type *bind()
	Type *bind(Type *a, map<atomic_name, Type *> nameBinding) {
	}
} // namespace types
#endif // TYPESYSTEM_TYPES_H
