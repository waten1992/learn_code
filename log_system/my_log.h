/*
** Author: HWG
** Date:2016-08-10
** Description: 
**			1- simple log file system for signal thread use
** 			2- use INFO an ERROR level info
*/


#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h> 
#include <errno.h>
#include <string.h>
#include <sys/mman.h>



typedef struct log_info
{
	int 			fd;	
	char 			*mmap_addr;
	unsigned int 	file_len;
	unsigned int 	offset;
}log_info_t;

void
set_log_file_len(log_info_t *hanle, unsigned int buf_size);

int 
log_init(char *path, log_info_t *hanle);

void
log_info(log_info_t *handle, char *content);

void 
log_error(log_info_t *handle, char *content);

int 
log_destory(log_info_t *handle);
