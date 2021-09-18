/*
 * Copyright (c) 2021 Li hsilin <lihsilyn@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "assist.h"

#include "ax/base.h"
#include "ax/class.h"
#include "ax/stuff.h"

#include "axut.h"
#include <stdio.h>

static void stoi(axut_runner *r)
{
	static const struct { const char *name; int value; } table[] = {
		{ "",   	AX_ST_NIL },
		{ "?",  	0         },
		{ "c",  	AX_ST_C   },
		{ "f",  	AX_ST_F   },
		{ "h",  	AX_ST_H   },
		{ "i",  	AX_ST_I   },
		{ "i16",	AX_ST_I16 },
		{ "i32",	AX_ST_I32 },
		{ "i64",	AX_ST_I64 },
		{ "i8", 	AX_ST_I8  },
		{ "l",  	AX_ST_L   },
		{ "lf", 	AX_ST_LF  },
		{ "ll", 	AX_ST_LL  },
		{ "p",  	AX_ST_PTR },
		{ "s",  	AX_ST_S   },
		{ "u",  	AX_ST_U   },
		{ "u16",	AX_ST_U16 },
		{ "u32",	AX_ST_U32 },
		{ "u64",	AX_ST_U64 },
		{ "u8", 	AX_ST_U8  },
		{ "uc", 	AX_ST_UC  },
		{ "uh", 	AX_ST_UH  },
		{ "ul", 	AX_ST_UL  },
		{ "ull",	AX_ST_ULL },
		{ "ws", 	AX_ST_WS  },
		{ "z",  	AX_ST_Z   },
	};
	for (int i = 0; i < sizeof table/ sizeof *table; i++) {
		axut_assert(r, table[i].value == ax_stuff_stoi(table[i].name));
	}
}

axut_suite *suite_for_stuff(ax_base *base)
{
	axut_suite* suite = axut_suite_create(ax_base_local(base), "stuff");
	axut_suite_add(suite, stoi, 0);
	return suite;
}
