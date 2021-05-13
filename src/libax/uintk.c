/* 
 * https://github.com/kokke/tiny-bignum-c
 * */

#include <ax/uintk.h>
#include <ax/def.h>
#include <stdio.h>

#include "check.h"

static void uintk_rshift_one_bit(ax_uintk* a);
static void uintk_lshift_one_bit(ax_uintk* a);
static void uintk_lshift_word(ax_uintk* a, int nwords);
static void uintk_rshift_word(ax_uintk* a, int nwords);

void ax_uintk_init(ax_uintk* n)
{
	CHECK_PARAM_NULL(n);

	int i;
	for (i = 0; i < AX_UINTK_ARR_LEN; ++i)
		n->array[i] = 0;
}

void ax_uintk_from_int(ax_uintk* n, uint64_t i)
{
	CHECK_PARAM_NULL(n);

	ax_uintk_init(n);
	n->array[0] = i;
	uint64_t num_32 = 32;
	uint64_t tmp = i >> num_32; /* bit-shift with U64 operands to force 64-bit results */
	n->array[1] = tmp;
}

int ax_uintk_to_int(ax_uintk* n)
{
	CHECK_PARAM_NULL(n);

	int ret = 0;
	ret += n->array[0];
	return ret;
}

void ax_uintk_from_string(ax_uintk* n, char* str, int nbytes)
{
	CHECK_PARAM_NULL(n);
	CHECK_PARAM_NULL(str);
	ax_assert(nbytes > 0, "nbytes must be positive");
	ax_assert((nbytes & 1) == 0, "string format must be in hex -> equal number of bytes");
	ax_assert((nbytes % (sizeof(uint32_t) * 2)) == 0, "string length must be a multiple of (sizeof(DTYPE) * 2) characters");
	ax_uintk_init(n);

	uint32_t tmp;
	int i = nbytes - (2 * AX_UINTK_WORD_SIZE); /* index into string */
	int j = 0; /* index into array */

	/* reading last hex-byte "MSB" from string first -> big endian */
	/* MSB ~= most significant byte / block ? :) */
	while (i >= 0) {
		tmp = 0;
		sscanf(&str[i], "%8x", &tmp);
		n->array[j] = tmp;
		i -= (2 * AX_UINTK_WORD_SIZE); /* step AX_UINTK_WORD_SIZE hex-byte(s) back in the string. */
		j += 1;               /* step one element forward in the array. */
	}
}

