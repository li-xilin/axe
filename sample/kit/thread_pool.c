#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include <ax/mem.h>
#include "ax/thread.h"
#include "ax/tpool.h"


void worker(void *arg)
{
	int *val = arg;
	int  old = *val;

	*val *= -1;
	ax_thread self = ax_thread_self();
	printf("hash(thread)=%zx old=%d, val=%d\n",ax_memhash(&self, sizeof self) , old, *val);
	fflush(stdout);

	ax_thread_sleep(100);
}

int main(int argc, char **argv)
{
	ax_tpool *tm;
	size_t   i;

	static const size_t num_threads = 5;
	static const size_t num_items   = 40;

	tm   = ax_tpool_create(num_threads);
	int vals[num_items];

	for (i=0; i<num_items; i++) {
		vals[i] = i;
		ax_tpool_add_work(tm, worker, vals+i);
	}

	ax_tpool_wait(tm);

	for (i=0; i<num_items; i++) {
		printf("%d ", vals[i]);
		fflush(stdout);
	}
	putchar('\n');

	ax_tpool_destroy(tm);
	return 0;
}
