#include <ax/class.h>
#include <ax/type/any.h>
#include <ax/dump.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* -- Interface sharp base on any -- */
#define ax_baseof_sharp any

typedef struct ax_sharp_st ax_sharp;

ax_begin_env(sharp)
ax_end;

ax_begin_trait(sharp)
	float (*area)(const struct ax_sharp_st *sharp);
	float (*side_length)(const struct ax_sharp_st *sharp);
ax_end;

ax_abstract(2, sharp);

inline static float ax_sharp_area(const struct ax_sharp_st *sharp)
{
	return sharp->tr->area(sharp);
}

inline static float ax_sharp_side_length(const struct ax_sharp_st *sharp)
{
	return sharp->tr->side_length(sharp);
}

/* -- Class base on sharp -- */

#define ax_baseof_circle sharp

ax_begin_data(circle)
	float radius;
ax_end;

ax_concrete(3, circle);

static void one_free(ax_one *one);
static ax_dump *any_dump(const ax_any *any);
static ax_any *any_copy(const ax_any *any);
static float sharp_area(const struct ax_sharp_st *sharp);
static float sharp_side_length(const struct ax_sharp_st *sharp);
static void ax_circle_set_radius(struct ax_circle_st *circle, float radius);
static float ax_circle_radius(const struct ax_circle_st *circle);
inline static ax_class_constructor0(circle);
inline static ax_class_constructor(circle, int radius);

/* Implementation */

static void one_free(ax_one *one)
{
	free(one);
}

static const char *one_name(const ax_one *one)
{
	return ax_class_name(3, circle);
}

static ax_dump *any_dump(const ax_any *any)
{
	ax_circle_cr r = { .any = any };
	ax_dump *blk = ax_dump_block(ax_one_name(r.one), 3);
	ax_dump_bind(blk, 0, ax_dump_pair(
				ax_dump_symbol("Radius"),
				ax_dump_float(ax_circle_radius(r.circle))));
	ax_dump_bind(blk, 1, ax_dump_pair(
				ax_dump_symbol("Area"),
				ax_dump_float(ax_sharp_area(r.sharp))));
	ax_dump_bind(blk, 2, ax_dump_pair(
				ax_dump_symbol("SideLen"),
				ax_dump_float(ax_sharp_side_length(r.sharp))));
	return blk;
}

static ax_any *any_copy(const ax_any *any)
{
	ax_circle_cr src = { .any = any };
	return ax_new(circle, src.circle->radius).any;
}

static float sharp_area(const struct ax_sharp_st *sharp)
{
	ax_circle_cr cir = { .sharp = sharp };
	return 3.14159 * pow(cir.circle->radius, 2);
}

static float sharp_side_length(const struct ax_sharp_st *sharp)
{
	ax_circle_cr cir = { .sharp = sharp };
	return 2 * 3.14159 * cir.circle->radius;
}

static void ax_circle_set_radius(struct ax_circle_st *circle, float radius)
{
	circle->radius = radius;
}

static float ax_circle_radius(const struct ax_circle_st *circle)
{
	return circle->radius;
}

static const ax_sharp_trait ax_circle_tr = {
	.any = {
		.one.name = one_name,
		.one.free = one_free,
		.dump = any_dump,
		.copy = any_copy,
	},
	.area = sharp_area,
	.side_length = sharp_side_length,
};

inline static ax_class_constructor(circle, int radius)
{
	ax_circle_r circ = ax_new0(circle);
	ax_circle_set_radius(circ.circle, radius);
	return circ.sharp;
}

inline static ax_class_constructor0(circle)
{
	ax_circle_r circ = { malloc(sizeof(struct ax_circle_st)) };
	const struct ax_sharp_st init = {
		.tr = &ax_circle_tr,
	};
	memcpy(circ.sharp, &init, sizeof init);
	if (!circ.one)
		return NULL;
	return circ.sharp;
}

/* -- Client -- */

int main()
{
	ax_circle_r cir1 = ax_new(circle, 2);
	printf("Create cir1 using class circle, radius is 2\n");
	printf("Name of cir1: %s\n", ax_one_name(cir1.one));
	printf("Area of cir1: %f\n", ax_sharp_area(cir1.sharp));
	printf("Side length of cir1: %f\n", ax_sharp_side_length(cir1.sharp));

	ax_circle_r cir2 = { .any = ax_any_copy(cir1.any) };
	printf("Copy cir1 to cir2\n");
	printf("Radius of cir2: %f\n", ax_circle_radius(cir2.circle));

	ax_circle_r cir3 = ax_new0(circle);
	printf("Create cir3 using class circle, with no parameter\n");
	ax_circle_set_radius(cir3.circle, 5);
	ax_any_so(cir3.any); /* Dump out */

	ax_one_free(cir1.one);
	ax_one_free(cir2.one);
	ax_one_free(cir3.one);
	return 0;
}
