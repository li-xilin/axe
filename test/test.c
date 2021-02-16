#include <axut.h>
#include <axe/base.h>
#include <stdio.h>

extern axut_suite *suite_for_hashmap(ax_base *base);
extern axut_suite *suite_for_vector(ax_base *base);
extern axut_suite *suite_for_string(ax_base *base);
extern axut_suite *suite_for_scope(ax_base *base);
extern axut_suite *suite_for_algo(ax_base *base);
extern axut_suite *suite_for_vail(ax_base *base);
extern axut_suite *suite_for_avltree(ax_base *base);
extern axut_suite *suite_for_pool(ax_base *base);
extern axut_suite *suite_for_list(ax_base *base);
extern axut_suite *suite_for_pred(ax_base *base);
extern axut_suite *suite_for_uintk(ax_base *base);

int main()
{
	ax_base *base = ax_base_create();

	axut_runner *r = axut_runner_create(ax_base_local(base), NULL);
	axut_runner_add(r, suite_for_hashmap(base));
	axut_runner_add(r, suite_for_vector(base));
	axut_runner_add(r, suite_for_string(base));
	axut_runner_add(r, suite_for_scope(base));
	axut_runner_add(r, suite_for_algo(base));
	axut_runner_add(r, suite_for_avltree(base));
	axut_runner_add(r, suite_for_pool(base));
	axut_runner_add(r, suite_for_list(base));
	axut_runner_add(r, suite_for_pred(base));
	axut_runner_add(r, suite_for_uintk(base));

	axut_runner_run(r);

	fputs(axut_runner_result(r), stdout);

	ax_base_destroy(base);
}

