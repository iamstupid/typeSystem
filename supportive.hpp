#ifndef TYPESYSTEM_SUPPORTIVE_H
#define TYPESYSTEM_SUPPORTIVE_H
#include <cstring>
#include <iostream>
#include <map>
#include <set>
#include <utility>
#include <vector>
namespace Hashing {
	using namespace std;
	struct CRC32 {
		CRC32(void) { this->Initialize(); }
		unsigned int iTable[256]; // CRC lookup table array.
		unsigned int Reflect(unsigned int iReflect, const char cChar) {
			unsigned int iValue = 0;
			for (int iPos = 1; iPos < (cChar + 1); iPos++) {
				if (iReflect & 1)
					iValue |= (1 << (cChar - iPos));
				iReflect >>= 1;
			}
			return iValue;
		}
		void Initialize(void) {
			unsigned int iPolynomial = 0x04C11DB7;
			memset(&this->iTable, 0, sizeof(this->iTable));
			for (int iCodes = 0; iCodes <= 0xFF; iCodes++) {
				this->iTable[iCodes] = this->Reflect(iCodes, 8) << 24;
				for (int iPos = 0; iPos < 8; iPos++) {
					this->iTable[iCodes] =
							(this->iTable[iCodes] << 1) ^
							((this->iTable[iCodes] & (1 << 31)) ? iPolynomial : 0);
				}
				this->iTable[iCodes] = this->Reflect(this->iTable[iCodes], 32);
			}
		}
		void PartialCRC(unsigned int *iCRC, const unsigned char *sData,
							 size_t iDataLength) {
			while (iDataLength--)
				*iCRC = (*iCRC >> 8) ^ this->iTable[(*iCRC & 0xFF) ^ *sData++];
		}
		unsigned int FullCRC(const unsigned char *sData, size_t iDataLength) {
			unsigned int iCRC = 0xffffffff;
			this->PartialCRC(&iCRC, sData, iDataLength);
			return (iCRC ^ 0xffffffff);
		}
		unsigned int operator()(const unsigned char *t) {
			return FullCRC(t, strlen((const char *) t));
		}
		unsigned int operator()(const string &p) {
			return operator()((const unsigned char *) p.data());
		}
	} Hasher;
} // namespace Hashing
namespace functional {
	template<class A, class B>
	struct zip_object {
		struct it {
			A a;
			B b;
			it(A a, B b) : A(a), B(b) {}
			bool operator==(const it &b) { return a == b.a || this->b == b.b; }
			bool operator!=(const it &b) { return a != b.a && this->b != b.b; }
			it operator++(int) {
				it c = *this;
				++a;
				++b;
				return c;
			}
			it &operator++() {
				++a;
				++b;
				return *this;
			}
			std::pair<decltype(*a), decltype(*b)> operator*() {
				return make_pair(*a, *b);
			}
		};
		A bgA, edA;
		B bgB, edB;
		it begin() { return it(bgA, bgB); }
		it end() { return it(edA, edB); }
		zip_object(A ba, A ea, B bb, B eb) : bgA(ba), edA(ea), bgB(bb), edB(eb) {}
	};
	template<class A, class B>
	zip_object<typename A::iterator, typename B::iterator> zip(A &a, B &b) {
		return zip_object<typename A::iterator, typename B::iterator>(
				a.begin(), a.end(), b.begin(), b.end());
	}
} // namespace functional
#endif