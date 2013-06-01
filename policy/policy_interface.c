/*
 * =====================================================================================
 *
 *       Filename:  polity_interface.c
 *
 *    Description:  interfaces for AP
 *
 *        Version:  1.0
 *        Created:  05/06/2011 02:16:19 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Morgan
 *        Company:  TMSBG-CDPG-RDSW
 *
 * =====================================================================================
 */
#ifdef _UBOOT_
#include <common.h>
#elif defined(_ANDROID_)
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <android/log.h>
#include "cutils/log.h"
#endif
#include "item.h"
#include "policy_interface.h"
#include "error_msg.h"
#include "policy.h"
#include "device.h"
#include "item.h"

#define RO 0
#define RW 1

struct policy_data
{
    char name[11];
    unsigned char type;
    int    max_length;
    unsigned char attr;
}usr_data[] =
{
    {VCOM, STRING_TYPE, 16,   RW},
    {WAVEFORM, BINARY_TYPE, 1 * 1024 * 1024, RO},
 //   {DATA1  , BINARY_TYPE, 1024, RO},
 //   {DATA2  , BINARY_TYPE, 1024, RO},
 //   {DATA3  , BINARY_TYPE, 1024, RO},
//    {CHARGE_LOGO   , BINARY_TYPE, 1024, RO}
};
#if 0
int policy_clear_all(void)
{
    unsigned char *buffer = NULL;

    buffer = (unsigned char *)malloc(FORMAT_SZ);
    if (NULL == buffer)
    {
        printf("error~~ malloc fail\n");
        return MALLOC_ERROR;
    }

    printf("now format policy partition\n");
   
    memset(buffer, EMPTY_OF_ITEM, FORMAT_SZ);
    write_device(buffer, 0, FORMAT_SZ);

    memset(buffer, 0x00, sizeof(manage_t));
    write_device(buffer, INDEX_AR_SZ + INFO_AR_SZ, sizeof(manage_t));

    free(buffer);

    buffer = (unsigned char *)malloc(1*1024*1024);
    if (NULL == buffer)
    {
        printf("error~~ malloc fail\n");
        return MALLOC_ERROR;
    }
    memset(buffer, 0x00, 1*1024*1024);
	printf("clear data area.....\n");
    write_device(buffer, 1*1024*1024, 1*1024*1024);
    write_device(buffer, 1*1024*1024 + 1*1024*1024, 1*1024*1024);
	printf("clear done.....\n");
    free(buffer);

    return 0;
}
#endif
static int check_id(char *item, int size)
{
    int i, count;

    count = sizeof(usr_data) / sizeof(usr_data[0]);
    for (i = 0; i < count; i++)
    {
        if(strcmp(item, usr_data[i].name) == 0)
            break;
    }
    if (i == count)
    {
        printf("The ID type you inputed  doesn't exist\n");
        return ITEM_NOT_FOUND;
    }
    else
    {
       if (size > usr_data[i].max_length) 
           return ITEM_SIZE_WRONG;
       if (usr_data[i].attr == RO)
           return i;
    }

    return 0;
}

int policy_set_id(char *item, void *mem, int size)
{
     int ret;
  
//     SLOGE("^_^ ^_^ ^_^ now call policy set id\n");
#if 1
    ret = check_id(item, size);
    if (ret < 0)
        return ret;
    if (usr_data[ret].attr == RO)
        return ITEM_WRITE_DENY;
#endif 
    ret = set_item(item, mem, size, STRING_TYPE, 2);
    return ret;
}

int policy_get_id(char *item, void *mem)
{
    int ret;
//     SLOGE("^_^ ^_^ ^_^ now call policy get id\n");
    ret = get_item(item, mem);
    return ret;
}

int policy_store_item_data(char *item, void *mem, int size)
{
    int ret;
#if 1
    ret = check_id(item, size);
    if (ret < 0)
        return ret;
#endif 
    ret = set_item(item, mem, size, BINARY_TYPE, 2);
    return ret;
}

int policy_get_item_data(char *item, void *mem)
{
    int ret;
    ret = get_item(item, mem);
    return ret;
}

int policy_get_item_size(char *item)
{
    int size;
    int ret;

    ret = init_global_buffer();
    if (ret < 0)
        return ret;
    
    size = get_item_size(item);
    free_global_buffer();

    return size;
}

