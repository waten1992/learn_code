#include <stdio.h>
#include <malloc.h>
#include <sys/mman.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include "c_quote_dce_my.h"

#define DAY_START_TIME 		(85000000)
#define DAY_END_TIME   		(160000000)
#define NIGHT_START_TIME 	(205000000)
#define NIGHT_END_TIME 		(234000000)

#define DCE_MDBESTANDDEEP_QUOTE_TYPE    2
#define DCE_ARBI_QUOTE_TYPE             3
#define DCE_MDTENENTRUST_QUOTE_TYPE     4
#define DCE_MDORDERSTATISTIC_QUOTE_TYPE 5
#define DCE_MDREALTIMEPRICE_QUOTE_TYPE  6
#define DCE_MDMARCHPRICEQTY_QUOTE_TYPE  7

typedef struct binary_header {
	unsigned int		tot_cnt;
	unsigned short		type;
	unsigned short		quote_type_len;
}binary_header_t;


typedef struct quote_struct 
{
	char 				*mmap_addr;
	unsigned int 		cur_write_offset;
	unsigned int 		file_len;
	int 				fd;
	binary_header_t     binary_head;
}quote_struct_t;

static inline unsigned int turing_time_convert(char *src)
{
	unsigned int t;
	/* src like: HH:MM:SS.mmm 10:23:10.123 */
	t = src[0] - 48;
	t = t * 10;
	t = t + (src[1] - 48);
	t = t * 10;
	t = t + (src[3] - 48);
	t = t * 10;
	t = t + (src[4] - 48);
	t = t * 10;
	t = t + (src[6] - 48);
	t = t * 10;
	t = t + (src[7] - 48);
	t = t * 10;
	t = t + (src[9] - 48);
	t = t * 10;
	t = t + (src[10] - 48);
	t = t * 10;
	t = t + (src[11] - 48);

	return t;
}

int
open_file_init(char *path, quote_struct_t *quote_addr)
{
	memset(quote_addr, 0, sizeof(quote_struct_t));
	quote_addr->fd = open(path, O_RDWR);
	if (-1 == quote_addr->fd) 
	{  
		printf("[ERROR] :open(%s) got error, reason: %s! FILE:%s ,LINE:%d, FUNC:%s\n", 
				path,
				strerror(errno),
				__FILE__,
				__LINE__,
				__FUNCTION__);

		return -1;
	}
	
	quote_addr->file_len = lseek(quote_addr->fd, 0, SEEK_END);
	
	quote_addr->mmap_addr= mmap(NULL, quote_addr->file_len, PROT_READ | PROT_WRITE, MAP_SHARED, quote_addr->fd, 0);
	if (MAP_FAILED == quote_addr->mmap_addr)
	{
		close(quote_addr->fd);
		printf("[ERROR]: mmap() got error, reason:%s ! FILE:%s ,LINE:%d, FUNC:%s\n",
				strerror(errno),
				__FILE__,
				__LINE__,
				__FUNCTION__);
		return -1;
	}

	//*((binary_header_t *)(quote_addr->binary_head)) =  *((binary_header_t *)(quote_addr->mmap_addr));
	quote_addr->binary_head.type = ((binary_header_t *)(quote_addr->mmap_addr))->type;
	quote_addr->binary_head.quote_type_len = ((binary_header_t *)(quote_addr->mmap_addr))->quote_type_len;
	quote_addr->binary_head.tot_cnt = ((binary_header_t *)(quote_addr->mmap_addr))->tot_cnt;

	return 0;
}

int 
close_file(quote_struct_t *quote_addr)
{
	int ret = 0;

	if (quote_addr->mmap_addr == NULL) {
		ret = munmap(quote_addr->mmap_addr, quote_addr->file_len);
		if (-1 == ret) {
			printf("[ERROR]: FILE:%s ,LINE:%d, FUNC:%s got error\n",
					__FILE__,
					__LINE__,
					__FUNCTION__);
			close(quote_addr->fd);
			return -1;
		}

		quote_addr->mmap_addr = NULL;
		close(quote_addr->fd);
	}
	
}

