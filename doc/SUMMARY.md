# AXE #

AXE 是一个容器库，他包含了通用容器和算法，可以让程序员轻易的访问常见的数据结构。

AXE 通过结构体的依赖关系表达操作集合的继承关系。

* one
  * any
    * box
      * seq
      * map
      * str
      * wstr

每一个结构体都包含一组操作的集合，称之为trait，通过函数指针来实现。

如果一个结构体的第一个成员是这些结构体之一，并实现了对应的trait，那么可称它为one对象。

one对象包括

* vector
* hashmap
* avltree
* scope
* pair
* 

