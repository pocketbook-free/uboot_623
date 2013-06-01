#ifndef __ITEM_H
#define __ITEM_H

#ifdef __cplusplus
extern "C" {
#endif

int set_item(char *item, void *mem, unsigned int size, unsigned char type, unsigned char attr);
int get_item(char *item, void *mem);
int get_item_size(char *file_name);

int policy_format(void);
int init_global_buffer(void);
void free_global_buffer(void);

#ifdef __cplusplus
}
#endif

#endif


