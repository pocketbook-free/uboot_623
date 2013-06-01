#ifndef __CORE_H
#define __CORE_H

#define  DEBUG    printf("\n\ndebug here ^_^ ^_^ ^_^ ^_^ ^_^ ^_^ ^_^\n\n\n");
#define  DEBUG_INFO(p)  printf("*********************************\n");\
                        printf("*name :%s\n",p->name);\
                        printf("*start:%0x\n",p->start);\
                        printf("*size :%d\n",p->size);\
                        printf("*crc  :%0x\n",p->crc);\
                        printf("*info_crc  :%0x\n",p->info_crc);\
                        printf("*********************************\n");

#define  DEBUG_MNG(p)   printf("*********************************\n");\
                        printf("*crc  :%0x\n",p->crc);\
                        printf("*info_crc  :%0x\n",p->info_crc);\
                        printf("*index_crc  :%0x\n",p->index_crc);\
                        printf("*********************************\n");

#define  DEBUG_INDEX(p) printf("*********************************\n");\
                        printf("*crc  :%0x\n",p);\
                        printf("*********************************\n");

#ifdef __cplusplus
extern "C" {
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif


