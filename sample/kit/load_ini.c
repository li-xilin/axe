#include <ax/io.h>
#include <ax/ini.h>
#include <stdio.h>

int err_cb(unsigned line, unsigned err, void *args)
{
	ax_fprintf(stderr, ax_u("error: line = %d, err = %d\n"), line, err);
	return 0;
}
int main()
{
	FILE *fp = ax_fopen(ax_u("example.ini"), ax_u("r"));
	ax_ini *d = ax_ini_load(fp, err_cb, NULL);

	puts("\nFind option:");
	char buf[1024];
	ax_ustr_ansi(ax_ini_get(d, ax_u("Pizza"), ax_u("Ham")), buf, sizeof buf);
	printf("Pizza:Ham -> %s\n", buf);
	ax_ustr_ansi(ax_ini_get(d, ax_u("wine"), ax_u("grape")), buf, sizeof buf);
	printf("wine:grape -> %s\n", buf);

	ax_ustr_ansi(ax_ini_get_str(d, ax_u("wine:grape"), ax_u("default")), buf, sizeof buf);
	printf("wine:grape -> %s\n", buf);

	puts("\nAdd option:");
	ax_ini_set(d, ax_u("Foo"), ax_u("Bar"), ax_u("42"));
	ax_ustr_ansi(ax_ini_get(d, ax_u("Foo"), ax_u("Bar")), buf, sizeof buf);
	printf("Foo:Bar -> %s\n", buf);


	puts("\nDump example.ini:");
	ax_ini_dump(d, stdout);

}
