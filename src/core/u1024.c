/* 
 * https://github.com/kokke/tiny-bignum-c
 */

#include "ax/u1024.h"
#include "ax/def.h"
#include "check.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

static void u1024_rshift_one_bit(ax_u1024* a);
static void u1024_lshift_one_bit(ax_u1024* a);
static void u1024_lshift_word(ax_u1024* a, int nwords);
static void u1024_rshift_word(ax_u1024* a, int nwords);

void ax_u1024_init(ax_u1024* n)
{
	CHECK_PARAM_NULL(n);

	int i;
	for (i = 0; i < AX_U1024_ARR_LEN; ++i)
		n->array[i] = 0;
}

void ax_u1024_from_int(ax_u1024* n, uint64_t i)
{
	CHECK_PARAM_NULL(n);

	ax_u1024_init(n);
	n->array[0] = i;
	n->array[1] = i >> 32;
}

ax_fail ax_u1024_to_int(ax_u1024* n, uint64_t *p)
{
	CHECK_PARAM_NULL(n);

	for (int i = 2; i < AX_U1024_ARR_LEN; i++) {
		if (n->array[i]) {
			errno = EDOM;
			return true;
		}
	}

	*p = (uint64_t)n->array[1] << 32 | n->array[0];
	return false;
}

ax_fail ax_u1024_from_string(ax_u1024* n, char* str, int nbytes)
{
	CHECK_PARAM_NULL(n);
	CHECK_PARAM_NULL(str);
	ax_assert(nbytes > 0, "nbytes must be positive");

	if (nbytes > AX_U1024_WORD_SIZE * AX_U1024_ARR_LEN * 2) {
		errno = ERANGE;
		return true;
	}

	for (int i = 0; i < nbytes; i++)
		if (!isdigit(str[i]) && !(str[i] >= 'a' && str[i] <= 'f')
				&& !(str[i] >= 'A' && str[i] <= 'F')) {
			errno = EINVAL;
			return true;
		}

	ax_u1024_init(n);

	uint32_t tmp;
	int i = nbytes - (2 * AX_U1024_WORD_SIZE); /* index into string */
	int j = 0; /* index into array */

	/* reading last hex-byte "MSB" from string first -> big endian */
	/* MSB ~= most significant byte / block ? :) */
	while (i >= 0) {
		tmp = 0;
		sscanf(&str[i], "%8x", &tmp);
		n->array[j] = tmp;
		i -= (2 * AX_U1024_WORD_SIZE); /* step AX_U1024_WORD_SIZE hex-byte(s) back in the string. */
		j += 1;               /* step one element forward in the array. */
	}
	char *end;
	char buf[8];
	strncpy(buf, str, (2 * AX_U1024_WORD_SIZE) + i);
	n->array[j] = strtol(buf, &end, 16);
	return false;
}

void ax_u1024_to_string(ax_u1024* n, char* str, int nbytes)
{
	CHECK_PARAM_NULL(n);
	CHECK_PARAM_NULL(str);
	ax_assert(nbytes > 0, "nbytes must be positive");
	ax_assert((nbytes & 1) == 0, "string format must be in hex -> equal number of bytes");

	int j = AX_U1024_ARR_LEN - 1; /* index into array - reading "MSB" first -> big-endian */
	int i = 0;                 /* index into string representation. */

	/* reading last array-element "MSB" first -> big endian */
	while ((j >= 0) && (nbytes > (i + 1))) {
		sprintf(&str[i], "%.08x", n->array[j]);
		i += (2 * AX_U1024_WORD_SIZE); /* step AX_U1024_WORD_SIZE hex-byte(s) forward in the string. */
		j -= 1;               /* step one element back in the array. */
	}

	/* Count leading zeros: */
	j = 0;
	while (str[j] == '0')
		j += 1;

	/* Move string j places ahead, effectively skipping leading zeros */ 
	for (i = 0; i < (nbytes - j); ++i)
		str[i] = str[i + j];

	/* Zero-terminate string */
	str[i] = 0;
}

void ax_u1024_dec(ax_u1024* n)
{
	CHECK_PARAM_NULL(n);
	for (int i = 0; i < AX_U1024_ARR_LEN; ++i) {
		uint32_t tmp = n->array[i];
		uint32_t res = tmp - 1;
		n->array[i] = res;
		if (!(res > tmp))
			break;
	}
}

void ax_u1024_inc(ax_u1024* n)
{
	CHECK_PARAM_NULL(n);
	for (int i = 0; i < AX_U1024_ARR_LEN; ++i) {
		uint32_t tmp = n->array[i];
		uint32_t res = tmp + 1;
		n->array[i] = res;
		if (res > tmp)
			break;
	}
}

