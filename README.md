# Axe #

Axe 是一个容器库，他包含了通用容器和算法，可以有效简化对数据的封装。

该工程包括两个模块：AX 和 AXUT

AX包含容器、算法和迭代器等基本模块，头文件包括：

| 名称          | 描述 |
|---            |---   |
| ax/def.h      | 基本类型和程序控制结构定义 |
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
| ax/list.h     | 双链表，支持快速插入、移除元素 |
| ax/hmap.h     | 散列表，一种无序的映射表，常数时间的增删查和更新操作 |
| ax/avl.h      | AVL树，一种有序的映射表，对数时间的增删查和更新操作 |
| ax/string.h   | 字符串 |
| ax/btrie.h    | 平衡字典树 |
| ax/queue.h    | 队列，一种先进先出容器 |
| ax/stack.h    | 栈，一种后进先出容器 |

AXUT 用于单元测试，头文件包括：

| 名称          | 描述 |
|---            |---   |
| axut/case.h   | 测试用例结构定义 |
| axut/suite.h  | 测试用例集合 |
| axut/runner.h | 测试用例的执行和结果统计 |

