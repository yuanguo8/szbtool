#ifndef SZBTOOLS_H
#define SZBTOOLS_H
#define INFOSIZE 256
#define DEBUG 1
#define BUFF_SIZE 2048
#define CFG_FATBOOT (0x40000000)
typedef unsigned char u8;
typedef unsigned int u32;
typedef struct {
	u8 magic[8];
	u32 checksum;
	u32 filesize;
	u8 author[32];
	u8 version[32];
	u32 timestamp;
	u32 imagecount;
	u8 tools[32];
	u8 reserve[INFOSIZE-56];
}szb_header_t;

typedef struct{
	u8 filename[64];
	u8 partname[32];
	u32 checksum;
	u32 timestamp;
	u32 imageoffset;
	u32 imagesize;
	u32 eraseflag;
	u32 writeflag;
	u8 reserve[INFOSIZE-120];
}szb_images_t;

typedef struct
{
	szb_images_t *data;
	struct image_node *next;
}image_node;

typedef struct{
	u32 flag;
	u8 filename[64];
	u8 partname[32];
	u32 eraseflag;
	u32 writeflag;
}header_temp_t;
#endif
