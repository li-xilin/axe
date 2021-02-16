#include "def.h"
#include <stdint.h>

#define AX_UINTK_ARR_LEN 32
#define AX_UINTK_MAX ((uint64_t)0xFFFFFFFF)
#define AX_UINTK_WORD_SIZE (sizeof(uint32_t))

typedef struct ax_uintk_st ax_uintk;
struct ax_uintk_st
{
	uint32_t array[AX_UINTK_ARR_LEN];
};

enum {
	AX_UINTK_SMALLER = -1,
	AX_UINTK_EQUAL = 0,
	AX_UINTK_LARGER = 1
};

/* Initialization functions: */
void ax_uintk_init(ax_uintk* n);
void ax_uintk_from_int(ax_uintk* n, uint64_t i);
int  ax_uintk_to_int(ax_uintk* n);
void ax_uintk_from_string(ax_uintk* n, char* str, int nbytes);
void ax_uintk_to_string(ax_uintk* n, char* str, int maxsize);

/* Basic arithmetic operations: */
void ax_uintk_add(const ax_uintk* a, const ax_uintk* b, ax_uintk* c); /* c = a + b */
void ax_uintk_sub(const ax_uintk* a, const ax_uintk* b, ax_uintk* c); /* c = a - b */
void ax_uintk_mul(const ax_uintk* a, const ax_uintk* b, ax_uintk* c); /* c = a * b */
void ax_uintk_div(const ax_uintk* a, const ax_uintk* b, ax_uintk* c); /* c = a / b */
void ax_uintk_mod(const ax_uintk* a, const ax_uintk* b, ax_uintk* c); /* c = a % b */
void ax_uintk_divmod(const ax_uintk* a, const ax_uintk* b, ax_uintk* c, ax_uintk* d); /* c = a/b, d = a%b */

/* Bitwise operations: */
void ax_uintk_and(const ax_uintk* a, const ax_uintk* b, ax_uintk* c); /* c = a & b */
void ax_uintk_or(const ax_uintk* a, const ax_uintk* b, ax_uintk* c);  /* c = a | b */
void ax_uintk_xor(const ax_uintk* a, const ax_uintk* b, ax_uintk* c); /* c = a ^ b */
void ax_uintk_lshift(const ax_uintk* a, ax_uintk* b, int nbits);      /* b = a << nbits */
void ax_uintk_rshift(const ax_uintk* a, ax_uintk* b, int nbits);      /* b = a >> nbits */

/* Special operators and comparison */
int  ax_uintk_cmp(const ax_uintk* a, const ax_uintk* b);              /* Compare: returns AX_UINTK_LARGER, AX_UINTK_EQUAL or AX_UINTK_SMALLER */
int  ax_uintk_is_zero(const ax_uintk* n);                                   /* For comparison with zero */
void ax_uintk_inc(ax_uintk* n);                                       /* Increment: add one to n */
void ax_uintk_dec(ax_uintk* n);                                       /* Decrement: subtract one from n */
void ax_uintk_pow(const ax_uintk* a, const ax_uintk* b, ax_uintk* c); /* Calculate a^b -- e.g. 2^10 => 1024 */
void ax_uintk_isqrt(const ax_uintk* a, ax_uintk* b);                  /* Integer square root -- e.g. isqrt(5) => 2*/
void ax_uintk_assign(ax_uintk* dst, const ax_uintk* src);             /* Copy src into dst -- dst := src */

