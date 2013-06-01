/*
 * =====================================================================================
 *
 *       Filename:  device.c
 *
 *    Description:  hardware read and write for other functions
 *
 *        Version:  1.0
 *        Created:  05/09/2011 05:57:35 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Morgan
 *        Company:  TMSBG-CDPG-RDSW
 *
 * =====================================================================================
 */

#ifdef _ANDROID_
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#elif defined(_UBOOT_)
#include <common.h>
#endif
#include "device.h"
#include "error_msg.h"
#include "policy.h"

int read_device(void *buffer, unsigned long offset, unsigned int size)
{
#ifdef _ANDROID_
    int fd;

    fd = open(POLICY_DEV, O_RDWR | O_SYNC);
    if (fd < 0)
    {
        printf("error~~can't open device  \n");
        return OPEN_DEVICE_ERROR; 
    }

    lseek(fd, offset, SEEK_SET);
    read(fd, buffer, size);

    close(fd);
    return 0;
#elif defined(_UBOOT_)
	char tmp_cmd[100];
	int block, count, remain;
	char *tmp = NULL;
	int rider = 0;

	printf("%x, %lx, %x\n", (unsigned)buffer, offset, size);

	tmp = (char *)malloc(512);
	if (!tmp) {
		printf("malloc fail!\n");
		goto ret_lb;
	}
	block = (offset + NORMAL_PAR_OFS)/512;
//	sprintf(tmp_cmd, "mmc read 0 0x%x 0x%x 0x%x", (unsigned)tmp, block, 1);
	sprintf(tmp_cmd, "mmc read 2 0x%x 0x%x 0x%x", (unsigned)tmp, block, 1);
	run_command(tmp_cmd, 0);
	memcpy(buffer, tmp, size);
#if 0
	//block = (offset + NORMAL_PAR_OFS)/512;
	block = offset/512;
	// first block
	if ((offset+BACKUP_PAR_OFS)%512) {
		sprintf(tmp_cmd, "mmc read 0 0x%x 0x%x 0x%x", (unsigned)tmp, block, 1);
//		sprintf(tmp_cmd, "mmc read 2 0x%x 0x%x 0x%x", (unsigned)tmp, block, 1);
		run_command(tmp_cmd, 0);

		remain = (offset + NORMAL_PAR_OFS)%512;
		if (size >= 512) {
			memcpy(buffer, tmp + remain, 512-remain);

			rider = 512-remain;
			size -= rider;
			block++;
		} else {
			memcpy(buffer, tmp + remain, size);
			size = 0;
		}

		if(!size) {
			goto ret_lb;
		}
	}

	// body blocks
	count = size/512;
	if (count) {
		sprintf(tmp_cmd, "mmc read 0 0x%x 0x%x 0x%x", (unsigned)(buffer + rider), block, count);
//		sprintf(tmp_cmd, "mmc read 2 0x%x 0x%x 0x%x", (unsigned)(buffer + rider), block, count);
		run_command(tmp_cmd, 0);
	}

	// last block
	remain = size%512;
	if (remain) {
		block += count;
		rider += count*512;
		sprintf(tmp_cmd, "mmc read 0 0x%x 0x%x 0x%x", (unsigned)tmp, block, 1);
//		sprintf(tmp_cmd, "mmc read 2 0x%x 0x%x 0x%x", (unsigned)tmp, block, 1);
		run_command(tmp_cmd, 0);

		memcpy((buffer + rider), tmp, remain);
	}
#endif
ret_lb:
	if (tmp)
		free(tmp);
	return 0;
#endif
}

