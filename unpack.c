#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include"szbtools.h"
const u8 SZB_MAGIC[]="lmSzBfMt";
///extern displayInfo();
void display(image_node *header);
int unpack(int szb_path,szb_header_t *szb_header,image_node *header);
image_node *unpackSzb(int szb_path,szb_header_t *szb_header);

unsigned int calcChecksum(unsigned int *data,unsigned int size)
{
	unsigned int i,checksum=0;
	for (i = 0; i < size/sizeof(unsigned int); i++)
	{
		checksum+=*(data+i);
	}
	return checksum;

}
int  uboot_calcChecksum(int szb_path,image_node *prefix)
{
	char buff[1048576];
	printf("%x\n", prefix->data->checksum);
	lseek(szb_path,prefix->data->imageoffset,SEEK_SET);
	read(szb_path,buff,prefix->data->imagesize);
	printf("%x\n",calcChecksum((unsigned int *)buff,prefix->data->imagesize));
	return 0;
}

int displayInfo(int szb_path,szb_header_t *szb_header)
{
		char str[8192];
		lseek(szb_path,0,SEEK_SET);
		read(szb_path,str,8192);
		//printf("%s\n",szb_header->magic);
		printf("checksum is :%x\n",szb_header->checksum);
		printf("%d\n",szb_header->filesize);
		//printf("%s\n",szb_header->author);
		//printf("%s\n",szb_header->version);
		printf("%x\n",szb_header->timestamp);
		printf("%d\n",szb_header->imagecount);
		///printf("%d\n",sizeof(unsigned int));
		int off_set=CFG_FATBOOT;
		printf("%x\n",calcChecksum((unsigned int *)(str+12),8192-12));
		printf("%d\n",sizeof(szb_header_t));

}
int unpack_main(char *packname,image_node *header,szb_header_t *szb_header)
{
	char head_buff[INFOSIZE];
	int szb_path;
	int i;
	printf("start unpack szbPack!\n");

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
	
	///displayInfo();
	////analysis header data;
	/*if(memcmp(szb_header->magic,SZB_MAGIC,8)!=0)
	{
		printf("sorry!szbPack Error!\n");
		exit(1);
	}
	*/
	szb_header=(szb_header_t *)head_buff;
	printf("images count is :%d\n",szb_header->imagecount);
	header=unpackSzb(szb_path,szb_header);
	//displayInfo(szb_path,szb_header);
	printf("unpack is end!!\n" );
	close(szb_path);
	return 0;
}

void display(image_node *header)
{
	image_node *prefix=header;
	while(prefix->next!=NULL)
	{
		printf("..............................................\n");
		printf("%s.img infmation:\n", prefix->data->partname);
		printf("CRC校验码  :%x\n", prefix->data->checksum);
		printf("偏移位置   :ox%x\n", prefix->data->imageoffset);
		printf("分区大小   :%dB\n", prefix->data->imagesize);
		printf("是否擦除   :%d\n", prefix->data->eraseflag);
		prefix=prefix->next;
	}
	printf("\n");
}

int unpack_image(int szb_path,image_node *temp_header,char *partname)
{
	int image_part=0,i=0;
	char str_buff[BUFF_SIZE];
	unsigned int image_tail=(temp_header->data->imagesize)%BUFF_SIZE;
	unsigned int image_size=(temp_header->data->imagesize)/BUFF_SIZE;
	off_t set;
	if((image_part=open(partname,O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR))>=0){
		printf("%s creat is successed \n", partname);
	}
	else
	{
		printf("creat %s fail \n",partname);
		return 1;
	}
	set=lseek(szb_path,temp_header->data->imageoffset,SEEK_SET);
	//printf("%x\n",set );
	printf("%s part ddress is :%x\n",partname,temp_header->data->imageoffset);
	printf("%s part size is :%d\n",partname,temp_header->data->imagesize);

	for(i=0;i<image_size;i++)
	{
		if(read(szb_path,str_buff,BUFF_SIZE)>=0)
			{
				if(write(image_part,str_buff,BUFF_SIZE)<0)
					{
						printf("unpack %s part is Error!\n",partname);
					};
					
			};
		//fprintf(image_part, "%s\n",str_buff );
	}
	for (i = 0; i < image_tail; i++)
	{
		if(read(szb_path,str_buff,1)>=0)
			{
				if(write(image_part,str_buff,1)<0)
					{
						printf("unpack %s part is Error!\n",partname);
					};
			};
	}
	printf("%s part unpack is successed!\n", partname);
	close(image_part);
	return 0;
}

