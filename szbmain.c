#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include"szbtools.h"
extern char *optarg;
extern int optind,opterr,optopt;
extern int create_chain(unsigned int location,header_temp_t *header_temp,char *filename,char *partname);
extern int unpack_main(char *packname,image_node *header,szb_header_t *szb_header);
extern int pack_main(image_node *header,szb_header_t *szb_header,header_temp_t *header_temp,char new_pack_header,char *packname,char *author_name);
extern int get_szb_info(char *packname,image_node *header,szb_header_t *szb_header);
void disp_aut_info()///display author infomation;
{
	printf("..............................................................\n");
	printf(".                  Welcome to use leszb                      .\n");
	printf(".       This tool released by www.lephone.cc Forum           .\n");
	printf(".            BUG feedback to yuanguo@lephone.cc              .\n");
	printf(".                                      --Version:1.0         .\n");
	printf(".                                      --Author:木马男孩     .\n");
	printf("..............................................................\n");
}
void disp_help_info()//Usage
{
	printf("............................................................\n");
	printf(".Usage:                                                    .\n");
	printf(".  leszb -b ubootfile        # add uboot partition         .\n");
	printf(".  leszb -k kernelfile       # add kernel partition        .\n");
	printf(".  leszb -y recoveryfile     # add recovery partition      .\n");
	printf(".  leszb -s systemfile       # add system partition        .\n");
	printf(".  leszb -c cpimagefile      # add cpimage partition       .\n");
	printf(".  leszb -p preloadfile      # add preloadfile partition   .\n");
	printf(".  leszb -d datafile         # add userdata                .\n");
	printf(".  leszb -x szbfile          # extract szb file            .\n");
	printf(".  leszb -i szbfinfo         # szbfinfo                    .\n");
	printf(".  leszb -v newpackname      # setting Version             .\n");
	printf(".  leszb -a yourname         # yourname                    .\n");
	printf(".  leszb -e                  # erase cache                 .\n");
	printf(".  leszb -h                  # help                        .\n");
	printf(".  leszb -l                  # register                    .\n");
	printf("............................................................\n");
}

void displayhead(header_temp_t *header_temp)
{
	int i=0;
	for (i = 0;header_temp[i].flag==1; i++)
	{
		/* code */
	printf("%d\n",header_temp[i].flag);
	printf("%d\n",header_temp[i].writeflag);
	printf("%d\n",header_temp[i].eraseflag);
	printf("%s\n",header_temp[i].filename);
	printf("%s\n",header_temp[i].partname);
	}



}

int main(int argc,char *argv)
{	
	char c;
	unsigned int location=0;
	image_node *header;
	szb_header_t *szb_header;
	char new_pack_header[INFOSIZE][15];
	header_temp_t header_temp[15];
	char author_name[32];
	disp_aut_info();
	while(1)
	{
		c=getopt(argc,argv,"hb:k:y:s:c:p:x:d:e:a:v:i:");///options get
		if(c==EOF)
		{	
			if(argc<2)
			{
			printf("You enter the parameters error!!please see description:\n");
			disp_help_info();
			break;
			}
			else
			{
				break;
			}	
		} 
		switch(c)
		{
			case 'e':
			create_chain(location++,header_temp,optarg,"cache");
			break;

			case 'a':
			strncpy(author_name,optarg,32);
			break;

			case 'h':
			disp_help_info();
			break;

			case 'l':
			break;

			case 'b':
			create_chain(location++,header_temp,optarg,"bootloader");
			//nt create_chain(unsigned int location,char *new_pack_header,char *packname,image_node *header,szb_header_t *szb_header);
			break;

			case 'k':
			create_chain(location++,header_temp,optarg,"boot");
			break;

			case 'y':
			create_chain(location++,header_temp,optarg,"recovery");
			break;

			case 's':
			create_chain(location++,header_temp,optarg,"system");
			break;

			case 'd':
			create_chain(location++,header_temp,optarg,"userdata");
			break;

			case 'c':
			create_chain(location++,header_temp,optarg,"cpimage");
			break;

			case 'p':
			create_chain(location++,header_temp,optarg,"preload");
			break;

			case 'x':
			unpack_main(optarg,header,szb_header);
			break;

			case 'v':
			printf("start pack new szbpack!!\n");
			printf("lephone.cc provide !!\n");
			pack_main(header,szb_header,header_temp,new_pack_header, optarg,author_name);
			///pack_main(image_node *header,szb_header_t *szb_header,header_temp_t *header_temp,char *new_pack_header,char *packname);
			printf("pack szb is end!\n");
			break;

			case 'i':
			get_szb_info(optarg,header,szb_header);
			break;

			default:
			printf("You enter the parameters error!!please see description:\n");
			disp_help_info();
			exit(1);
		}

	}
	return 0;
}

