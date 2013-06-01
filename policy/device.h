/*
 * =====================================================================================
 *
 *       Filename:  device.h
 *
 *    Description:  Provide hardware read and write for other modules
 *
 *        Version:  1.0
 *        Created:  05/09/2011 07:11:05 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Morgan
 *        Company:  TMSBG-CDPG-RDSW
 *
 * =====================================================================================
 */
#ifndef __DEVICE_H
#define __DEVICE_H

#ifdef __cplusplus
extern "C" {
#endif

int read_device(void *buffer, unsigned long offset, unsigned int size);
int write_device(const void *buffer, unsigned long offset, unsigned int size);
int recovery_fs(unsigned char flag);
int recovery_data(unsigned char flag);

#ifdef __cplusplus
}
#endif

#endif



