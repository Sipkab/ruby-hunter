/* Auto-generated file, do not modify */
#ifndef RHFW_GEN_FWD_TYPES_H_
#define RHFW_GEN_FWD_TYPES_H_

#include <cstdint>

#define NULLPTR_TYPE @this.nullptr_type@

#define GENERATE_FLAG_OPERATORS(type, flagtype) \
	inline type operator~(type a){ return (type) ~(flagtype) a; } \
	inline type operator|(type a, type b){ return (type) ((flagtype) a | (flagtype) b); } \
	inline type operator&(type a, type b){ return (type) ((flagtype) a & (flagtype) b); } \
	inline type operator^(type a, type b){ return (type) ((flagtype) a ^ (flagtype) b); } \
	inline type operator<<(type a, type b){ return (type) ((flagtype) a << (flagtype) b); } \
	inline type operator>>(type a, type b){ return (type) ((flagtype) a >> (flagtype) b); } \
	inline type operator<<(type a, int i){ return (type) ((flagtype) a << i); } \
	inline type operator>>(type a, int i){ return (type) ((flagtype) a >> i); } \
	inline type& operator|=(type& a, type b){ a = a | b; return a; } \
	inline type& operator&=(type& a, type b){ a = a & b; return a; } \
	inline type& operator^=(type& a, type b){ a = a ^ b; return a; } \
	inline type& operator<<=(type& a, type b){ a = a << b; return a; } \
	inline type& operator>>=(type& a, type b){ a = a >> b; return a; } \
	inline type& operator<<=(type& a, int i){ a = a << i; return a; } \
	inline type& operator>>=(type& a, int i){ a = a >> i; return a; } \
	inline bool operator==(type a, flagtype i){ return (flagtype) a == i; } \
	inline bool operator!=(type a, flagtype i){ return (flagtype) a != i; } \
	inline bool operator==(flagtype i, type a){ return (flagtype) a == i; } \
	inline bool operator!=(flagtype i, type a){ return (flagtype) a != i; }
#define HAS_FLAG(value, flag) (((value) & (flag)) == (flag))
#define SET_FLAG(value, flag) ((value) |= (flag))
#define CLEAR_FLAG(value, flag) ((value) &= ~(flag))

//if int types are not declared in the namespace std, then we have to forward declare it
//else the using namespace std; directive results in error (unknown namespace)
namespace std {
}  // namespace std

//
//declaration of integer types
//
namespace rhfw {
//matching intx_t or std::intx_t to intx types
using namespace std;

typedef int8_t int8;
typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
typedef uint64_t uint64;

typedef int_least8_t int_least8;
typedef uint_least8_t uint_least8;
typedef int_least16_t int_least16;
typedef uint_least16_t uint_least16;
typedef int_least32_t int_least32;
typedef uint_least32_t uint_least32;
typedef int_least64_t int_least64;
typedef uint_least64_t uint_least64;

typedef int_fast8_t int_fast8;
typedef uint_fast8_t uint_fast8;
typedef int_fast16_t int_fast16;
typedef uint_fast16_t uint_fast16;
typedef int_fast32_t int_fast32;
typedef uint_fast32_t uint_fast32;
typedef int_fast64_t int_fast64;
typedef uint_fast64_t uint_fast64;

}  // namespace rhfw

//forward declaration of user types
namespace rhfw {
@foreach type in this.typeDeclarations.values() :
@opt type.toSourceForwardDeclaration()@
@
} // namespace rhfw

#endif /* RHFW_GEN_FWD_TYPES_H_ */
