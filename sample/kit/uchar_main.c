#include <ax/uchar.h>
#include <ax/io.h>

int ax_main(int argc, ax_uchar *argv[])
{
	for (int i = 0; i < argc; i++) {
		ax_printf(ax_u("%s\n"), argv[i]); 
	}
	return 0;
}

