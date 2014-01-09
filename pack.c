#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/time.h>
#include"szbtools.h"
extern void display(image_node *header);
int	pack_szb_image(int new_szb_file, szb_header_t *szb_header,image_node *header,char *packname);
int create_chain(unsigned int location,header_temp_t *header_temp,char *filename,char *partname)
{
	header_temp[location].flag=1;
	header_temp[location].writeflag=1;
	header_temp[location].eraseflag=1;
	strncpy(header_temp[location].filename,filename,64);
	strncpy(header_temp[location].partname,partname,32);
	header_temp[location+1].flag=0;
	return 0;
}
int pack_main(image_node *header,szb_header_t *szb_header,header_temp_t *header_temp,char new_pack_header,char *packname,char *author_name)
{
	int i;
	int new_szb_file;
	/*
	typedef struct {
	u8 magic[8];
	u32 checksum;
	u32 filesize;
	u8 author[32];
	u8 version[32];
	u32 timestamp;
	u32 imagecount;
	u8 reserve[INFOSIZE-88];
	}szb_header_t;
	*/
	szb_header=(szb_header_t *)malloc(sizeof(szb_header_t));
	strncpy(szb_header->magic,"LmSzBfMt",8);
	strncpy(szb_header->author,author_name,32);
	strncpy(szb_header->version,"[www.lephone.cc]",16);
	strcat(szb_header->version,packname);
	strncpy(szb_header->tools,"木马男孩leszb 1.0",32);
	szb_header->imagecount=0;
	header=(image_node *)malloc(sizeof(image_node));
	header->data=(szb_images_t *)malloc(sizeof(szb_images_t));
	header->next=NULL;
	image_node *prefix=header,*suffix=header;
	for (i = 0; header_temp[i].flag==1; i++)
	{
		strncpy(prefix->data->partname,header_temp[i].partname,32);
		strncpy(prefix->data->filename,header_temp[i].filename,64);
		prefix->data->writeflag=header_temp[i].writeflag;
		prefix->data->eraseflag=header_temp[i].eraseflag;
		prefix=(image_node *)malloc(sizeof(image_node));
		prefix->data=(szb_images_t *)malloc(sizeof(szb_images_t));
		suffix->next=prefix;
		prefix->next=NULL;
		suffix=prefix;
	}
	szb_header->imagecount=i;
	////display(header);
	if((new_szb_file = open(packname,O_RDWR|O_CREAT|O_TRUNC,0777))<0)
	{
		printf("%s file creat faill!\n",packname);
		exit(1);
	}
	pack_szb_image(new_szb_file,szb_header,header,packname);
	return 0;
}

unsigned int get_file_size(char *file_path)
{
	unsigned int filesize = -1;
	struct stat statbuff;
	if (stat(file_path,&statbuff)<0)
	{
		return filesize;
	}
	else
	{
		filesize=statbuff.st_size;
	}
	return filesize;
}
time_t get_system_time()
{
	time_t t,ptr;
	t=localtime(&ptr);
	return t;
}
unsigned int get_crc_checksum(int new_szb_file,image_node *prefix,char *filename)
{
	int src_path;
	unsigned int checksum=0,i;
	char str_buff[BUFF_SIZE];
	unsigned int *temp_buff;
	unsigned int set;
	if((src_path=open(prefix->data->filename,O_RDWR,0666))<0)
	{
		printf("creat %s fail!\n",filename);
		exit(1);
	}
	if((set=lseek(new_szb_file,prefix->data->imageoffset,SEEK_SET))<0)
	{
		printf("read fail!!\n");
		exit(1);
	}
	if((read(src_path,str_buff,BUFF_SIZE)<0))
	{
		printf("read fail!\n");
		exit(1);
	}

	if(write(new_szb_file,str_buff,BUFF_SIZE)<0)
	{
		printf("pack %s part is Error!\n",filename);
		exit(1);
	};

	temp_buff=(unsigned int *)str_buff;
	for (i = 0; i <prefix->data->imagesize/sizeof(unsigned int); i++)
	{
		checksum+=*(temp_buff);
		temp_buff++;
		if((i+1)%(BUFF_SIZE/sizeof(unsigned int ))==0)
		{
			if(read(src_path,str_buff,BUFF_SIZE)<0)
			{
				printf("read fail!\n");
				exit(1);
			}
			if(write(new_szb_file,str_buff,BUFF_SIZE)<0)
			{
				printf("pack %s part is Error!\n",filename);
				exit(1);
			};
		temp_buff=(unsigned int *)str_buff;
		}

	}
	close(src_path);
	strncpy(prefix->data->filename,filename,32);
	return checksum;
}
unsigned int header_crc_checksum(int new_szb_file,unsigned int start_offset,unsigned int size)
{
	unsigned int checksum=0,i;
	char str_buff[BUFF_SIZE];
	unsigned int *temp_buff;
	int set;
	if((set=lseek(new_szb_file,start_offset,SEEK_SET))<0)
		{
			printf("read fail!!\n");
			exit(1);
		};
	if((read(new_szb_file,str_buff,BUFF_SIZE)<0))
		{
			printf("read fail!\n");
			exit(1);
		}
	temp_buff=(unsigned int *)str_buff;
	for (i = 0; i <size; i++)
	{
		/* code */
		checksum+=*(temp_buff);
		temp_buff++;
		if((i+1)%(BUFF_SIZE/sizeof(unsigned int ))==0)
		{
			if(read(new_szb_file,str_buff,BUFF_SIZE)<0)
			{
			printf("read fail!\n");
			exit(1);
			}
		temp_buff=(unsigned int *)str_buff;

		}

	}
	return checksum;
}

