# Axe #

Axe 是一个容器库，他包含了通用容器和算法，可以有效简化对数据的封装。

该工程包括两个模块：AX 和 AXUT

AX包含容器、算法和迭代器等基本模块，头文件包括：

| 名称          | 描述 |
|---            |---   |
| ax/def.h      | 基础类型定义和宏定义 |
| ax/flow.h     | 程序流程控制结构 |
| ax/narg.h     | 用于参数个数计算的辅助宏 |
| ax/debug.h    | 可打印消息说明的断言函数 |
| ax/arraya.h   | 匿名数组 |
| ax/oper.h     | C语言运算符的函数封装 |
| ax/dump.h     | 容器的可视化转储，用于调试 |
| ax/log.h      | 日志打印 |
| ax/pred.h     | 谓词对象创建和参数绑定 |
| ax/arch.h     | 机器架构宏定义 |
| ax/trait.h    | 类型特性，用于定义容器中元素支持的操作 |
| ax/iter.h     | 迭代器操作 |
| ax/algo.h     | 一些基于迭代器的算法 |
| ax/mem.h      | 内存和串的操作 |
| ax/uintk.h    | 1024位无符号整数操作 |
| ax/one.h      | 根类型 |
| ax/any.h      | 可序列化对象抽象 |
| ax/box.h      | 可迭代容器抽象 |
| ax/seq.h      | 线性表抽象 |
| ax/str.h      | 字符串抽象 |
| ax/map.h      | 映射表抽象 |
| ax/trie.h     | 字典树抽象 |
| ax/tube.h     | 单进单出管道抽象 |
| ax/arr.h      | 静态数组的封装 |
| ax/vector.h   | 向量标，支持随机访问，并自动管理内存 |
| ax/deq.h      | 双端队列，可以从两端压入和弹出元素，支持随机访问 |
| ax/list.h     | 双链表，支持快速插入、移除元素 |
| ax/hmap.h     | 散列表，一种无序的映射表，常数时间的增删查和更新操作 |
| ax/avl.h      | 自平衡树，一种有序的映射表，对数时间的增删查和更新操作 |
| ax/rb.h       | 红黑树，与自平衡树类似，但是元素查找操作效率更高 |
| ax/string.h   | 字符串 |
| ax/btrie.h    | 平衡字典树 |
| ax/queue.h    | 队列，一种先进先出容器 |
| ax/stack.h    | 栈，一种后进先出容器 |
| ax/pque.h     | 优先队列，根据元素优先级弹出元素的特殊队列容器 |

AXUT 用于单元测试，头文件包括：

| 名称          | 描述 |
|---            |---   |
| axut/case.h   | 测试用例结构定义 |
| axut/suite.h  | 测试用例集合 |
| axut/runner.h | 测试用例的执行和结果统计 |

下面是一个简单的例子

```c
#include "ax/algo.h" /* 引入算法函数 */
#include "ax/list.h" /* 引入双连表 */
#include "ax/ptra.h" /* 引入自动指针 */
#include <stdlib.h>
#include <stdio.h>

/* 定义一个一元算子函数 */
void oper_rmodd(void *out, const void *in, void *args)
{
	const int *num = in;
	int *ret = out;
	if (*num % 2 == 1) /* 如果输入为基数，则输出为0 */
		*ret = 0;
}

int main(void)
{
	/* 定义一个双链表 */
	ax_list_r l = ax_new(list, ax_t(int));

	int *count = malloc(sizeof *count);

	/* 将双链表指针和堆变量加入范围块，当范围执行结束后，链表自动被释放，可放置多个指针 */
	ax_scope(l.one, ax_onelize(count)) {

		/* 对区间[1, 11)进行循环迭代 */
		ax_forrange(1, 11)
			/* 将区间每个元素逐个压入链表 */
			ax_seq_push(l.seq, &_);

		/* 对双链表进行DUMP */
		ax_any_so(l.any);
		// OUTPUT: string.c:12:one.any.box.seq.list {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}

		/* 创建一个一元谓词 */
		ax_pred rmodd = ax_pred_unary_make(oper_rmodd, NULL, NULL);
		/* 从头迭代器所在元素开始逐个执行谓词，将所有奇数元素设为0 */
		ax_transform(
				ax_p(ax_citer, ax_box_cbegin(l.box)),
				ax_p(ax_citer, ax_box_cend(l.box)),
				ax_p(ax_iter, ax_box_begin(l.box)),
				&rmodd);

		/* 对双链表进行DUMP */
		ax_any_so(l.any);
		// OUTPUT: string.c:33:one.any.box.seq.list {0, 2, 0, 4, 0, 6, 0, 8, 0, 10}

		/* 按顺序枚举链表所有元素，求和 */
		ax_box_foreach(l.box, int *, i)
			*count += *i;

		printf("Sum of elements = %d\n", *count);
		// OUTPUT: Sum of elements = 30
	}

	return 0;
}
```