void ax_u1024_add(const ax_u1024* a, const ax_u1024* b, ax_u1024* c)
{
	CHECK_PARAM_NULL(a);
	CHECK_PARAM_NULL(b);
	CHECK_PARAM_NULL(c);

	uint64_t tmp;
	int carry = 0;
	int i;
	for (i = 0; i < AX_U1024_ARR_LEN; ++i) {
		tmp = (uint64_t)a->array[i] + b->array[i] + carry;
		carry = (tmp > AX_U1024_MAX);
		c->array[i] = (tmp & AX_U1024_MAX);
	}
}

void ax_u1024_sub(const ax_u1024* a, const ax_u1024* b, ax_u1024* c)
{
	CHECK_PARAM_NULL(a);
	CHECK_PARAM_NULL(b);
	CHECK_PARAM_NULL(c);

	uint64_t res;
	uint64_t tmp1;
	uint64_t tmp2;
	int borrow = 0;
	int i;
	for (i = 0; i < AX_U1024_ARR_LEN; ++i) {
		tmp1 = (uint64_t)a->array[i] + (AX_U1024_MAX + 1); /* + number_base */
		tmp2 = (uint64_t)b->array[i] + borrow;;
		res = (tmp1 - tmp2);
		c->array[i] = (uint32_t)(res & AX_U1024_MAX); /* "modulo number_base" == "% (number_base - 1)" if number_base is 2^N */
		borrow = (res <= AX_U1024_MAX);
	}
}

void ax_u1024_mul(const ax_u1024* a, const ax_u1024* b, ax_u1024* c)
{
	CHECK_PARAM_NULL(a);
	CHECK_PARAM_NULL(b);
	CHECK_PARAM_NULL(c);

	ax_u1024 row;
	ax_u1024 tmp;
	int i, j;

	ax_u1024_init(c);

	for (i = 0; i < AX_U1024_ARR_LEN; ++i) {
		ax_u1024_init(&row);
		for (j = 0; j < AX_U1024_ARR_LEN; ++j) {
			if (i + j < AX_U1024_ARR_LEN) {
				ax_u1024_init(&tmp);
				uint64_t intermediate = ((uint64_t)a->array[i] * (uint64_t)b->array[j]);
				ax_u1024_from_int(&tmp, intermediate);
				u1024_lshift_word(&tmp, i + j);
				ax_u1024_add(&tmp, &row, &row);
			}
		}
		ax_u1024_add(c, &row, c);
	}
}


void ax_u1024_div(const ax_u1024* a, const ax_u1024* b, ax_u1024* c)
{
	CHECK_PARAM_NULL(a);
	CHECK_PARAM_NULL(b);
	CHECK_PARAM_NULL(c);

	ax_u1024 current;
	ax_u1024 denom;
	ax_u1024 tmp;

	ax_u1024_from_int(&current, 1);                            // int current = 1;
	ax_u1024_assign(&denom, b);                                // denom = b
	ax_u1024_assign(&tmp, a);                                  // tmp   = a

	const uint64_t half_max = 1 + (uint64_t)(AX_U1024_MAX / 2);
	bool overflow = false;
	while (ax_u1024_cmp(&denom, a) != AX_U1024_LARGER) {        // while (denom <= a) {
		if (denom.array[AX_U1024_ARR_LEN - 1] >= half_max) {
			overflow = true;
			break;
		}
		u1024_lshift_one_bit(&current);                     //   current <<= 1;
		u1024_lshift_one_bit(&denom);                       //   denom <<= 1;
	}
	if (!overflow) {
		u1024_rshift_one_bit(&denom);                       // denom >>= 1;
		u1024_rshift_one_bit(&current);                     // current >>= 1;
	}
	ax_u1024_init(c);                                           // int answer = 0;

	while (!ax_u1024_is_zero(&current)) {                       // while (current != 0)
		if (ax_u1024_cmp(&tmp, &denom) != AX_U1024_SMALLER) { //   if (dividend >= denom)
			ax_u1024_sub(&tmp, &denom, &tmp);           //     dividend -= denom;
			ax_u1024_or(c, &current, c);                //     answer |= current;
		}
		u1024_rshift_one_bit(&current);                     //   current >>= 1;
		u1024_rshift_one_bit(&denom);                       //   denom >>= 1;
	}                                                           // return answer;
}

