#include "uchar.h"
#include "path.h"
#include "sys.h"
#include "dir.h"
#include "stat.h"
#include <stdio.h>
#include <windows.h>
#include <ax/mem.h>
#include <time.h>

int main()
{
#if 0
	ax_uchar buf[6];
	int ret = ax_ustr_from_ansi(buf, sizeof buf, "我是中国人");
	printf("ret = %d\n", ret);

	MessageBoxW(NULL, buf, NULL, 0);

	char buf1[1000];
	ax_ustr_ansi(buf, buf1, 1000);

	MessageBoxA(NULL, buf1, NULL, 0);
#endif

#if 0
	ax_uchar buf[AX_PATH_MAX] = { 0 };// = L"\\..\\..\\.\\ab\\cd\\.\\..\\";

	// ax_path_getcwd(buf, sizeof buf);
	ax_path_tmpdir(buf, sizeof buf / sizeof(ax_uchar));
	// ax_path_normalize(buf);
	// ax_path_trim(buf);

	
	ax_ustrcpy(buf, ax_u("./"));
	ax_dir *dir = ax_dir_open(buf);
	ax_dirent *d;
	while ((d = ax_dir_read(dir))) {
		wprintf(ax_u("ino = %llu, type = %d, name = %ls\n"), *(uint64_t *)&d->d_ino, d->d_type, d->d_name);
	}
	ax_dir_close(dir);
#endif

	ax_uchar buf[AX_PATH_MAX] = { 0 };
	ax_ustrcpy(buf, ax_u("C:\\windows\\"));

	ax_stat st;
	ax_stat_get(buf, &st);
	printf("time = %s\n", ctime(&st.st_mtim));
	printf("dev = %llu\n", st.st_dev);
	printf("mode = %lo\n", st.st_mode);
	printf("size = %llu\n", st.st_size);

}
