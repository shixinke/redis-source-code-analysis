/* adlist.c - A generic doubly linked list implementation
 *
 * Copyright (c) 2006-2010, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */


#include <stdlib.h>
#include "adlist.h"
#include "zmalloc.h"

/* Create a new list. The created list can be freed with
 * AlFreeList(), but private value of every node need to be freed
 * by the user before to call AlFreeList().
 *
 * On error, NULL is returned. Otherwise the pointer to the new list. */
/**
 * 创建链表(链表结构体初始化)
 * @return （struct list *）
 */
list *listCreate(void)
{
    //声明一个链表指针
    struct list *list;
    //为链表申请分配内存
    if ((list = zmalloc(sizeof(*list))) == NULL)
        return NULL;
    //头结点和尾结点都指向NULL，即为一个空表
    list->head = list->tail = NULL;
    //表长为0
    list->len = 0;
    list->dup = NULL;
    list->free = NULL;
    list->match = NULL;
    return list;
}

/* Free the whole list.
 *
 * This function can't fail. */
/**
 * 释放链表空间(包括释放链接节点的空间)
 * @param list
 */
void listRelease(list *list)
{
    unsigned long len;
    listNode *current, *next;

    current = list->head;
    len = list->len;
    while(len--) {
        next = current->next;
        if (list->free) list->free(current->value);
        zfree(current);
        current = next;
    }
    zfree(list);
}

/* Add a new node to the list, to head, containing the specified 'value'
 * pointer as value.
 *
 * On error, NULL is returned and no operation is performed (i.e. the
 * list remains unaltered).
 * On success the 'list' pointer you pass to the function is returned. */
/**
 * 添加头结点
 * @param list
 * @param value
 * @return
 */
list *listAddNodeHead(list *list, void *value)
{
    //声明一个节点
    listNode *node;
    //为这个节点申请分配空间
    if ((node = zmalloc(sizeof(*node))) == NULL)
        return NULL;
    //赋值
    node->value = value;
    //如果链表长度为0,则没有任何结点
    if (list->len == 0) {
        //头结点和尾结点都指向刚分配的节点
        list->head = list->tail = node;
        //因为只有这个节点，所以它的前置结点和后续结点都是NULL
        node->prev = node->next = NULL;
    } else {
        //如果有其他节点元素
        //因为它是头结点，所以它前元素是NULL
        node->prev = NULL;
        //因为它是头结点，所以它的下一个结点指向原来的链表头指针
        node->next = list->head;
        //因为是双向链表，所以原来的链表头的前置节点变成新的表头指针
        list->head->prev = node;
        //设置链表的新的表头为刚插入的表头结点
        list->head = node;
    }
    //链表长度加1
    list->len++;
    return list;
}

/* Add a new node to the list, to tail, containing the specified 'value'
 * pointer as value.
 *
 * On error, NULL is returned and no operation is performed (i.e. the
 * list remains unaltered).
 * On success the 'list' pointer you pass to the function is returned. */
/**
 * 添加链表尾结点
 * @param list
 * @param value
 * @return
 */
list *listAddNodeTail(list *list, void *value)
{
    //声明一个结点指针(为新增加的尾结点使用)
    listNode *node;

    if ((node = zmalloc(sizeof(*node))) == NULL)
        return NULL;
    node->value = value;
    //如果原来是一张空表
    if (list->len == 0) {
        //对空表来说，新添加的元素即是表头也是表尾
        list->head = list->tail = node;
        //只有一个新插入的元素，所以这个元素的前置和后继结点都是NULL
        node->prev = node->next = NULL;
    } else {
        //如果原表不是空的
        //那新节点的前置就变成了原来表的表尾
        node->prev = list->tail;
        //当前节点是新的表尾，所以它的后继结点是NULL
        node->next = NULL;
        //将原来的尾结点的下一个指针指向这个新的结点
        list->tail->next = node;
        //将链表的尾结点指向到刚分配的新节点
        list->tail = node;
    }
    //结点长度加1
    list->len++;
    return list;
}

/**
 * 添加链表结点
 * @param list
 * @param old_node：指定结点
 * @param value
 * @param after：表示是插入到指定节点的后面，还是前面
 * @return
 */