void ax_u1024_lshift(const ax_u1024* a, ax_u1024* b, int nbits)
{
	CHECK_PARAM_NULL(a);
	CHECK_PARAM_NULL(b);
	ax_assert(nbits >= 0, "no negative shifts");

	ax_u1024_assign(b, a);
	/* Handle shift in multiples of word-size */
	const int nbits_pr_word = (AX_U1024_WORD_SIZE * 8);
	int nwords = nbits / nbits_pr_word;
	if (nwords != 0) {
		u1024_lshift_word(b, nwords);
		nbits -= (nwords * nbits_pr_word);
	}

	if (nbits != 0) {
		int i;
		for (i = (AX_U1024_ARR_LEN - 1); i > 0; --i)
			b->array[i] = (b->array[i] << nbits) | (b->array[i - 1] >> ((8 * AX_U1024_WORD_SIZE) - nbits));
		b->array[i] <<= nbits;
	}
}

void ax_u1024_rshift(const ax_u1024* a, ax_u1024* b, int nbits)
{
	CHECK_PARAM_NULL(a);
	CHECK_PARAM_NULL(b);
	ax_assert(nbits >= 0, "no negative shifts");

	ax_u1024_assign(b, a);
	/* Handle shift in multiples of word-size */
	const int nbits_pr_word = (AX_U1024_WORD_SIZE * 8);
	int nwords = nbits / nbits_pr_word;
	if (nwords != 0) {
		u1024_rshift_word(b, nwords);
		nbits -= (nwords * nbits_pr_word);
	}

	if (nbits != 0) {
		int i;
		for (i = 0; i < (AX_U1024_ARR_LEN - 1); ++i)
			b->array[i] = (b->array[i] >> nbits) | (b->array[i + 1] << ((8 * AX_U1024_WORD_SIZE) - nbits));
		b->array[i] >>= nbits;
	}

}


void ax_u1024_mod(const ax_u1024* a, const ax_u1024* b, ax_u1024* c)
{
	CHECK_PARAM_NULL(a);
	CHECK_PARAM_NULL(b);
	CHECK_PARAM_NULL(c);

	/*
	 * Take divmod and throw away div part
	 */

	ax_u1024 tmp;

	ax_u1024_divmod(a,b,&tmp,c);
}

void ax_u1024_divmod(const ax_u1024* a, const ax_u1024* b, ax_u1024* c, ax_u1024* d)
{
	CHECK_PARAM_NULL(a);
	CHECK_PARAM_NULL(b);
	CHECK_PARAM_NULL(c);

	/*
	 * Puts a%b in d
	 * and a/b in c
         *
	 * mod(a,b) = a - ((a / b) * b)
         *
	 * example:
	 * mod(8, 3) = 8 - ((8 / 3) * 3) = 2
	 */

	ax_u1024 tmp;

	/* c = (a / b) */
	ax_u1024_div(a, b, c);

	/* tmp = (c * b) */
	ax_u1024_mul(c, b, &tmp);

	/* c = a - tmp */
	ax_u1024_sub(a, &tmp, d);
}

void ax_u1024_and(const ax_u1024* a, const ax_u1024* b, ax_u1024* c)
{
	CHECK_PARAM_NULL(a);
	CHECK_PARAM_NULL(b);
	CHECK_PARAM_NULL(c);

	int i;
	for (i = 0; i < AX_U1024_ARR_LEN; ++i)
		c->array[i] = (a->array[i] & b->array[i]);
}

void ax_u1024_or(const ax_u1024* a, const ax_u1024* b, ax_u1024* c)
{
	CHECK_PARAM_NULL(a);
	CHECK_PARAM_NULL(b);
	CHECK_PARAM_NULL(c);

	for (int i = 0; i < AX_U1024_ARR_LEN; ++i)
		c->array[i] = (a->array[i] | b->array[i]);
}

void ax_u1024_xor(const ax_u1024* a, const ax_u1024* b, ax_u1024* c)
{
	CHECK_PARAM_NULL(a);
	CHECK_PARAM_NULL(b);
	CHECK_PARAM_NULL(c);

	for (int i = 0; i < AX_U1024_ARR_LEN; ++i)
		c->array[i] = (a->array[i] ^ b->array[i]);
}

void ax_u1024_not(const ax_u1024* a, ax_u1024* b)
{
	CHECK_PARAM_NULL(a);
	CHECK_PARAM_NULL(b);

	for (int i = 0; i < AX_U1024_ARR_LEN; ++i)
		b->array[i] = ~a->array[i];
}