int	pack_szb_image(int new_szb_file, szb_header_t *szb_header,image_node *header,char *packname)
{
	unsigned int i,set;
	image_node *prefix=header;
	unsigned int start_offset=8192;
	for(i=0;prefix->next!=NULL;i++)
	{
		if (memcmp(prefix->data->partname,"bootloader",10)==0)
		{
			printf("start pack uboot.bin......\n");
			prefix->data->imageoffset=start_offset;
			prefix->data->imagesize = get_file_size(prefix->data->filename);
			prefix->data->checksum = get_crc_checksum(new_szb_file,prefix,"uboot.bin");
			prefix->data->timestamp = get_system_time();
			start_offset +=((prefix->data->imagesize/16)+5)*16;
			prefix=prefix->next;
			continue;
		}
		if (memcmp(prefix->data->partname,"boot",4)==0)
		{
			printf("start pack boot.img......\n");
			prefix->data->imageoffset=start_offset;
			prefix->data->imagesize = get_file_size(prefix->data->filename);
			prefix->data->checksum = get_crc_checksum(new_szb_file,prefix,"boot.img");
			prefix->data->timestamp = get_system_time();
			start_offset +=((prefix->data->imagesize/16)+5)*16;
			prefix=prefix->next;
			continue;
		}
		if (memcmp(prefix->data->partname,"recovery",8)==0)
		{
			printf("start pack recovery.img......\n");
			prefix->data->imageoffset=start_offset;
			prefix->data->imagesize = get_file_size(prefix->data->filename);
			prefix->data->checksum = get_crc_checksum(new_szb_file,prefix,"recovery.img");
			prefix->data->timestamp = get_system_time();
			start_offset +=((prefix->data->imagesize/16)+5)*16;
			prefix=prefix->next;
			continue;
		}
		if (memcmp(prefix->data->partname,"system",6)==0)
		{
			printf("start pack system.img\n");
			prefix->data->imageoffset=start_offset;
			prefix->data->imagesize = get_file_size(prefix->data->filename);
			prefix->data->checksum = get_crc_checksum(new_szb_file,prefix,"system.img");
			prefix->data->timestamp = get_system_time();
			start_offset +=((prefix->data->imagesize/16)+5)*16;
			prefix=prefix->next;
			continue;
		}

		if (memcmp(prefix->data->partname,"userdata",8)==0)
		{
			printf("start pack userdata.img\n");
			prefix->data->imageoffset=start_offset;
			prefix->data->imagesize = get_file_size(prefix->data->filename);
			prefix->data->checksum = get_crc_checksum(new_szb_file,prefix,"userdata.img");
			prefix->data->timestamp = get_system_time();
			start_offset +=((prefix->data->imagesize/16)+5)*16;
			prefix=prefix->next;
			continue;
		}

		if (memcmp(prefix->data->partname,"cpimage",7)==0)
		{
			printf("start pack cpimage.img ......\n");
			prefix->data->imageoffset=start_offset;
			prefix->data->imagesize = get_file_size(prefix->data->filename);
			prefix->data->checksum = get_crc_checksum(new_szb_file,prefix,"cpimage.img");
			prefix->data->timestamp = get_system_time();
			start_offset +=((prefix->data->imagesize/16)+5)*16;
			prefix=prefix->next;
			continue;
		}
		if (memcmp(prefix->data->partname,"preload",7)==0)
		{
			printf("start pack preload.img......\n");
			prefix->data->imageoffset=start_offset;
			prefix->data->imagesize = get_file_size(prefix->data->filename);
			prefix->data->checksum = get_crc_checksum(new_szb_file,prefix,"preload.img");
			prefix->data->timestamp = get_system_time();
			start_offset +=((prefix->data->imagesize/16)+5)*16;
			prefix=prefix->next;
			continue;
		}
		if (memcmp(prefix->data->partname,"userdata",8)==0)
		{
			printf("start  pack userdata.img......\n");
			prefix=prefix->next;
			continue;
		}
		if (memcmp(prefix->data->partname,"cache",5)==0)
		{
			printf("start pack  cache.img...... \n");
			prefix=prefix->next;
			continue;
		}
		if (memcmp(prefix->data->partname,"fat",5)==0)
		{
			printf("start pack  fat.img...... \n");
			prefix=prefix->next;
			continue;
		}
	}
	szb_header->timestamp=get_system_time();;
	szb_header->filesize=get_file_size(packname);

	if((set=lseek(new_szb_file,0,SEEK_SET))<0)
	{
		printf("read fail!!\n");
		exit(1);
	}

	if(write(new_szb_file,(char *)szb_header,256)<0)
	{
		printf("pack  header  is Error!\n");
		exit(1);
	};
	prefix=header;
	for (i = 0; i < szb_header->imagecount; i++)
	{
		if(write(new_szb_file,(char *)prefix->data,256)<0)
		{
			printf("pack  header  is Error!\n");
			exit(1);
		};
		prefix=prefix->next;
	}
	szb_header->checksum = header_crc_checksum(new_szb_file,12,(INFOSIZE*32-12)/sizeof(unsigned int));

	if((set=lseek(new_szb_file,0,SEEK_SET))<0)
	{
		printf("read fail!!\n");
		exit(1);
	}

	if(write(new_szb_file,(char *)szb_header,256)<0)
	{
		printf("pack  header crc   is Error!\n");
		exit(1);
	};

	close(new_szb_file);

	return 0;
}