list *listInsertNode(list *list, listNode *old_node, void *value, int after) {
    //声明一个新的结点指针
    listNode *node;
    //分配内存
    if ((node = zmalloc(sizeof(*node))) == NULL)
        return NULL;
    //设置结点的值
    node->value = value;
    //如果是插入到指定结点的后面
    if (after) {
        //所以将这个新结点的前置元素指向那个指定的元素
        node->prev = old_node;
        //将新结点的后继元素指向原结点的后继元素
        node->next = old_node->next;
        //如果原结点是链表的表尾，那么重新设置表尾为新元素
        if (list->tail == old_node) {
            list->tail = node;
        }
    } else {
        //如果是插入到指定结点的前面
        //将新结点的后继指向原结点
        node->next = old_node;
        //前新结点的前驱指向原结点的前驱元素
        node->prev = old_node->prev;
        //如果原结点是表头，则将链表表头指向新结点
        if (list->head == old_node) {
            list->head = node;
        }
    }
    //如果新结点的前驱不为NULL(不是表头)
    if (node->prev != NULL) {
        //则新结点的前驱元素的后继结点为这个新结点
        //类似说法：我前面的那个人的后面的人是我
        node->prev->next = node;
    }
    //如果新结点的后继不为NULL(不是表尾)
    if (node->next != NULL) {
        //则新结点的后继元素的前驱结点为这个新结点
        //类似说法：我后面的那个人的前面的人是我
        node->next->prev = node;
    }
    //链表长度加一
    list->len++;
    return list;
}

/* Remove the specified node from the specified list.
 * It's up to the caller to free the private value of the node.
 *
 * This function can't fail. */
/**
 * 删除链表结点
 * @param list 操作的链表
 * @param node 要删除的结点
 */
void listDelNode(list *list, listNode *node)
{
    //如果这个要删除的结点有前驱(即它不是头结点)
    if (node->prev)
        //将要删除的结点的前驱元素的下一个结点的指针指向要删除元素的下一个结点
        node->prev->next = node->next;
    else
        //如果是有头结点，则重新设置表的头结点为要删除元素的下一个结点
        list->head = node->next;
    //如果这个要删除的元素有后继(即它不是表尾)
    if (node->next)
        //将要删除的元素的前驱元素的前一个结点的指针指向要删除元素的上一个结点
        node->next->prev = node->prev;
    else
        //如果是表尾，重新设置表尾为要删除元素的上一个结点
        list->tail = node->prev;
    //释放链表中的元素
    if (list->free) list->free(node->value);
    //释放元素节点的空间
    zfree(node);
    //链表长度减一
    list->len--;
}

/* Returns a list iterator 'iter'. After the initialization every
 * call to listNext() will return the next element of the list.
 *
 * This function can't fail. */
/**
 * 获取当前链表的链表迭代器
 * @param list
 * @param direction
 * @return
 */
listIter *listGetIterator(list *list, int direction)
{
    //声明链表迭代器指针
    listIter *iter;
    //为迭代器申请分配内存空间
    if ((iter = zmalloc(sizeof(*iter))) == NULL) return NULL;
    //如果direction为0，表示迭代器指针的下一个元素指向链表表头
    if (direction == AL_START_HEAD)
        iter->next = list->head;
    else
        //否则迭代器指针的下一个元素指向链表表尾
        iter->next = list->tail;
    //指定链表迭代器的方向
    iter->direction = direction;
    return iter;
}

/* Release the iterator memory */
/**
 * 释放迭代器的空间
 * release有发布、释放的意思(松开)
 * @param iter
 */
void listReleaseIterator(listIter *iter) {
    zfree(iter);
}

/* Create an iterator in the list private iterator structure */
/**
 * 将链表迭代器指针重新指向表头
 * @param list
 * @param li
 */
void listRewind(list *list, listIter *li) {
    li->next = list->head;
    li->direction = AL_START_HEAD;
}

/**
 * 将链表迭代器指针指向表尾
 * @param list
 * @param li
 */
void listRewindTail(list *list, listIter *li) {
    li->next = list->tail;
    li->direction = AL_START_TAIL;
}

/* Return the next element of an iterator.
 * It's valid to remove the currently returned element using
 * listDelNode(), but not to remove other elements.
 *
 * The function returns a pointer to the next element of the list,
 * or NULL if there are no more elements, so the classical usage patter
 * is:
 *
 * iter = listGetIterator(list,<direction>);
 * while ((node = listNext(iter)) != NULL) {
 *     doSomethingWith(listNodeValue(node));
 * }
 *
 * */
/**
 * 返回迭代器的下一个结点的指针
 * @param iter
 * @return
 */
listNode *listNext(listIter *iter)
{
    //声明链表结点指针
    listNode *current = iter->next;
    //如果当前指针不为NULL
    if (current != NULL) {
        //如果迭代器的方向是从表头开始的
        if (iter->direction == AL_START_HEAD)
            iter->next = current->next;
        else
            iter->next = current->prev;
    }
    return current;
}

/* Duplicate the whole list. On out of memory NULL is returned.
 * On success a copy of the original list is returned.
 *
 * The 'Dup' method set with listSetDupMethod() function is used
 * to copy the node value. Otherwise the same pointer value of
 * the original node is used as value of the copied node.
 *
 * The original list both on success or error is never modified. */
