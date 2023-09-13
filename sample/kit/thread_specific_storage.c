#include "ax/tss.h"
#include "ax/thread.h"
#include <stdio.h>

static ax_tss key;

static intptr_t main_thread_proc(void *arg)
{
	ax_tss_set(&key, malloc(4));
	fprintf(stderr, "thread0 key = %p\n", ax_tss_get(&key));
	ax_thread_sleep(100);
	return 0;
}

static uintptr_t thread1_proc(void *arg)
{
	ax_tss_set(&key, malloc(4));
	fprintf(stderr, "thread1 key = %p\n", ax_tss_get(&key));
	ax_thread_sleep(100);
	return 0;
}

static uintptr_t thread2_proc(void *arg)
{
	ax_tss_set(&key, malloc(4));
	fprintf(stderr, "thread2 key = %p\n", ax_tss_get(&key));
	return 0;
}

static void tss_free(void *value)
{
	fprintf(stderr, "free %p\n", value);
	free(value);
}

int main()
{
	ax_thread thread1, thread2;

	ax_tss_create(&key, tss_free);

	ax_thread_create(thread1_proc, NULL, &thread1);
	ax_thread_create(thread2_proc, NULL, &thread2);
	main_thread_proc(NULL);

	ax_thread_join(&thread1, NULL);
	ax_thread_join(&thread2, NULL);

	ax_tss_remove(&key);

	fprintf(stderr, "after removed, key = %p\n", ax_tss_get(&key));

	fflush(stderr);
}