void ax_uintk_to_string(ax_uintk* n, char* str, int nbytes)
{
	CHECK_PARAM_NULL(n);
	CHECK_PARAM_NULL(str);
	ax_assert(nbytes > 0, "nbytes must be positive");
	ax_assert((nbytes & 1) == 0, "string format must be in hex -> equal number of bytes");

	int j = AX_UINTK_ARR_LEN - 1; /* index into array - reading "MSB" first -> big-endian */
	int i = 0;                 /* index into string representation. */

	/* reading last array-element "MSB" first -> big endian */
	while ((j >= 0) && (nbytes > (i + 1))) {
		sprintf(&str[i], "%.08x", n->array[j]);
		i += (2 * AX_UINTK_WORD_SIZE); /* step AX_UINTK_WORD_SIZE hex-byte(s) forward in the string. */
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

void ax_uintk_dec(ax_uintk* n)
{
	CHECK_PARAM_NULL(n);

	uint32_t tmp; /* copy of n */
	uint32_t res;

	int i;
	for (i = 0; i < AX_UINTK_ARR_LEN; ++i) {
		tmp = n->array[i];
		res = tmp - 1;
		n->array[i] = res;
		if (!(res > tmp))
			break;
	}
}


void ax_uintk_inc(ax_uintk* n)
{
	CHECK_PARAM_NULL(n);

	uint32_t res;
	uint64_t tmp; /* copy of n */

	int i;
	for (i = 0; i < AX_UINTK_ARR_LEN; ++i) {
		tmp = n->array[i];
		res = tmp + 1;
		n->array[i] = res;

		if (res > tmp)
			break;
	}
}


void ax_uintk_add(const ax_uintk* a, const ax_uintk* b, ax_uintk* c)
{
	CHECK_PARAM_NULL(a);
	CHECK_PARAM_NULL(b);
	CHECK_PARAM_NULL(c);

	uint64_t tmp;
	int carry = 0;
	int i;
	for (i = 0; i < AX_UINTK_ARR_LEN; ++i) {
		tmp = (uint64_t)a->array[i] + b->array[i] + carry;
		carry = (tmp > AX_UINTK_MAX);
		c->array[i] = (tmp & AX_UINTK_MAX);
	}
}


void ax_uintk_sub(const ax_uintk* a, const ax_uintk* b, ax_uintk* c)
{
	CHECK_PARAM_NULL(a);
	CHECK_PARAM_NULL(b);
	CHECK_PARAM_NULL(c);

	uint64_t res;
	uint64_t tmp1;
	uint64_t tmp2;
	int borrow = 0;
	int i;
	for (i = 0; i < AX_UINTK_ARR_LEN; ++i) {
		tmp1 = (uint64_t)a->array[i] + (AX_UINTK_MAX + 1); /* + number_base */
		tmp2 = (uint64_t)b->array[i] + borrow;;
		res = (tmp1 - tmp2);
		c->array[i] = (uint32_t)(res & AX_UINTK_MAX); /* "modulo number_base" == "% (number_base - 1)" if number_base is 2^N */
		borrow = (res <= AX_UINTK_MAX);
	}
}


void ax_uintk_mul(const ax_uintk* a, const ax_uintk* b, ax_uintk* c)
{
	CHECK_PARAM_NULL(a);
	CHECK_PARAM_NULL(b);
	CHECK_PARAM_NULL(c);

	ax_uintk row;
	ax_uintk tmp;
	int i, j;

	ax_uintk_init(c);

	for (i = 0; i < AX_UINTK_ARR_LEN; ++i) {
		ax_uintk_init(&row);
		for (j = 0; j < AX_UINTK_ARR_LEN; ++j) {
			if (i + j < AX_UINTK_ARR_LEN) {
				ax_uintk_init(&tmp);
				uint64_t intermediate = ((uint64_t)a->array[i] * (uint64_t)b->array[j]);
				ax_uintk_from_int(&tmp, intermediate);
				uintk_lshift_word(&tmp, i + j);
				ax_uintk_add(&tmp, &row, &row);
			}
		}
		ax_uintk_add(c, &row, c);
	}
}


void ax_uintk_div(const ax_uintk* a, const ax_uintk* b, ax_uintk* c)
{
	CHECK_PARAM_NULL(a);
	CHECK_PARAM_NULL(b);
	CHECK_PARAM_NULL(c);

	ax_uintk current;
	ax_uintk denom;
	ax_uintk tmp;

	ax_uintk_from_int(&current, 1);                            // int current = 1;
	ax_uintk_assign(&denom, b);                                // denom = b
	ax_uintk_assign(&tmp, a);                                  // tmp   = a

	const uint64_t half_max = 1 + (uint64_t)(AX_UINTK_MAX / 2);
	bool overflow = false;
	while (ax_uintk_cmp(&denom, a) != AX_UINTK_LARGER) {        // while (denom <= a) {
		if (denom.array[AX_UINTK_ARR_LEN - 1] >= half_max) {
			overflow = true;
			break;
		}
		uintk_lshift_one_bit(&current);                     //   current <<= 1;
		uintk_lshift_one_bit(&denom);                       //   denom <<= 1;
	}
	if (!overflow) {
		uintk_rshift_one_bit(&denom);                       // denom >>= 1;
		uintk_rshift_one_bit(&current);                     // current >>= 1;
	}
	ax_uintk_init(c);                                           // int answer = 0;

	while (!ax_uintk_is_zero(&current)) {                       // while (current != 0)
		if (ax_uintk_cmp(&tmp, &denom) != AX_UINTK_SMALLER) { //   if (dividend >= denom)
			ax_uintk_sub(&tmp, &denom, &tmp);           //     dividend -= denom;
			ax_uintk_or(c, &current, c);                //     answer |= current;
		}
		uintk_rshift_one_bit(&current);                     //   current >>= 1;
		uintk_rshift_one_bit(&denom);                       //   denom >>= 1;
	}                                                           // return answer;
}


void ax_uintk__ax_lshift(const ax_uintk* a, ax_uintk* b, int nbits)
{
	CHECK_PARAM_NULL(a);
	CHECK_PARAM_NULL(b);
	ax_assert(nbits >= 0, "no negative shifts");

	ax_uintk_assign(b, a);
	/* Handle shift in multiples of word-size */
	const int nbits_pr_word = (AX_UINTK_WORD_SIZE * 8);
	int nwords = nbits / nbits_pr_word;
	if (nwords != 0) {
		uintk_lshift_word(b, nwords);
		nbits -= (nwords * nbits_pr_word);
	}

	if (nbits != 0) {
		int i;
		for (i = (AX_UINTK_ARR_LEN - 1); i > 0; --i)
			b->array[i] = (b->array[i] << nbits) | (b->array[i - 1] >> ((8 * AX_UINTK_WORD_SIZE) - nbits));
		b->array[i] <<= nbits;
	}
}


void ax_uintk__ax_rshift(const ax_uintk* a, ax_uintk* b, int nbits)
{
	CHECK_PARAM_NULL(a);
	CHECK_PARAM_NULL(b);
	ax_assert(nbits >= 0, "no negative shifts");

	ax_uintk_assign(b, a);
	/* Handle shift in multiples of word-size */
	const int nbits_pr_word = (AX_UINTK_WORD_SIZE * 8);
	int nwords = nbits / nbits_pr_word;
	if (nwords != 0) {
		uintk_rshift_word(b, nwords);
		nbits -= (nwords * nbits_pr_word);
	}

	if (nbits != 0) {
		int i;
		for (i = 0; i < (AX_UINTK_ARR_LEN - 1); ++i)
			b->array[i] = (b->array[i] >> nbits) | (b->array[i + 1] << ((8 * AX_UINTK_WORD_SIZE) - nbits));
		b->array[i] >>= nbits;
	}

}


void ax_uintk_mod(const ax_uintk* a, const ax_uintk* b, ax_uintk* c)
{
	CHECK_PARAM_NULL(a);
	CHECK_PARAM_NULL(b);
	CHECK_PARAM_NULL(c);

	/*
	 * Take divmod and throw away div part
	 */

	ax_uintk tmp;

	ax_uintk_divmod(a,b,&tmp,c);
}

void ax_uintk_divmod(const ax_uintk* a, const ax_uintk* b, ax_uintk* c, ax_uintk* d)
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

	ax_uintk tmp;

	/* c = (a / b) */
	ax_uintk_div(a, b, c);

	/* tmp = (c * b) */
	ax_uintk_mul(c, b, &tmp);

	/* c = a - tmp */
	ax_uintk_sub(a, &tmp, d);
}


void ax_uintk_and(const ax_uintk* a, const ax_uintk* b, ax_uintk* c)
{
	CHECK_PARAM_NULL(a);
	CHECK_PARAM_NULL(b);
	CHECK_PARAM_NULL(c);

	int i;
	for (i = 0; i < AX_UINTK_ARR_LEN; ++i)
		c->array[i] = (a->array[i] & b->array[i]);
}


void ax_uintk_or(const ax_uintk* a, const ax_uintk* b, ax_uintk* c)
{
	CHECK_PARAM_NULL(a);
	CHECK_PARAM_NULL(b);
	CHECK_PARAM_NULL(c);

	int i;
	for (i = 0; i < AX_UINTK_ARR_LEN; ++i)
		c->array[i] = (a->array[i] | b->array[i]);
}


void ax_uintk_xor(const ax_uintk* a, const ax_uintk* b, ax_uintk* c)
{
	CHECK_PARAM_NULL(a);
	CHECK_PARAM_NULL(b);
	CHECK_PARAM_NULL(c);

	int i;
	for (i = 0; i < AX_UINTK_ARR_LEN; ++i)
		c->array[i] = (a->array[i] ^ b->array[i]);
}


int ax_uintk_cmp(const ax_uintk* a, const ax_uintk* b)
{
	CHECK_PARAM_NULL(a);
	CHECK_PARAM_NULL(b);

	int i = AX_UINTK_ARR_LEN;
	do {
		i -= 1; /* Decrement first, to start with last array element */
		if (a->array[i] > b->array[i])
			return AX_UINTK_LARGER;
		else if (a->array[i] < b->array[i])
			return AX_UINTK_SMALLER;
	}
	while (i != 0);

	return AX_UINTK_EQUAL;
}


int ax_uintk_is_zero(const ax_uintk* n)
{
	CHECK_PARAM_NULL(n);

	int i;
	for (i = 0; i < AX_UINTK_ARR_LEN; ++i)
		if (n->array[i])
			return 0;
	return 1;
}


void ax_uintk_pow(const ax_uintk* a, const ax_uintk* b, ax_uintk* c)
{
	CHECK_PARAM_NULL(a);
	CHECK_PARAM_NULL(b);
	CHECK_PARAM_NULL(c);

	ax_uintk tmp;

	ax_uintk_init(c);

	if (ax_uintk_cmp(b, c) == AX_UINTK_EQUAL) {
		/* Return 1 when exponent is 0 -- n^0 = 1 */
		ax_uintk_inc(c);
	} else {
		ax_uintk bcopy;
		ax_uintk_assign(&bcopy, b);

		/* Copy a -> tmp */
		ax_uintk_assign(&tmp, a);

		ax_uintk_dec(&bcopy);

		/* Begin summing products: */
		while (!ax_uintk_is_zero(&bcopy)) {

			/* c = tmp * tmp */
			ax_uintk_mul(&tmp, a, c);
			/* Decrement b by one */
			ax_uintk_dec(&bcopy);

			ax_uintk_assign(&tmp, c);
		}

		/* c = tmp */
		ax_uintk_assign(c, &tmp);
	}
}

void ax_uintk_isqrt(const ax_uintk *a, ax_uintk* b)
{
	CHECK_PARAM_NULL(a);
	CHECK_PARAM_NULL(b);

	ax_uintk low, high, mid, tmp;

	ax_uintk_init(&low);
	ax_uintk_assign(&high, a);
	ax_uintk__ax_rshift(&high, &mid, 1);
	ax_uintk_inc(&mid);

	while (ax_uintk_cmp(&high, &low) > 0) {
		ax_uintk_mul(&mid, &mid, &tmp);
		if (ax_uintk_cmp(&tmp, a) > 0) {
			ax_uintk_assign(&high, &mid);
			ax_uintk_dec(&high);
		} else {
			ax_uintk_assign(&low, &mid);
		}
		ax_uintk_sub(&high,&low,&mid);
		uintk_rshift_one_bit(&mid);
		ax_uintk_add(&low,&mid,&mid);
		ax_uintk_inc(&mid);
	}
	ax_uintk_assign(b,&low);
}


void ax_uintk_assign(ax_uintk* dst, const ax_uintk* src)
{
	CHECK_PARAM_NULL(dst);
	CHECK_PARAM_NULL(src);

	int i;
	for (i = 0; i < AX_UINTK_ARR_LEN; ++i)
		dst->array[i] = src->array[i];
}


static void uintk_rshift_word(ax_uintk* a, int nwords)
{
	int i;
	if (nwords >= AX_UINTK_ARR_LEN) {
		for (i = 0; i < AX_UINTK_ARR_LEN; ++i)
			a->array[i] = 0;
		return;
	}

	for (i = 0; i < AX_UINTK_ARR_LEN - nwords; ++i)
		a->array[i] = a->array[i + nwords];
	for (; i < AX_UINTK_ARR_LEN; ++i)
		a->array[i] = 0;
}


static void uintk_lshift_word(ax_uintk* a, int nwords)
{
	int i;
	/* Shift whole words */
	for (i = (AX_UINTK_ARR_LEN - 1); i >= nwords; --i)
		a->array[i] = a->array[i - nwords];
	/* Zero pad shifted words. */
	for (; i >= 0; --i)
		a->array[i] = 0;
}


static void uintk_lshift_one_bit(ax_uintk* a)
{
	int i;
	for (i = (AX_UINTK_ARR_LEN - 1); i > 0; --i)
		a->array[i] = (a->array[i] << 1) | (a->array[i - 1] >> ((8 * AX_UINTK_WORD_SIZE) - 1));
	a->array[0] <<= 1;
}


static void uintk_rshift_one_bit(ax_uintk* a)
{
	int i;
	for (i = 0; i < (AX_UINTK_ARR_LEN - 1); ++i)
		a->array[i] = (a->array[i] >> 1) | (a->array[i + 1] << ((8 * AX_UINTK_WORD_SIZE) - 1));
	a->array[AX_UINTK_ARR_LEN - 1] >>= 1;
}

