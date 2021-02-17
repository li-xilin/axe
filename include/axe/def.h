#ifndef AXE_DEF_H_
#define AXE_DEF_H_
#include <stdint.h>
#include <stddef.h>

typedef char ax_bool;
#define ax_true 1
#define ax_false 0

inline static ax_bool ax_bool_equal(ax_bool b1, ax_bool b2) { return  !!b1 == !!b2; }

typedef ax_bool ax_fail;

typedef uint8_t ax_byte;

typedef size_t ax_fast_uint;

typedef void (*ax_unary_f)(void *out, const void *in, void *arg);

typedef void (*ax_binary_f)(void *out, const void *in1, const void *in2, void *arg);

#define AX_MIN(a, b) ((a) < (b) ? a : b)
#define AX_MAX(a, b) ((a) > (b) ? a : b)

#endif