image_node *show_szb_info(int szb_path,szb_header_t *szb_header)
{
	int i;
	image_node header_node,*header;
	image_node *prefix=&header_node,*suffix=&header_node;
	header=&header_node;
	szb_images_t *image_new;
	image_node *node_new;
	///debug
	for(i=0;i<szb_header->imagecount;i++)
	{	
		image_new=(szb_images_t *)malloc(sizeof(szb_images_t));
		if(read(szb_path,image_new,INFOSIZE)>=0)
			{
			}
			else
			{
				printf("read szbpack is fall\n");
				return 1;
			}
		prefix->data=image_new;
		prefix=(image_node *)malloc(sizeof(image_node));
		suffix->next=prefix;
		prefix->next=NULL;
		suffix=prefix;
	}
	//printf("read  szbpack is successed!\n\n");
	//printf("this pack contain  files as follows:\n");
	display(header);
	return header;
}
void display_head(char *packname,szb_header_t *szb_header)
{
	/*typedef struct {
	u8 magic[8];
	u32 checksum;
	u32 filesize;
	u8 author[32];
	u8 version[32];
	u32 timestamp;
	u32 imagecount;
	u8 reserve[INFOSIZE-88];
}szb_header_t;
*/
	//struct  tm *timenow;
	printf("Pack information :!\n");
	printf("..............................................\n");
	printf("CRC校验码  :%x\n",szb_header->checksum);
	printf("szb包大小  :%dB\n",szb_header->filesize);
	printf("szb作者    :%s\n",szb_header->author);
	printf("szb包版本  :%s\n",szb_header->version);
	printf("封包时间   :%x\n",szb_header->timestamp);
	printf("img包数量  :%d\n",szb_header->imagecount);
	printf("打包工具   :%s\n",szb_header->tools);
	printf("..............................................\n");
	
}
int get_szb_info(char *packname,image_node *header,szb_header_t *szb_header)
{
	char head_buff[INFOSIZE];
	int szb_path;
	int i;
	if((szb_path=open(packname,O_RDWR,0777))<0)
	{
		printf("open szbPack is faill!%d\n",szb_path);
		return 1;
	}

	if(read(szb_path,head_buff,INFOSIZE)<0)
	{
		printf("read head fail! exit \n");
		return 	1;	
	}
	
	szb_header=(szb_header_t *)head_buff;
	display_head(packname,szb_header);
	header=show_szb_info(szb_path,szb_header);
	close(szb_path);
	return 0;
}




