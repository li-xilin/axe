![AXE](./logo.png)

![](https://img.shields.io/badge/build-passing-green) ![](https://img.shields.io/badge/license-MIT-red) ![](https://img.shields.io/badge/language-C-purple)

---

AXE是一个轻量级的跨平台程序库，它为C语言提供了常用数据结构操作、对象系统、网络支持、系统接口封装和简单的用户界面.

## COMPILE AND INSTALL

该库在编译之前需要执行*configure*脚本进行配置，然后通过*make*(1)编译和安装.

编译结束后，在*lib*目录会生成一些库文件，包括

* *libaxcore.a* 核心模块（只依赖C99标准）
* *libaxut.a* 单元测试模块
* *libaxnet.a* 网络统信模块
* *libaxkit.a* 依赖操作系统系的工具
* *libaxgui.a* 图形用户界面

在这些模块中，除了 `axgui` 需要依赖 `libui` 以外，其他模块不依赖任何第三方程序，其中`axcore`使用纯C99实现，并作为其他几个模块的基础依赖。
开发者可以选择性的引用指定的模块，通过执行`configure --help`获取更多配置信息.

```
$ cd axe
$ ./configure
$ make
$ sudo make install
```

## HOW TO USE

您可以直接通过引入相关头文件来使用它们的功能，下面是一个样例程序，这其中包括一些有趣的特性，比如链表、迭代器算法、RAII、对象转储和容器遍历等操作. 在编译客户程序时需要通过加入编译参数`-lax*`来连接相关的库文件. 详细的帮助文档请参考MAN手册页，它们位于工程目录的 *man/man3* 目录下，或参考单元测试程序和附带的样例程序，它们分别位于工程目录的 *test* 和 *sample* 目录. 

```c
/* gcc foo.c -laxcore */
#include "ax/algo.h" /* 基于迭代器的算法函数 */
#include "ax/list.h" /* 双链表 */
#include "ax/ptra.h" /* 引入自动指针 */
#include <stdlib.h>
#include <stdio.h>

/* 定义一个一元算子函数 */
void oper_del_odd(void *out, const void *in, void *args)
{
        const int *num = in;
        int *ret = out;
        if (*num % 2 == 1) /* 如果输入为基数，则输出为0 */
                *ret = 0;
}

int main(void)
{
        /* 定义双链表 */
        ax_list_r l = ax_new(ax_list, ax_t(int));

        int *count = malloc(sizeof *count);

        /* RAII块，退出作用域后，l和count自动被释放 */
        ax_scope(l.ax_one, ax_onelize(count)) {

                /* 对区间[1, 11)进行循环迭代 */
                ax_forrange(10, 0, -1)
                        /* 将区间每个元素逐个压入链表 */
                        ax_seq_push(l.ax_seq, &_);

                /* 转储 */
                ax_dump_out(l.ax_any);
                /* OUTPUT: foo.c:33:ax_one.ax_any.ax_box.ax_seq.ax_list {10, 9, 8, 7, 6, 5, 4, 3, 2, 1} */

                /* 创建一个一元谓词 */
                ax_pred del_odd = ax_pred_unary_make(oper_del_odd, NULL, NULL);

                ax_citer cbegin = ax_box_cbegin(l.ax_box);
                ax_citer cend = ax_box_cend(l.ax_box);

                /* 遍历容器中所有元素，对每个元素应用rmodd谓词 */
                ax_transform(&cbegin, &cend, ax_p(ax_iter, ax_box_begin(l.ax_box)), &del_odd);

                /* 转储 */
                ax_dump_out(l.ax_any);
                // OUTPUT: a.c:46:ax_one.ax_any.ax_box.ax_seq.ax_list {10, 0, 8, 0, 6, 0, 4, 0, 2, 0}

                /* 遍历链表元素并求和 */
                ax_box_foreach(l.ax_box, int *, i)
                        *count += *i;

                printf("Sum of elements = %d\n", *count);
                /* OUTPUT: Sum of elements = 30 */
        }

        return 0;
}
```

## HEADERS DESCRIPTION

### libaxcore

| 名称           | 描述 |
|---             |---   |
| ax/type/one.h  | 根类型 |
| ax/type/any.h  | 可序列化对象抽象 |
| ax/type/box.h  | 可迭代容器抽象 |
| ax/type/seq.h  | 线性表抽象 |
| ax/type/str.h  | 字符串抽象 |
| ax/type/map.h  | 映射表抽象 |
| ax/type/trie.h | 字典树抽象 |
| ax/type/tube.h | 单进单出管道抽象 |
| ax/def.h       | 基本声明 |
| ax/flow.h      | 高级流程控制 |
| ax/ring.h      | 模板化循环队列 |
| ax/trick.h     | 魔法宏 |
| ax/narg.h      | 参数测量宏 |
| ax/detect.h    | 编译环境探测宏 |
| ax/debug.h     | 断言 |
| ax/arraya.h    | 匿名的栈数组 |
| ax/oper.h      | 算子函数，与迭代器算法函数组合使用 |
| ax/dump.h      | 可视化转储 |
| ax/log.h       | 打印 |
| ax/algo.h      | 基于迭代器的算法 |
| ax/pred.h      | 算法函数的谓词和参数绑定 |
| ax/trait.h     | 类型特性，对数据类型的描述 |
| ax/iter.h      | 迭代器 |
| ax/mem.h       | 内存和串的操作 |
| ax/u1024.h     | 1024位无符号整数运算 |
| ax/array.h     | 静态数组容器 |
| ax/vector.h    | 向量表容器 |
| ax/deq.h       | 双端队列容器 |
| ax/list.h      | 双链表容器 |
| ax/hmap.h      | 散列表容器 |
| ax/avl.h       | 自平衡树容器 |
| ax/rb.h        | 红黑树容器 |
| ax/string.h    | 字符串容器 |
| ax/btrie.h     | 平衡字典树容器 |
| ax/queue.h     | 队列 |
| ax/stack.h     | 栈 |
| ax/pque.h      | 优先队列 |
| ax/unicode.h   | Unicode支持 |

### libaxnet

| 名称           | 描述 |
|---             |---   |
| ax/reactor.h   | 基于Reactor结构的socket事件驱动模型 |
| ax/event.h     | 事件结构 |
| ax/socket.h    | 套接字操作 |

### libaxkit

| 名称           | 描述 |
|---             |---   |
| cond.h         | 条件变量 |
| ctrlc.h        | 终端Ctrl C事件 |
| detect.h       | 架构、操作系统、编译器测试宏 |
| dir.h          | 遍历文件夹 |
| edit.h         | 终端行编辑 |
| errno.h        | 跨平台的错误代码 |
| ini.h          | ini配置文件操作 |
| io.h           | 输入输出操作 |
| lib.h          | 加载动态链接程序 |
| mutex.h        | 互斥量 |
| once.h         | 一次初始化 |
| option.h       | 命令行解析 |
| path.h         | 路径字符串操作 |
| proc.h         | 进程支持 |
| rwlock.h       | 读写锁 |
| sem.h          | 信号量 |
| stat.h         | 获取文件信息 |
| sys.h          | 文件系统操作等 |
| tcolor.h       | 终端颜色支持 |
| thread.h       | 线程 |
| tpool.h        | 线程池 |
| tss.h          | 线程本地存储 |
| types.h        | 类型定义 |
| uchar.h        | 跨平台字符串操作 |

### libaxgui

| 名称           | 描述 |
|---             |---   |
| ui.h           | 全局UI操作 |
| widget.h       | 基本控件 |
| window.h       | 窗口 |
| types.h        | UI相关类型定义 |
| box.h          | 线性布局 |
| button.h       | 按钮 |
| checkbox.h     | 复选框 |
| colorpicker.h  | 颜色拾取按钮 |
| combobox.h     | 复选框 |
| entry.h        | 文本框 |
| form.h         | 表单 |
| group.h        | 组控件 |
| image.h        | 设备无关图像 |
| label.h        | 文件控件 |
| menu.h         | 菜单控件 |
| radiobox.h     | 单选按钮组 |
| selectentry.h  | 下拉输入框 |
| slider.h       | 滑动输入条 |
| spinbox.h      | 数量输入框 |
| table.h        | 表格 |
| model.h        | 表格的模型数据 |
| timepicker.h   | 时间提取按钮 |

### libaxut

| ut/case.h      | 测试用例 |
| ut/suite.h     | 测试用例套件，用于批量执行测试 |
| ut/runner.h    | 测试用例执行容器 |

![GUI sample](./ui_sample.png "表格示例")

## LICENSE

该软件程序基于MIT协议发布. 参考[LICENSE](./LICENSE)文件

## AUTHOR

李希林 <lixilin@gmx.com>

