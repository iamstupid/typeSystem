#ifndef TYPESYSTEM_CPPTYPES_H
#define TYPESYSTEM_CPPTYPES_H
namespace cpptype {
	template<class TA, class TB>
	struct Variant {
		enum {
			A, B
		} t;
		union {
			TA a;
			TB b;
		};
		Variant(TA a) : a(a) {}
		Variant(TB a) : b(a) {}
		template<class Ta, class Tb>
		inline auto operator()(Ta fa, Tb fb) { return t == A ? fa(a) : fb(b); }
	};
	struct Null {
	};
}
#endif //TYPESYSTEM_CPPTYPES_H
