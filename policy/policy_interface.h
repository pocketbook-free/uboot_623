/*
 * =====================================================================================
 *
 *       Filename:  policy_interface.h
 *
 *    Description:  interfaces for AP 
 *
 *        Version:  1.0
 *        Created:  05/06/2011 05:32:29 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Morgan 
 *        Company:  TMSBG-CDPG-RDSW
 *
 * =====================================================================================
 */
#ifndef __POLICY_INTERFACE_H
#define __POLICY_INTERFACE_H

#define MACH_ID         "mach_ID"
#define VCOM         	"VCOM"
#define WAVEFORM        "WAVEFORM"
#define DATA2           "data2"
#define DATA3           "data3"
#define CHARGE_LOGO     "chrg_logo"

#ifdef __cplusplus
extern "C" {
#endif

int policy_clear_all(void);
int policy_set_id(char *item, void *mem, int size);
int policy_get_id(char *item, void *mem);
int policy_store_item_data(char *item, void *mem, int size);
int policy_get_item_data(char *item, void *mem);
int policy_get_item_size(char *item);

#ifdef __cplusplus
}
#endif

#endif

