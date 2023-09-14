#include <stdio.h>
#include <ax/thread.h>

int main(int argc, char *argv[])
{
	int c;
	setvbuf(stdout, 0, _IOLBF, 0);

	for (int i = 0; argv[i]; i++)
		fprintf(stdout, "%s\n", argv[i]);

	while ((c = getchar()) != EOF) {
		ax_thread_sleep(100);
		fputc(c, stdout);
	}
	return 42;
}
