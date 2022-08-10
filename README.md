![AXE](./logo.png)

![](https://img.shields.io/badge/build-passing-green) ![](https://img.shields.io/badge/license-MIT-red) ![](https://img.shields.io/badge/language-C-purple)

---

AXE is the light-weight and low-level library. It provides data structure handling and object system for C.

AXE是一个轻量级的低级程序库，它为C语言提供了数据结构处理能力和对象系统.

## COMPILE AND INSTALL

该函数库可以使用 *make*(1)编译和安装，使用`make debug`将函数库编译为调试版本，安装程序会将头文件、库文件和手册页文件正确复制到系统的特定目录中.

```
$ cd axe
$ make
$ sudo make install
```

## HOW TO USE

您可以直接通过引入相关头文件来使用它们的功能，下面是一个样例程序，这其中包括一些有趣的特性，比如链表、迭代器算法、RAII、对象转储和容器遍历等操作. 在编译客户程序时需要通过加入编译参数`-lax`或`-laxut`来连接相关的库文件. 详细的帮助文档请参考MAN手册页，它们位于工程目录的 *man/man3* 目录下，或参考单元测试程序和附带的样例程序，它们分别位于工程目录的 *test* 和 *sample* 目录. 

```c
/* gcc foo.c -lax */
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
        ax_list_r l = ax_new(ax_list, ax_t(int));

        int *count = malloc(sizeof *count);

        /* 将双链表指针和堆变量加入范围块，当范围执行结束后，链表自动被释放，可放置多个指针 */
        ax_scope(l.ax_one, ax_onelize(count)) {

                /* 对区间[1, 11)进行循环迭代 */
                ax_forrange(1, 11)
                        /* 将区间每个元素逐个压入链表 */
                        ax_seq_push(l.ax_seq, &_);

                /* 对双链表进行转储 */
                ax_any_so(l.ax_any);
                // OUTPUT: string.c:12:one.any.box.seq.list {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}

                /* 创建一个一元谓词 */
                ax_pred rmodd = ax_pred_unary_make(oper_rmodd, NULL, NULL);
                /* 从头迭代器所在元素开始逐个执行谓词，将所有奇数元素设为0 */
                ax_transform(
                                ax_p(ax_citer, ax_box_cbegin(l.ax_box)),
                                ax_p(ax_citer, ax_box_cend(l.ax_box)),
                                ax_p(ax_iter, ax_box_begin(l.ax_box)),
                                &rmodd);

                /* 对双链表进行DUMP */
                ax_any_so(l.ax_any);
                // OUTPUT: string.c:33:one.any.box.seq.list {0, 2, 0, 4, 0, 6, 0, 8, 0, 10}

                /* 按顺序枚举链表所有元素，求和 */
                ax_box_foreach(l.ax_box, int *, i)
                        *count += *i;

                printf("Sum of elements = %d\n", *count);
                // OUTPUT: Sum of elements = 30
        }

        return 0;
}
```

## HEADERS DESCRIPTION

| 名称          | 描述 |
|---            |---   |
| ax/type/one.h  | 根类型 |
| ax/type/any.h  | 可序列化对象抽象 |
| ax/type/box.h  | 可迭代容器抽象 |
| ax/type/seq.h  | 线性表抽象 |
| ax/type/str.h  | 字符串抽象 |
| ax/type/map.h  | 映射表抽象 |
| ax/type/trie.h | 字典树抽象 |
| ax/type/tube.h | 单进单出管道抽象 |
| ax/def.h       | 基础类型定义 |
| ax/flow.h      | 程序流程控制 |
| ax/trick.h     | 低级辅助宏，用于宏定义 |
| ax/narg.h      | 参数个数计算的辅助宏 |
| ax/sys.h       | 操作系统探测宏 |
| ax/arch.h      | 硬件架构探测宏 |
| ax/debug.h     | 断言 |
| ax/arraya.h    | 自动数组 |
| ax/oper.h      | 算子，包括C语言运算符的函数化包装 |
| ax/dump.h      | 容器内容的可视化转储 |
| ax/log.h       | 日志打印 |
| ax/pred.h      | 谓词和参数绑定 |
| ax/trait.h     | 类型特性，对数据类型的描述 |
| ax/iter.h      | 迭代器封装 |
| ax/algo.h      | 基于迭代器的算法 |
| ax/mem.h       | 内存和串的操作 |
| ax/uintk.h     | 1024位无符号整数操作 |
| ax/arr.h       | 无内存分配的数组实现 |
| ax/vector.h    | 向量表实现 |
| ax/deq.h       | 双端队列实现 |
| ax/list.h      | 双链表实现 |
| ax/hmap.h      | 散列表实现 |
| ax/avl.h       | 自平衡树实现 |
| ax/rb.h        | 红黑树实现 |
| ax/string.h    | 字符串实现 |
| ax/btrie.h     | 平衡字典树实现 |
| ax/queue.h     | 队列实现 |
| ax/stack.h     | 栈实现 |
| ax/pque.h      | 优先队列实现 |
| axut/case.h    | 测试用例结构定义 |
| axut/suite.h   | 测试用例集合 |
| axut/runner.h  | 执行和统计测试用例 |

## LICENSE

该软件程序基于MIT协议发布.

## AUTHOR

李希林 <lihsilyn@gmail.com>

