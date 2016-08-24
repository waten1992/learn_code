#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h> 
#include <errno.h>
#include <string.h>
#include <sys/mman.h>

#define MAX_SYMBOL_LEN  (16)
#define MAX_SYM_IDX  	(256)
#define MAX_SYM_SIZE	(1<<24)
#define TIMESTAMP_LEN	(8)
#define TYPE_LEN 		(2)
#define TEST_SYM_IDX	(1)
#define PRIME_NUM  		(253) 

/*for speed use hash instead of travel compare */

typedef struct binary_header{
	unsigned short 	type;
	unsigned short  type_len;
	unsigned int 	sym_cnt;
}binary_header_t;


typedef struct quote_struct {
	unsigned int		hash_key;/*calcluate by symbol*/
	char 				*mmap_addr;
	unsigned int 		cur_write_offset;
	unsigned int 		file_len;
	int 				fd;
	quote_struct_t 		*next;
}quote_struct_t;

typedef struct split_node
{
	quote_struct_t 		node_ar[MAX_SYM_IDX];
	quote_struct_t 		orginal_node;
}split_node_t;


int 
split_init(char *quote_path, split_node_t *node);

int 
split_scan(split_node_t *node);

int 
split_destory(split_node_t *node);