unsigned long
get_file_tot_len(quote_struct_t *quote_addr)
{
		unsigned long tot_len;
		tot_len = (quote_addr->binary_head.tot_cnt * quote_addr->binary_head.quote_type_len) + sizeof(binary_header_t);
		return tot_len;
}

char * 
get_best_gentime_addr(quote_struct_t *quote_addr, char *gentime)
{
	return  ((struct dce_my_best_deep_quote *)(quote_addr->mmap_addr + quote_addr->cur_write_offset + sizeof(unsigned long)))->GenTime;
}

char * 
get_ten_gentime_addr(quote_struct_t *quote_addr ,char *gentime)
{
	return ((struct dce_my_ten_entr_quote *)(quote_addr->mmap_addr + quote_addr->cur_write_offset + sizeof(unsigned long)))->GenTime;
}



unsigned int 
check_quote_time(quote_struct_t *quote_addr, int type)
{
	unsigned int quote_time;
	char *GenTime = NULL;
	

	if (type == DCE_MDBESTANDDEEP_QUOTE_TYPE) 
		GenTime = get_best_gentime_addr(quote_addr , GenTime);
	else if (type == DCE_MDTENENTRUST_QUOTE_TYPE) 
		GenTime =  get_ten_gentime_addr(quote_addr , GenTime);
	else 
		printf("ELSE type :%d \n", type);

	quote_time = turing_time_convert(GenTime);
	
	return quote_time;
}


int 
main(int argc, char const *argv[])
{
	quote_struct_t quote_addr;
	int ret  = 0 , idx = 0;
	unsigned long mm_len = 0, cur_quote_len = 0;
	unsigned int quote_time;
	char  *new_quote;

	if(argc != 2 )
	{
		printf("please input: ./exec  xxx.dat \n");
		return -1;
	}

	ret = open_file_init(argv[1], &quote_addr);
	
	mm_len = get_file_tot_len(&quote_addr);

	new_quote = calloc(1, mm_len);
	if (new_quote == NULL){
		printf("[ERROR]: FILE:%s ,LINE:%d, FUNC:%s got error\n",
					__FILE__,
					__LINE__,
					__FUNCTION__);
		close_file(&quote_addr);
	}
	
	((binary_header_t *)new_quote)->type = ((binary_header_t *)(quote_addr.mmap_addr))->type;
	((binary_header_t *)new_quote)->quote_type_len = ((binary_header_t *)(quote_addr.mmap_addr))->quote_type_len;
	
	((binary_header_t *)new_quote)->tot_cnt = 0;
	cur_quote_len += sizeof(binary_header_t);
	
	int type =  quote_addr.binary_head.type;

	quote_addr.cur_write_offset += sizeof(binary_header_t);

	for (idx = 0; idx < quote_addr.binary_head.tot_cnt; idx++)
	{
		quote_time = check_quote_time(&quote_addr, type);
		if (quote_time >= DAY_START_TIME && quote_time <= DAY_END_TIME)
		{
			memcpy(new_quote+cur_quote_len, quote_addr.mmap_addr+ quote_addr.cur_write_offset, quote_addr.binary_head.quote_type_len);
			cur_quote_len += quote_addr.binary_head.quote_type_len;
			((binary_header_t *)new_quote)->tot_cnt += 1; 
		}else {
			printf("Filter time: %d \n",quote_time);
		}
		quote_addr.cur_write_offset += quote_addr.binary_head.quote_type_len;

	}
		
	FILE *save;
	save = fopen("output.txt","wr+");
	if (save == NULL) {
		printf("save is NULL");
		return 0;
	}

	fwrite(new_quote, cur_quote_len, 1,save);
	fclose(save);	
	printf("finish !\n");
	close_file(&quote_addr);

	return 0;
}
