#define	WAVEFORM_BASE	0x75a00000
#define WAVEFORM_RESERVED_SIZE	0x300000
#define CRC_BASE 		(WAVEFORM_BASE + WAVEFORM_RESERVED_SIZE - sizeof(unsigned long ))	

struct jerry_waveform_t
{
	unsigned char * waveform;
	unsigned long *checksum;
};

typedef struct jerry_waveform_t	jerry_waveform_t;