/**
 * 复制链表
 * @param orig
 * @return
 */
list *listDup(list *orig)
{
    //用于复制的链表指针
    list *copy;
    listIter *iter;
    listNode *node;

    //创建空表
    if ((copy = listCreate()) == NULL)
        return NULL;
    copy->dup = orig->dup;
    copy->free = orig->free;
    copy->match = orig->match;
    iter = listGetIterator(orig, AL_START_HEAD);
    //遍历迭代器
    while((node = listNext(iter)) != NULL) {
        void *value;

        if (copy->dup) {
            value = copy->dup(node->value);
            if (value == NULL) {
                listRelease(copy);
                listReleaseIterator(iter);
                return NULL;
            }
        } else
            //获取结点的值
            value = node->value;
        //不断向表尾添加新元素
        if (listAddNodeTail(copy, value) == NULL) {
            listRelease(copy);
            listReleaseIterator(iter);
            return NULL;
        }
    }
    listReleaseIterator(iter);
    return copy;
}

/* Search the list for a node matching a given key.
 * The match is performed using the 'match' method
 * set with listSetMatchMethod(). If no 'match' method
 * is set, the 'value' pointer of every node is directly
 * compared with the 'key' pointer.
 *
 * On success the first matching node pointer is returned
 * (search starts from head). If no matching node exists
 * NULL is returned. */
/**
 * 在链表中查找指定的key
 * @param list
 * @param key
 * @return
 */
listNode *listSearchKey(list *list, void *key)
{
    //迭代器指针
    listIter *iter;
    //链表结点指针
    listNode *node;
    //初始化迭代器的位置(目前迭代器的next指针，指向list的头结点)
    iter = listGetIterator(list, AL_START_HEAD);
    while((node = listNext(iter)) != NULL) {
        if (list->match) {
            //如果key值与当前结点的值匹配指定的规则，则返回该结点
            if (list->match(node->value, key)) {
                //释放迭代器的内存空间
                listReleaseIterator(iter);
                return node;
            }
        } else {
            //如果当前结点的值，与指定的key相等，则返回当前结点
            if (key == node->value) {
                listReleaseIterator(iter);
                return node;
            }
        }
    }
    listReleaseIterator(iter);
    return NULL;
}

/* Return the element at the specified zero-based index
 * where 0 is the head, 1 is the element next to head
 * and so on. Negative integers are used in order to count
 * from the tail, -1 is the last element, -2 the penultimate
 * and so on. If the index is out of range NULL is returned. */
/**
 * 获取指定索引的链表节点
 * @param list
 * @param index
 * @return
 */
listNode *listIndex(list *list, long index) {
    //声明结点指针
    listNode *n;
    //如果索引小于0,那就表示是链表的尾部取(开始位置为表尾)
    if (index < 0) {
        //如果索引为负数的话，那么-1索引就是从后面数的第一个元素，而索引值是从0开始的，所以(-index)-1
        index = (-index)-1;
        //起始位置为表尾
        n = list->tail;
        //不断减小索引值，而且之个结点n要存在，不断将指针往前一个元素移动
        while(index-- && n) n = n->prev;
    } else {
        //开始位置为表头
        n = list->head;
        //不断增加索引值以移动位置，不断将指指针往后一个元素移动
        while(index-- && n) n = n->next;
    }
    return n;
}

/* Rotate the list removing the tail node and inserting it to the head. */
/**
 * 弹出(删除)列表尾结点，并这个结点放到链表头部位置(类似于队列中的pop+unshift)
 * rotate：回转、轮流的意思(因为它从尾部移动到头部，就像我们平时说的上帝为你关上了一扇门，一定会为你打开一扇窗)
 * @param list
 */
void listRotate(list *list) {
    //获取链表尾结点指针
    listNode *tail = list->tail;
    //如果表长度小于等于1,这个操作就没有任何意义，所以直接返回
    if (listLength(list) <= 1) return;

    /* Detach current tail */
    //将链接表尾结点指向到当前尾结点的前一个(因为这个当前尾结点要被删除掉，所以新的尾结点就是它前一个元素)
    list->tail = tail->prev;
    //将当前尾结点的前一个元素的next指针指向NULL(因为当前尾结点的前一个元素会成为新的尾结点，所以它的next为NULL)
    list->tail->next = NULL;
    /* Move it as head */
    //重新设置头结点，将头结点指向要删除的这个尾结点
    list->head->prev = tail;
    //将要删除的尾结点的前驱节点指向到NULL(因为它就是新的头结点)
    tail->prev = NULL;
    //将要删除的尾结点的下一个元素指向到链表的原来的头结点
    tail->next = list->head;
    //重新设置表的头结点，指向这个被删除的尾结点
    list->head = tail;
}
