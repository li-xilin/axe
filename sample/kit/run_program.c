#include <ax/detect.h>
#include <ax/stdio.h>
#include <inttypes.h>
#include <ax/proc.h>
#include <ax/errno.h>
#include <ax/def.h>
#include <stdlib.h>

#ifdef AX_OS_WIN
#define TEST_EXEC ".\\show_argv.exe"
#else
#define TEST_EXEC "./show_argv.out"
#endif

int main()
{
	ax_uchar *const argv[] = {
		ax_u("/path/to/program"),
		ax_u("-file"),
		ax_u("Picture 1.jpg"),
		ax_u("-char"),
		ax_u("\""),
		ax_u("-text"),
		ax_u("I'll be in touch."),
		ax_u("-empty"),
		ax_u(""),
		NULL };
	ax_proc *p = ax_proc_open(ax_u(TEST_EXEC), argv);
	if (!p) {
		ax_uchar buf[AX_ERRBUF_MAX];
		ax_printf(ax_u("%s\n"), ax_strerror(buf));
		exit(1);
	}
	FILE *in = ax_proc_stdio(p, 0);
	fputs("First line to stdin.\n", in);
	fputs("Second line to stdin.\n", in);
	fputs("Third line to stdin.\n", in);
	ax_proc_fclose(p, 0);
	
	FILE *out = ax_proc_stdio(p, 1);
	char buf[1024];
	while (fgets(buf, sizeof buf, out)) {
		fprintf(stdout, "fgets: %s", buf);
	}

	int ret = ax_proc_close(p);
	ax_printf(ax_u("ret = %d, err = %d\n"), ret, errno);

	return 0;
}

