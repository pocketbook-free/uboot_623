#ifndef __POLICY_H
#define __POLICY_H

#include <asm/types.h>

//#define POLICY_DEV      "/dev/block/mmcblk0"
//#define POLICY_DEV      "/mnt/sdcard/sd_file"
#define EMPTY_OF_ITEM   0xFFFF


//#define POLICY_AREA_SZ		(20*1024*1024)		/* Total size of policy area */
#define POLICY_AREA_SZ		(3*1024*1024)		/* Total size of policy area */
//#define POLICY_SIZE		(POLICY_AREA_SZ/2)	/* Policy area divide into 2 part: normal, backup */
#define POLICY_SIZE			0	/* Policy area divide into 2 part: normal, backup */

/* Here we call normal or backup part "Policy Partition".
 * Policy Partition can be divided into three part:
 * index area: save item link offset
 * infomation area: save infomation of each item
 * crc area: store crcs of index area and info area
 * data area: for save each item's real content
 * -----------------------------------------------------------------
 * |  index  |  info  | MNG & rsv |                 data                 |
 * -----------------------------------------------------------------
 */
#define DATA_UNIT_SZ		512	/* 256 < 8M/512B=16384 < 65535, so use __u16 as offset */
#define DATA_AR_SZ		(2*1024*1024)
#define DATA_UNIT_NR		(DATA_AR_SZ/DATA_UNIT_SZ)

#define INDEX_UNIT_SZ		2	/* offset of index unit point to related DATA_UNIT  */
#define INDEX_AR_SZ		(DATA_UNIT_NR*INDEX_UNIT_SZ) /* 16384*2=32K */

#define ITEM_PREDEF		0x1
#define ITEM_WRITE		0x2

#define STRING_TYPE     0x01
#define BINARY_TYPE     0x02

#define NORMAL          0x01
#define BACKUP          0x02

typedef struct {
    char    name[11];		/*  0~ a: Name and extension */
    __u8    attr;			/*     b: Attribute bits */
    __u8    type;			/*     c: reserved */
    __u8    rsv[3];			/* 19~1b: reserved */
    __u16   start;			/*  d~ e: start data unit offset */
    __u16   rsv2;			/*  f~10: reserved */
    __u32   size;			/* 11~14: File size in bytes */
    __u32   crc;			/* 15~18: item crc */
    __u32	info_crc;		/* 1c~1f: crc of this struct */
} item_info_t;
#define INFO_UNIT_SZ		(sizeof(item_info_t))	/* sizeof item_info_t */
#define INFO_AR_SZ		(DATA_UNIT_NR*INFO_UNIT_SZ)	/* 512K */
#define INFO_AR_OFS         (INDEX_AR_SZ)

/* crc for check validity */
typedef struct {
    __u32 crc;		/* crc of this struct */
    __u32 index_crc;	/* crc of index area */
    __u32 info_crc;		/* crc of info area */
    __u16 info_bdr;
} manage_t;
#define CRC_AR_SZ		(1024*1024-INDEX_AR_SZ-INFO_AR_SZ)
#define CRC_AR_OFS		(INDEX_AR_SZ+INFO_AR_SZ)

#define NORMAL_PAR_OFS	(11266*512)	/* noraml policy partiton offset */
//#define NORMAL_PAR_OFS	(0*512)	/* noraml policy partiton offset */
#define BACKUP_PAR_OFS	(NORMAL_PAR_OFS+POLICY_SIZE)	/* backup policy partiton offset */

/* offset of index area */
#define NOR_INDEX_OFS	NORMAL_PAR_OFS
#define BAC_INDEX_OFS	BACKUP_PAR_OFS

/* offset of infomation area */
#define NOR_INFO_OFS	(NOR_INDEX_OFS+INDEX_AR_SZ)
#define BAC_INFO_OFS	(BAC_INDEX_OFS+INDEX_AR_SZ)

/* offset of infomation area */
#define NOR_CRC_OFS	(NOR_INFO_OFS+INFO_AR_SZ)
#define BAC_CRC_OFS	(BAC_INFO_OFS+INFO_AR_SZ)

/* offset of data area */
#define NOR_DATA_OFS	(NOR_CRC_OFS+CRC_AR_SZ)
#define BAC_DATA_OFS	(BAC_CRC_OFS+CRC_AR_SZ)

#define FORMAT_SZ       (NOR_DATA_OFS-NOR_INDEX_OFS)

/* globle buffer pointer */
extern __u16 *index_buf;
extern item_info_t *info_buf;
extern manage_t *manage_buf;

#endif