int ax_u1024_cmp(const ax_u1024* a, const ax_u1024* b)
{
	CHECK_PARAM_NULL(a);
	CHECK_PARAM_NULL(b);

	int i = AX_U1024_ARR_LEN;
	do {
		i -= 1; /* Decrement first, to start with last array element */
		if (a->array[i] > b->array[i])
			return AX_U1024_LARGER;
		else if (a->array[i] < b->array[i])
			return AX_U1024_SMALLER;
	}
	while (i != 0);

	return AX_U1024_EQUAL;
}

int ax_u1024_is_zero(const ax_u1024* n)
{
	CHECK_PARAM_NULL(n);

	int i;
	for (i = 0; i < AX_U1024_ARR_LEN; ++i)
		if (n->array[i])
			return 0;
	return 1;
}

void ax_u1024_pow(const ax_u1024* a, const ax_u1024* b, ax_u1024* c)
{
	CHECK_PARAM_NULL(a);
	CHECK_PARAM_NULL(b);
	CHECK_PARAM_NULL(c);

	ax_u1024 tmp;

	ax_u1024_init(c);

	if (ax_u1024_cmp(b, c) == AX_U1024_EQUAL) {
		/* Return 1 when exponent is 0 -- n^0 = 1 */
		ax_u1024_inc(c);
	} else {
		ax_u1024 bcopy;
		ax_u1024_assign(&bcopy, b);

		/* Copy a -> tmp */
		ax_u1024_assign(&tmp, a);

		ax_u1024_dec(&bcopy);

		/* Begin summing products: */
		while (!ax_u1024_is_zero(&bcopy)) {

			/* c = tmp * tmp */
			ax_u1024_mul(&tmp, a, c);
			/* Decrement b by one */
			ax_u1024_dec(&bcopy);

			ax_u1024_assign(&tmp, c);
		}

		/* c = tmp */
		ax_u1024_assign(c, &tmp);
	}
}

void ax_u1024_isqrt(const ax_u1024 *a, ax_u1024* b)
{
	CHECK_PARAM_NULL(a);
	CHECK_PARAM_NULL(b);

	ax_u1024 low, high, mid, tmp;

	ax_u1024_init(&low);
	ax_u1024_assign(&high, a);
	ax_u1024_rshift(&high, &mid, 1);
	ax_u1024_inc(&mid);

	while (ax_u1024_cmp(&high, &low) > 0) {
		ax_u1024_mul(&mid, &mid, &tmp);
		if (ax_u1024_cmp(&tmp, a) > 0) {
			ax_u1024_assign(&high, &mid);
			ax_u1024_dec(&high);
		} else {
			ax_u1024_assign(&low, &mid);
		}
		ax_u1024_sub(&high,&low,&mid);
		u1024_rshift_one_bit(&mid);
		ax_u1024_add(&low,&mid,&mid);
		ax_u1024_inc(&mid);
	}
	ax_u1024_assign(b,&low);
}

void ax_u1024_assign(ax_u1024* dst, const ax_u1024* src)
{
	CHECK_PARAM_NULL(dst);
	CHECK_PARAM_NULL(src);

	int i;
	for (i = 0; i < AX_U1024_ARR_LEN; ++i)
		dst->array[i] = src->array[i];
}


static void u1024_rshift_word(ax_u1024* a, int nwords)
{
	int i;
	if (nwords >= AX_U1024_ARR_LEN) {
		for (i = 0; i < AX_U1024_ARR_LEN; ++i)
			a->array[i] = 0;
		return;
	}

	for (i = 0; i < AX_U1024_ARR_LEN - nwords; ++i)
		a->array[i] = a->array[i + nwords];
	for (; i < AX_U1024_ARR_LEN; ++i)
		a->array[i] = 0;
}

static void u1024_lshift_word(ax_u1024* a, int nwords)
{
	int i;
	/* Shift whole words */
	for (i = (AX_U1024_ARR_LEN - 1); i >= nwords; --i)
		a->array[i] = a->array[i - nwords];
	/* Zero pad shifted words. */
	for (; i >= 0; --i)
		a->array[i] = 0;
}

static void u1024_lshift_one_bit(ax_u1024* a)
{
	int i;
	for (i = (AX_U1024_ARR_LEN - 1); i > 0; --i)
		a->array[i] = (a->array[i] << 1) | (a->array[i - 1] >> ((8 * AX_U1024_WORD_SIZE) - 1));
	a->array[0] <<= 1;
}

static void u1024_rshift_one_bit(ax_u1024* a)
{
	int i;
	for (i = 0; i < (AX_U1024_ARR_LEN - 1); ++i)
		a->array[i] = (a->array[i] >> 1) | (a->array[i + 1] << ((8 * AX_U1024_WORD_SIZE) - 1));
	a->array[AX_U1024_ARR_LEN - 1] >>= 1;
}

