#ifndef AX_LIB_H
#define AX_LIB_H
#include <stdint.h>

#ifndef AX_LIB_DEFINED
#define AX_LIB_DEFINED
typedef struct ax_lib_st ax_lib;
#endif

ax_lib *ax_lib_open(const char* fname);

void *ax_lib_symbol(ax_lib *lib, const char *symb);

int ax_lib_close(ax_lib *lib);

const char *ax_lib_error(void);

#endif
