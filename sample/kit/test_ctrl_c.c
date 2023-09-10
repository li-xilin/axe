#include <ax/ctrlc.h>
#include <ax/thread.h>
#include <ax/detect.h>
#include <stdio.h>

void handler(int act)
{
	switch (act) {
		case AX_CTRLC_INT:
			printf("Ctrl+C taped\n");
			break;
		case AX_CTRLC_QUIT:
			printf("Ctrl+\\ or Ctrl+Break taped, restore to default procedure\n");
			ax_ctrlc_unset();
			break;
		case AX_CTRLC_CLOSE:
			printf("SIGTERM received or console is closed, quiting\n");
			break;
	}
}
int main()
{
	ax_ctrlc_set(handler);

	for (int i = 0; ; i++) {
		printf("%d\n", i);
		ax_thread_sleep(300);
	}

}

