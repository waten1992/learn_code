#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h> 
#include <errno.h>
#include <string.h>
#include <sys/mman.h>

#define MAX_SYMBOL_LEN  (16)
#define MAX_SYM_IDX  	(256)
#define MAX_SYM_SIZE	(1<<24)

typedef struct binary_header{
	unsigned short 	type;
	unsigned short  type_len;
	unsigned int 	sym_cnt;
}binary_header_t;


typedef struct quote_struct {
	char				symbol[16];
	char 				*mmap_addr;
	int 				fd;
	binary_header_t 	quote_head;
}quote_struct_t;

typedef struct split_node
{
	quote_struct_t 		node_ar[MAX_SYM_IDX];
	unsigned int 		node_idx;
}split_node_t;


int 
split_init(char *quote_path, split_node_t *node);

int 
split_scan(split_node_t *node);

int 
split_destory(split_node_t *node);