/*
 * =====================================================================================
 *
 *       Filename:  item.c
 *
 *    Description:  Operations of Item area 
 *
 *        Version:  1.0
 *        Created:  05/05/2011 08:46:17 AM
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
#include "crc32.h"
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#endif
#include "item.h"
#include "error_msg.h"
#include "policy.h"
#include "device.h"

/* globle buffer pointer */
__u16 *index_buf;                                                                
item_info_t *info_buf;                                                           
manage_t *manage_buf;


static unsigned long get_area_crc(unsigned long offset, unsigned int size)
{
    unsigned char *buffer = NULL;
    unsigned long crc;

    buffer = (unsigned char *)malloc(size);
    if (buffer == NULL)
        return MALLOC_ERROR;

    read_device(buffer, offset, size);

    crc = crc32(0L, buffer, size);

    free(buffer);
    return crc;
}
#if 0
int policy_clear_all(void)
{
	unsigned char *buffer = NULL;
	manage_t *mng = NULL;
	buffer = (unsigned char *)malloc(FORMAT_SZ);
	if (NULL == buffer) {
		printf("error~~ malloc fail\n");
		return MALLOC_ERROR;
	    printf("now format policy partition\n");
	}
						   
	memset(buffer, EMPTY_OF_ITEM, FORMAT_SZ);
	write_device(buffer, 0, FORMAT_SZ);
	memset(buffer, 0x00, sizeof(manage_t));
	write_device(buffer, INDEX_AR_SZ + INFO_AR_SZ, sizeof(manage_t));
	mng = (manage_t *)malloc(sizeof(manage_t));
	if(mng == NULL)
       return MALLOC_ERROR;
    read_device(mng, NORMAL_PAR_OFS + CRC_AR_OFS, sizeof(manage_t));

    mng->index_crc = get_area_crc(NORMAL_PAR_OFS , INDEX_AR_SZ);
    mng->info_crc  = get_area_crc(NORMAL_PAR_OFS + INFO_AR_OFS, INFO_AR_SZ);
    mng->crc = crc32(0L, (unsigned char *)mng, sizeof(manage_t));
    write_device(mng, CRC_AR_OFS, sizeof(manage_t));
    free(mng);
    free(buffer);
    buffer = (unsigned char *)malloc(1*1024*1024);
    if (NULL == buffer) {
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
int policy_format(void)
{
	unsigned char *buffer = NULL;
#if 0    
	 manage_t *mng = NULL;

	 buffer = (unsigned char *)malloc(FORMAT_SZ);
	 if (NULL == buffer) {
		 printf("error~~ malloc fail\n");
		 return MALLOC_ERROR;
	 }
	 printf("now format policy partition\n");
						  
	 memset(buffer, EMPTY_OF_ITEM, FORMAT_SZ);
	 write_device(buffer, 0, FORMAT_SZ);
	 memset(buffer, 0x00, sizeof(manage_t));
	 write_device(buffer, INDEX_AR_SZ + INFO_AR_SZ, sizeof(manage_t));
		   
	 mng = (manage_t *)malloc(sizeof(manage_t));
	 if (mng == NULL)
		 return MALLOC_ERROR;
	 read_device(mng, NORMAL_PAR_OFS + CRC_AR_OFS, sizeof(manage_t));
	 mng->index_crc = get_area_crc(NORMAL_PAR_OFS , INDEX_AR_SZ);
	 mng->info_crc  = get_area_crc(NORMAL_PAR_OFS + INFO_AR_OFS, INFO_AR_SZ);
	 mng->crc       = crc32(0L, (unsigned char *)mng, sizeof(manage_t));
	 write_device(mng, CRC_AR_OFS, sizeof(manage_t));

	 free(buffer);
#endif
	// buffer = (unsigned char *)malloc(1*1024*1024);
	 buffer = (unsigned char *)malloc(1*512);
   	 if (NULL == buffer) {
     printf("error~~ malloc fail\n");
     return MALLOC_ERROR;
	 }
     memset(buffer, 0x00, 1*512);
     printf("clear data area.....\n");
     write_device(buffer, 0, 1*512);
//   write_device(buffer, 1*1024*1024 + 1*1024*1024, 1*1024*1024);
     printf("clear done.....\n");
     free(buffer);
//	 free(mng);

	 return 0;
}


static int check_manage_crc(unsigned long offset)
{
    unsigned long crc,old_crc;
    manage_t *mng = NULL;

    mng = (manage_t *)malloc(sizeof(manage_t));
    if (mng == NULL)
        return MALLOC_ERROR;
    read_device(mng, offset + CRC_AR_OFS, sizeof(manage_t));

    old_crc = mng->crc;
    mng->crc &= ~0xffffffff;
    crc = crc32(0L, (unsigned char *)mng, sizeof(manage_t));

    if (crc != old_crc)
    {
        printf("manage area error\n");
        return MANAGE_AREA_ERROR;
    }

    if (mng->index_crc != get_area_crc(offset , INDEX_AR_SZ))
    {
        printf("index area crc error\n");
        return INDEX_AREA_CRC_ERROR;
    }

    if (mng->info_crc != get_area_crc(offset + INFO_AR_OFS, INFO_AR_SZ))
    {
        printf("info area crc error\n");
        return INFO_AREA_CRC_ERROR;
    }

    free(mng);
    return 0;
}

static int check_policy_crc(void)
{
    int nor, bak;
	bak = 0;
    nor = check_manage_crc(NORMAL_PAR_OFS);
    //bak = check_manage_crc(BACKUP_PAR_OFS);

    if (nor < 0) 
    {  
      //  if (bak < 0)
        {
            printf("policy partition both invalid\nneed to format policy partition\n");
            return BOTH_NOT_VALID;
        }
#if 0
		else
        {
            printf("normal partition error\n");
           // recovery_fs(NORMAL);
        }
#endif
    }
    else
    {
        if (bak < 0)
        {
            printf("backup partition error\n");
            //recovery_fs(BACKUP);
        }
    }

    return 0;
}


static void set_manage_crc()
{
    unsigned long crc;

    manage_buf->index_crc = get_area_crc(NOR_INDEX_OFS, INDEX_AR_SZ);
    manage_buf->info_crc = get_area_crc(NOR_INFO_OFS, INFO_AR_SZ);

    manage_buf->crc &= ~0xffffffff;
    crc = crc32(0L, (unsigned char *)manage_buf, sizeof(manage_t));
    manage_buf->crc = crc;

    write_device(manage_buf, INDEX_AR_SZ + INFO_AR_SZ, sizeof(manage_t));
}

static int check_info_crc(item_info_t *current_info)
{
    unsigned long crc, old_crc;

    old_crc = current_info->info_crc;
    current_info->info_crc &= ~0xffffffff;
    crc = crc32(0L, (unsigned char *)current_info, sizeof(item_info_t));
    if (crc != old_crc)
    {
        printf("crc of item info error\n");
        return ITEM_INFO_CRC_ERROR;
    }
    current_info->info_crc = old_crc;

    return 0;
}
static int check_data_crc(void *mem, item_info_t *current_info)
{
    if (current_info->crc != crc32(0L, mem, current_info->size))
    {
        printf("content error\n");
        return ITEM_CONTENT_CRC_ERROR;
    }

    return 0;
}

void free_global_buffer(void)
{
    free(index_buf);
    free(info_buf);
    free(manage_buf);
}

int init_global_buffer(void)
{
    int ret;

    ret = check_policy_crc();
    if (ret < 0)
        return ret;

    index_buf   = (unsigned short *)malloc(INDEX_AR_SZ);
    if (index_buf == NULL)
    {
        printf("malloc fail\n");
        return MALLOC_ERROR;
    }
    info_buf    = (item_info_t *)malloc(INFO_AR_SZ);
    if (info_buf == NULL)
    {
        printf("malloc fail\n");
        return MALLOC_ERROR;
    }
    manage_buf  = (manage_t *)malloc(sizeof(manage_t));
    if (manage_buf == NULL)
    {
        printf("malloc fail\n");
        return MALLOC_ERROR;
    }

    read_device( manage_buf, NOR_CRC_OFS, sizeof(manage_t));
    read_device( index_buf, NOR_INDEX_OFS, INDEX_AR_SZ);
    read_device( info_buf, NOR_INFO_OFS, INFO_AR_SZ);

    return 0;
}

static int get_total_unit(int size)
{
    int total_unit;

    total_unit = size / DATA_UNIT_SZ;
    if (size % DATA_UNIT_SZ > 0)
        total_unit += 1;

    return total_unit;
}
static int get_surplus_unit(void)
{
    item_info_t *current_info= NULL;
    int surplus_unit = 0, total_unit, i;

    current_info = info_buf;

    for (i = 0; i < manage_buf->info_bdr; i++)
    {
        total_unit = get_total_unit(current_info->size);
        current_info++;
        surplus_unit += total_unit;
    }

    return (DATA_UNIT_NR - surplus_unit);
}
static int get_empty_info_offset(void)
{
    item_info_t *current_info = NULL;
    int i = 0;

    current_info = info_buf;

    while (0xFFFF != current_info->start)
    {
        current_info++;
        i++;
    }

    return i;
}

static int get_item_info_offset(char *file_name)
{
    item_info_t *current_info= NULL;
    int i = 0;

    current_info = info_buf;

    while (strcmp(current_info->name, file_name) != 0)
    {
        current_info++;
        i++;
        if (i > manage_buf->info_bdr + 1)
            return ITEM_NOT_FOUND;
    }

    return i;
}

static void remove_item_info(char *file_name)
{
    int current_offset;
    item_info_t *current_info;

    current_offset = get_item_info_offset(file_name);
    current_info = info_buf + current_offset;
    memset(current_info, EMPTY_OF_ITEM, INFO_AR_OFS);
    write_device(current_info,INFO_AR_OFS + current_offset*INFO_UNIT_SZ ,INFO_UNIT_SZ);
}

static int get_empty_index(unsigned short flag)
{
    unsigned short *current_index;    

    current_index = index_buf + flag;
    while (*current_index != EMPTY_OF_ITEM )
        current_index++;

    return (current_index - index_buf);
}

static void remove_item_index(item_info_t *current_info)
{
    unsigned short next_index,*current_index, total_unit;
    int i = 0;

    current_index = index_buf + current_info->start;

    total_unit = get_total_unit(current_info->size);

    do
    {
        next_index = *current_index;
        *current_index = EMPTY_OF_ITEM;
        current_index = index_buf + next_index;
        i++;
    }while (i < total_unit);

    write_device(index_buf, 0 , INDEX_AR_SZ);
}


static int verify_item_exist(char *file_name)
{
    int offset_of_info;
    item_info_t *current_info = NULL;

    offset_of_info = get_item_info_offset(file_name);

    if (offset_of_info >= 0)
    {
        current_info = info_buf + offset_of_info ;
        if (current_info->attr == ITEM_PREDEF)
        {
            printf("ITEM_WRITE_DENY\n");
            return ITEM_WRITE_DENY;
        }
        printf("the file have existed, now begin to remove file\n");
        remove_item_index(current_info);
        remove_item_info(file_name);

    }

    return 0;
}

static int add_item_info(char *file_name, void *mem, int size, unsigned char att, unsigned char type)
{
    int offset_of_info,total_unit,ret ;
    item_info_t *current_info = NULL;
    unsigned long crc;

    if (strlen(file_name) > 11)
    {
        printf("the file_name is too long");
        return TOO_LONG_FILENAME;
    }

    total_unit = get_total_unit(size);
    if (total_unit > get_surplus_unit())
    {
        printf("there is no enough space\n");
        return NO_ENOUGH_SPACE;
    }

    ret = verify_item_exist(file_name);
    if (ret < 0)
        return ret;

    offset_of_info = get_empty_info_offset();
    current_info = info_buf + offset_of_info;

    strcpy(current_info->name, file_name);
    current_info->attr = att;
    current_info->type = type;
    current_info->size = size;
    current_info->start = get_empty_index(0); 
    current_info->crc = crc32(0L, (unsigned char *)mem, size);
    current_info->info_crc &= ~0xffffffff;
    crc = crc32(0L, (unsigned char *)current_info, INFO_UNIT_SZ);
    current_info->info_crc = crc;

    write_device(current_info,INFO_AR_OFS + offset_of_info*INFO_UNIT_SZ, INFO_UNIT_SZ);
    if(manage_buf->info_bdr == offset_of_info)
        manage_buf->info_bdr  += 1;
    write_device(manage_buf, INDEX_AR_SZ + INFO_AR_SZ, sizeof(manage_t));

    return 0;
}

static void set_item_data(void *mem, unsigned int size, item_info_t *current_info)
{
    unsigned short current_index, next_index,last_unit_size, total_unit ; 
    unsigned char *nonius_of_mem = NULL;
    int i = 0;

    last_unit_size = current_info->size % DATA_UNIT_SZ;
    total_unit = get_total_unit(current_info->size);

    current_index = current_info->start;
    nonius_of_mem = (unsigned char *)mem;

    while (i < total_unit)
    {
        if (i == total_unit - 1)
        {
            write_device(nonius_of_mem, FORMAT_SZ + current_index*DATA_UNIT_SZ, last_unit_size);
            next_index = 0xfffe;
        }
        else
        {
            write_device(nonius_of_mem, FORMAT_SZ + current_index*DATA_UNIT_SZ, DATA_UNIT_SZ);
            next_index = get_empty_index(current_index + 1);
        }

        write_device(&next_index, current_index * INDEX_UNIT_SZ, INDEX_UNIT_SZ);

        current_index = next_index;
        nonius_of_mem += DATA_UNIT_SZ;
        i++;
    }
}


static void get_item_data(void *mem, item_info_t *current_info, unsigned char flag) 
{
    int i = 0;
    unsigned char *nonius_of_mem = NULL;
    unsigned short current_index, last_unit_size, total_unit;

    last_unit_size = current_info->size % DATA_UNIT_SZ;
    total_unit = get_total_unit(current_info->size);
    current_index = current_info->start;

    nonius_of_mem = (unsigned char *)mem;

    while (i < total_unit)
    {
        if (i == total_unit - 1)
        {
            read_device(nonius_of_mem, NORMAL_PAR_OFS+(flag-1)*POLICY_SIZE + FORMAT_SZ + current_index*DATA_UNIT_SZ, last_unit_size);
        }
        else
        {
            read_device(nonius_of_mem, NORMAL_PAR_OFS+(flag-1)*POLICY_SIZE + FORMAT_SZ + current_index*DATA_UNIT_SZ, DATA_UNIT_SZ);
        }
        nonius_of_mem += DATA_UNIT_SZ;

        read_device(&current_index, NORMAL_PAR_OFS+(flag-1)*POLICY_SIZE + current_index*INDEX_UNIT_SZ, INDEX_UNIT_SZ);

 //       if (current_index == 0xfffe)
  //          break;
        i++;
    }
}

int get_item_size(char *file_name)
{
    int size;

    size = get_item_info_offset(file_name);
    if (size < 0)
        return size;
    size = (info_buf + size)->size;

    return size;
}

int set_item(char *file_name, void *mem, unsigned int size, unsigned char type, unsigned char attr)
{
    item_info_t *current_info;    
    int ret;

    ret = init_global_buffer();
    if (ret < 0)
        return ret;

    ret = add_item_info(file_name, mem, size, attr, type);
    if (ret < 0)
        return ret;

    current_info = info_buf + get_item_info_offset(file_name);
    set_item_data(mem, size, current_info);

    set_manage_crc();
    free_global_buffer();
    return 0;
}

int get_item(char *file_name, void *mem)
{
    int ret = 0, ret_check_nor_info, ret_check_bak_info, ret_check_nor_data, ret_check_bak_data;
    unsigned char *buffer = NULL;
    item_info_t *normal_info = NULL, *backup_info = NULL;    
	
	ret_check_bak_info = 0;
	ret_check_bak_data = 0;
    ret = init_global_buffer();
    if (ret < 0)
        return ret;

    normal_info = info_buf + get_item_info_offset(file_name);
    get_item_data(mem, normal_info, NORMAL);
    ret_check_nor_info = check_info_crc(normal_info);
    ret_check_nor_data = check_data_crc(mem, normal_info);

    buffer = (unsigned char *)malloc(normal_info->size);
    read_device( manage_buf, BAC_CRC_OFS, sizeof(manage_t));
    read_device( index_buf, BAC_INDEX_OFS, INDEX_AR_SZ);
    read_device( info_buf, BAC_INFO_OFS, INFO_AR_SZ);

    backup_info = info_buf + get_item_info_offset(file_name);
    get_item_data(buffer, backup_info, BACKUP);
    ret_check_bak_info = check_info_crc(backup_info);
    ret_check_bak_data = check_data_crc(buffer, backup_info);

    if (ret_check_nor_info < 0)
    {
        if (ret_check_bak_info < 0)
        {
            printf("info of %s at normal and backup are error\n", file_name);
            return BOTH_NOT_VALID;
        }
        else
        {
            printf("info of %s at normal is error\n", file_name);
            //recovery_fs(NORMAL);
        }
    }
    else
    {
        if (ret_check_bak_info < 0)
        {
            printf("info of %s at backup is error\n", file_name);
            //recovery_fs(BACKUP);

        }
    }

    if (ret_check_nor_data < 0)
    {
        if (ret_check_bak_data < 0)
        {
            printf("data of %s at normal and backup are error\n", file_name);
            return BOTH_NOT_VALID;
        }
        else
        {
            printf("data of %s at normal is error\n", file_name);
            //recovery_data(NORMAL);
            memcpy(mem, buffer, backup_info->size);
        }

    }
    else
    {
        if (ret_check_bak_data < 0)
        {
            printf("data of %s at backup is error\n", file_name);
            //recovery_data(BACKUP);
        }
    }

    free(buffer);
    free_global_buffer();

    return ret;
}


