/* SDSLib, A C dynamic strings library
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

#ifndef __SDS_H
#define __SDS_H

#define SDS_MAX_PREALLOC (1024*1024)

#include <sys/types.h>
#include <stdarg.h>

/**
 * 别名定义：定义字符串指针sds
 */
typedef char *sds;

/**
 * 定义用于替换c字符串的结构体sdshdr
 *  len 表示字符串长度
 *  free 表示未使用的内存空间大小
 *  buf 表示用于保存字符串字面值的变量
 */
struct sdshdr {
    unsigned int len;
    unsigned int free;
    char buf[];
};

/**
 * 获取sds字符串长度(暂且这么叫吧)
 * @param s:sds字符串
 * @return size_t
 *
 * size_t 是一个long unsigned int
 */
static inline size_t sdslen(const sds s) {
    /**
     * 为什么这么实现呢？
     * 分析点如下：
     * 1、因为sdslen是在初始化sdshdr结构体后才能使用，所以要结合sdshdr的声明，注意这里的s其实和sdshdr->buf是指向同一个内在空间地址的
     * 2、结构体和数组类似，它的成员在内存中存储的地址是连续的，即结构体变量在内存中占用一段连续的地址，每个成员变量地址是相连的
     *
     * 做个实验：
     #include <stdio.h>
     #include <string.h>
     #include <stdlib.h>
     typedef char *sds;
     struct sdshdr {
         int len;
         int free;
         char buf[];
     };

     int main()
     {
         char *str = "name";
         struct sdshdr sdsobj, *ptr;
         ptr = &sdsobj;

         ptr->len = 4;
         ptr->free = 0;
         memcpy(ptr->buf, str, 5);
         printf("&ptr->len:%p\n", &ptr->len);    //0x7fff4e6c8ff0
         printf("&ptr->free:%p\n", &ptr->free);  //0x7fff4e6c8ff4
         printf("&ptr->buf:%p\n", &ptr->buf);    //0x7fff4e6c8ff8
         printf("ptr:%p\n", ptr);                //0x7fff4e6c8ff0
         return 0;
     }
     * 从上面的实验可知结构体指针ptr与buf指向的指针相关的就是len+free的size(即sdshdr的size，因为buf[]没有分配内存空间)之差
     * 因此可以通过buf的地址减去sdshdr的size，可以得到sdshdr结构体的指针
     */
    struct sdshdr *sh = (void*)(s-(sizeof(struct sdshdr)));
    return sh->len;
}

/**
 * 获取sds字符串可使用的内在空间大小
 * @param s
 * @return
 */
static inline size_t sdsavail(const sds s) {
    struct sdshdr *sh = (void*)(s-(sizeof(struct sdshdr)));
    return sh->free;
}

sds sdsnewlen(const void *init, size_t initlen);
sds sdsnew(const char *init);
sds sdsempty(void);
size_t sdslen(const sds s);
sds sdsdup(const sds s);
void sdsfree(sds s);
size_t sdsavail(const sds s);
sds sdsgrowzero(sds s, size_t len);
sds sdscatlen(sds s, const void *t, size_t len);
sds sdscat(sds s, const char *t);
sds sdscatsds(sds s, const sds t);
sds sdscpylen(sds s, const char *t, size_t len);
sds sdscpy(sds s, const char *t);

sds sdscatvprintf(sds s, const char *fmt, va_list ap);
#ifdef __GNUC__
sds sdscatprintf(sds s, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));
#else
sds sdscatprintf(sds s, const char *fmt, ...);
#endif

sds sdscatfmt(sds s, char const *fmt, ...);
sds sdstrim(sds s, const char *cset);
void sdsrange(sds s, int start, int end);
void sdsupdatelen(sds s);
void sdsclear(sds s);
int sdscmp(const sds s1, const sds s2);
sds *sdssplitlen(const char *s, int len, const char *sep, int seplen, int *count);
void sdsfreesplitres(sds *tokens, int count);
void sdstolower(sds s);
void sdstoupper(sds s);
sds sdsfromlonglong(long long value);
sds sdscatrepr(sds s, const char *p, size_t len);
sds *sdssplitargs(const char *line, int *argc);
sds sdsmapchars(sds s, const char *from, const char *to, size_t setlen);
sds sdsjoin(char **argv, int argc, char *sep);

/* Low level functions exposed to the user API */
sds sdsMakeRoomFor(sds s, size_t addlen);
void sdsIncrLen(sds s, int incr);
sds sdsRemoveFreeSpace(sds s);
size_t sdsAllocSize(sds s);

#endif