unsigned int crc_checksum(int szb_path,image_node *temp_header,char *partname)
{
	unsigned int checksum=0,i;
	char str_buff[BUFF_SIZE];
	unsigned int *temp_buff;
	int set;
	if((set=lseek(szb_path,temp_header->data->imageoffset,SEEK_SET))<0)
		{
			printf("read fail!!\n");
			exit(1);
		};
	if((read(szb_path,str_buff,BUFF_SIZE)<0))
		{
			printf("read fail!\n");
			exit(1);
		}
	temp_buff=(unsigned int *)str_buff;
	for (i = 0; i <temp_header->data->imagesize/sizeof(unsigned int); i++)
	{
		/* code */
		checksum+=*(temp_buff);
		temp_buff++;
		if((i+1)%(BUFF_SIZE/sizeof(unsigned int ))==0)
		{
			if(read(szb_path,str_buff,BUFF_SIZE)<0)
			{
			printf("read fail!\n");
			exit(1);
			}
		temp_buff=(unsigned int *)str_buff;

		}

	}
	if (checksum==temp_header->data->checksum)
			{
				/* code */
				printf("%s CRC check is OK !\n",partname);
			}
		else
		{
			printf("%s CRC check is fail !\n",partname);
			exit(1);
		}		
	return checksum;
}

int unpack(int szb_path,szb_header_t *szb_header,image_node *header)
{
	unsigned int i;
	image_node *prefix=header;
	for (i = 0; i <szb_header->imagecount; ++i)
	{

		if (memcmp(prefix->data->partname,"bootloader",10)==0)
		{
			printf("start unpack uboot.bin......\n");
			crc_checksum(szb_path,prefix,"uboot.img");
			unpack_image(szb_path,prefix,"uboot.bin");
			prefix=prefix->next;
			continue;
		}
		if (memcmp(prefix->data->partname,"boot",4)==0)
		{
			printf("start unpack boot.img......\n");
			crc_checksum(szb_path,prefix,"boot.img");
			unpack_image(szb_path,prefix,"boot.img");
			prefix=prefix->next;
			continue;
		}
		if (memcmp(prefix->data->partname,"recovery",8)==0)
		{
			printf("start unpack recovery.img......\n");
			crc_checksum(szb_path,prefix,"recovery.img");
			unpack_image(szb_path,prefix,"recovery.img");
			prefix=prefix->next;
			continue;
		}
		if (memcmp(prefix->data->partname,"system",6)==0)
		{

			printf("start unpack system.img\n");
			crc_checksum(szb_path,prefix,"system.img");
			unpack_image(szb_path,prefix,"system.img");
			prefix=prefix->next;
			continue;
		}
		if (memcmp(prefix->data->partname,"cpimage",7)==0)
		{
			printf("start unpack cpimage.img ......\n");
			crc_checksum(szb_path,prefix,"cpimage.img");
			unpack_image(szb_path,prefix,"cpimage.img");
			prefix=prefix->next;
			continue;
		}
		if (memcmp(prefix->data->partname,"preload",7)==0)
		{
			printf("start unpack preload.img......\n");
			crc_checksum(szb_path,prefix,"preload.img");
			unpack_image(szb_path,prefix,"preload.img");
			prefix=prefix->next;
			continue;
		}
		if (memcmp(prefix->data->partname,"userdata",8)==0)
		{
			printf("start  analysis userdata.img......\n");
			unpack_image(szb_path,prefix,"userdata.img");
			prefix=prefix->next;
			continue;
		}
		if (memcmp(prefix->data->partname,"cache",5)==0)
		{
			printf("start analysis cache.img...... \n");
			unpack_image(szb_path,prefix,"cache.img");
			prefix=prefix->next;
			continue;
		}

	}
	return 0;
}

image_node *unpackSzb(int szb_path,szb_header_t *szb_header)
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
	printf("read  szbpack is successed!\n\n");
	printf("this pack contain  files as follows:\n");
	//display(header);
	unpack(szb_path,szb_header,header);
	return header;
}