int write_device(const void *buffer, unsigned long offset, unsigned int size)
{
#ifdef _ANDROID_
    int fd;

    fd = open(POLICY_DEV, O_RDWR);
    if (fd < 0)
    {
        printf("error~~can't open device  \n");
        return OPEN_DEVICE_ERROR;
    }

    lseek(fd, offset + NORMAL_PAR_OFS, SEEK_SET);
    write(fd, buffer, size);

    lseek(fd, offset + BACKUP_PAR_OFS, SEEK_SET);
    write(fd, buffer, size);

    close(fd);
    return 0;
#elif defined(_UBOOT_)
	char tmp_cmd[100];
	int block, count, remain;
	char *tmp = NULL;
	int rider = 0;

//	printf("%x, %lx, %x\n", (unsigned)buffer, offset, size);

	tmp = (char *)malloc(512);
	if (!tmp) {
		printf("malloc fail!\n");
		goto ret_lb;
	}

	block = (offset + NORMAL_PAR_OFS)/512;
	/* for offset and may not 512-bytes-align, we need 
	 * to cope with this
	 */
	// first block
	if ((offset+BACKUP_PAR_OFS)%512) {
	//	sprintf(tmp_cmd, "mmc read 0 0x%x 0x%x 0x%x", (unsigned)tmp, block, 1);
		sprintf(tmp_cmd, "mmc read 2 0x%x 0x%x 0x%x", (unsigned)tmp, block, 1);
		run_command(tmp_cmd, 0);

		remain = (offset + NORMAL_PAR_OFS)%512;
		if (size >= 512) {
			memcpy(tmp + remain, buffer, 512-remain);

			rider = 512-remain;
			size -= rider;
			block++;
		} else {
			memcpy(tmp + remain, buffer, size);
			size = 0;
		}
	//	sprintf(tmp_cmd, "mmc write 0 0x%x 0x%x 0x%x", (unsigned)tmp, block, 1);
		sprintf(tmp_cmd, "mmc write 2 0x%x 0x%x 0x%x", (unsigned)tmp, block, 1);
		run_command(tmp_cmd, 0);

		if(!size) {
			goto ret_lb;
		}
	}

	// body blocks
	count = size/512;
	if (count) {
	//	sprintf(tmp_cmd, "mmc write 0 0x%x 0x%x 0x%x", (unsigned)(buffer + rider), block, count);
		sprintf(tmp_cmd, "mmc write 2 0x%x 0x%x 0x%x", (unsigned)(buffer + rider), block, count);
		run_command(tmp_cmd, 0);
	}

	// last block
	remain = size%512;
	if (remain) {
		block += count;
		rider += count*512;
	//	sprintf(tmp_cmd, "mmc read 0 0x%x 0x%x 0x%x", (unsigned)tmp, block, 1);
		sprintf(tmp_cmd, "mmc read 2 0x%x 0x%x 0x%x", (unsigned)tmp, block, 1);
		run_command(tmp_cmd, 0);

		memcpy(tmp, (buffer + rider), remain);
	//	sprintf(tmp_cmd, "mmc write 0 0x%x 0x%x 0x%x", (unsigned)tmp, block, 1);
		sprintf(tmp_cmd, "mmc write 2 0x%x 0x%x 0x%x", (unsigned)tmp, block, 1);
		run_command(tmp_cmd, 0);
	}

ret_lb:
	if (tmp)
		free(tmp);
	return 0;
#endif
}
#if 0
int recovery_fs(unsigned char flag)
{
#ifdef _ANDROID_
    int fd;
    unsigned char *buffer = NULL;

    printf("now recovery filesystem\n");
    fd = open(POLICY_DEV, O_RDWR);
    if (fd < 0)
    {
        printf("error~~can't open device  \n");
        return OPEN_DEVICE_ERROR;
    }
#elif defined(_UBOOT_)
	unsigned char *buffer = NULL;
#endif

    buffer = (unsigned char *)malloc(FORMAT_SZ);
    if (buffer == NULL)
        return MALLOC_ERROR;

    if (flag == NORMAL)
    {
#ifdef _ANDROID_
        lseek(fd, BACKUP_PAR_OFS, SEEK_SET);
        read(fd, buffer, FORMAT_SZ);
        lseek(fd, NORMAL_PAR_OFS, SEEK_SET);
        write(fd, buffer, FORMAT_SZ);
#elif defined(_UBOOT_)
	read_device(buffer, BACKUP_PAR_OFS, FORMAT_SZ);
	write_device(buffer, NORMAL_PAR_OFS, FORMAT_SZ);
#endif
    }
    if (flag == BACKUP)
    {
#ifdef _ANDROID_
        lseek(fd, NORMAL_PAR_OFS, SEEK_SET);
        read(fd, buffer, FORMAT_SZ);
        lseek(fd, BACKUP_PAR_OFS, SEEK_SET);
        write(fd, buffer, FORMAT_SZ);
#elif defined(_UBOOT_)
	read_device(buffer, NORMAL_PAR_OFS, FORMAT_SZ);
	write_device(buffer, BACKUP_PAR_OFS, FORMAT_SZ);
#endif
    }

    free(buffer);
#ifdef _ANDROID_
    close(fd);
#endif
    return 0;
}
int recovery_data(unsigned char flag)
{
#ifdef _ANDROID_
    int fd;
    unsigned char *buffer = NULL;

    printf("now recovery data\n");
    fd = open(POLICY_DEV, O_RDWR);
    if (fd < 0)
    {
        printf("error~~can't open device  \n");
        return OPEN_DEVICE_ERROR;
    }
#elif defined(_UBOOT_)
	unsigned char *buffer = NULL;
#endif

    buffer = (unsigned char *)malloc(DATA_AR_SZ);
    if (buffer == NULL)
        return MALLOC_ERROR;

    if (flag == NORMAL)
    {
#ifdef _ANDROID_
        lseek(fd, BAC_DATA_OFS, SEEK_SET);
        read(fd, buffer, DATA_AR_SZ);

        lseek(fd, NOR_DATA_OFS, SEEK_SET);
        write(fd, buffer, DATA_AR_SZ);
#elif defined(_UBOOT_)
	read_device(buffer, BAC_DATA_OFS, DATA_AR_SZ);
	write_device(buffer, NOR_DATA_OFS, DATA_AR_SZ);
#endif
    }
    if (flag == BACKUP)
    {
#ifdef _ANDROID_
        lseek(fd, NOR_DATA_OFS, SEEK_SET);
        read(fd, buffer, DATA_AR_SZ);

        lseek(fd, BAC_DATA_OFS, SEEK_SET);
        write(fd, buffer, DATA_AR_SZ);
#elif defined(_UBOOT_)
	read_device(buffer, NOR_DATA_OFS, DATA_AR_SZ);
	write_device(buffer, BAC_DATA_OFS, DATA_AR_SZ);
#endif
    }

    free(buffer);
#ifdef _ANDROID_
    close(fd);
#endif
    return 0;
}
#endif
