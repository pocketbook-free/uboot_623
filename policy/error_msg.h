#ifdef __cplusplus
extern "C" {
#endif

/* Error return value 
 * Each return value is unique.
 */
#define BOTH_NOT_VALID		-1
#define ITEM_NOT_FOUND		-2
#define ITEM_WRITE_DENY		-3
#define NO_ENOUGH_SPACE		-4
#define TOO_LONG_FILENAME   -5
#define ITEM_SIZE_WRONG     -6

#define MANAGE_AREA_ERROR       -10
#define INDEX_AREA_CRC_ERROR    -11
#define INFO_AREA_CRC_ERROR     -12
#define ITEM_INFO_CRC_ERROR     -13
#define ITEM_CONTENT_CRC_ERROR  -14

#define MALLOC_ERROR        -20
#define OPEN_DEVICE_ERROR   -21

#ifdef __cplusplus
}
#endif
