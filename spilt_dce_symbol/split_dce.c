#include "split_dce.h"

static binary_header_t  s_g_binary_common_head_info;

void
set_log_file_len(log_info_t *handle, unsigned int buf_size)
{
	handle->file_len = buf_size;
}

void 
set_binary_common_head(binary_header_t *original_info)
{
	s_g_binary_common_head_info = *original_info;
}

void
get_binary_common_head(char *addr)
{
	binary_header_t *set_head = (binary_header_t *)(addr);
	
	set_head->type = s_g_binary_common_head_info->type;
	set_head->type_len = s_g_binary_common_head_info->type_len;
}


int 
split_create(quote_struct_t *quote_file, char *symbol, char *quote)
{
	char *quote_type_name[] = { NULL, "_best_and_deep", "_match_price", "_order_statistic", "_realtime_price", "_ten_entrust"};

	/* malloc some memory store split contract info, default 10M for every contract*/
	quote_file->mmap_addr = calloc(1, MAX_SYM_SIZE);
	if (quote_file->mmap_addr == NULL) {
		printf("[ERROR]-[MEMORY]-[FAIL] FILE:%s ,LINE:%d, FUNC:%s \n",
				__FILE__,
				__LINE__,
				__FUNCTION__);
		return -1;
	}

	strncpy(quote_file->symbol, symbol, sizeof(quote_file->symbol));
	get_binary_common_head(quote_file->mmap_addr);
	quote_file->cur_write_offset += BINARY_HEAD_CNT;
}

int 
split_append(quote_struct_t  *quote_file)
{

}


int 
parser_quote_header(split_node_t *node , char *quote_path, binary_header_t *binary_info)
{
	int idx = 0;
	
	node->orginal_node.fd = open(path, O_RDWR);
	if (-1 == node->orginal_node.fd) 
	{  
		printf("[ERROR] :open(%s) got error, reason: %s! FILE:%s ,LINE:%d, FUNC:%s\n", 
				path,
				strerror(errno),
				__FILE__,
				__LINE__,
				__FUNCTION__);

		return -1;
	}
	
	node->orginal_node.file_len = lseek(node->orginal_node.fd, 0, SEEK_END);
	
	node->orginal_node.mmap_addr= mmap(NULL, node->orginal_node.file_len, PROT_READ | PROT_WRITE, MAP_SHARED, handle->fd, 0);
	if (MAP_FAILED == node->orginal_node.mmap_addr)
	{
		close(node->orginal_node.fd);
		printf("[ERROR]: mmap() got error, reason:%s ! FILE:%s ,LINE:%d, FUNC:%s\n",
				strerror(errno),
				__FILE__,
				__LINE__,
				__FUNCTION__);
		return -1;
	}
	
	*binary_info = *(binary_header_t *)(node->orginal_node.mmap_addr);
	node->orginal_node.cur_write_offset = sizeof(binary_header_t);
	
	return 0;
}

char *
get_symbol(quote_struct_t *quote)
{
	switch(quote->quote_head.type) {
		case MI_DCE_BEST_DEEP: 
		{
			break;
		}
		case  MI_DCE_MARCH_PRICE: 
		{
			break;
		}
		case MI_DCE_ORDER_STATISTIC:
		{
			break;
		}
		case MI_DCE_REALTIME_PRICE:
		{
			break;
		}
		case MI_DCE_TEN_ENTRUST: 
		{
			break;
		}
	}
}

unsigned int 
calculate_hash_key(char *symbol)
{	
	unsigned int hash_key;

	/*dce symbol like: a1609, cs1609, according to current contract designed max len 7bytes*/
	if (1 != symbol[TEST_SYM_IDX]) {
		/*it means just like cs1609 */
		hash_key = atoi(symbol)*100 + symbol[0];
	} else {
		hash_key = atoi(symbol)*10000 + symbol[0] + symbol[1];
	}

	return hash_key;
}

int 
get_hash_table_idx(unsigned int hash_key)
{
	return (hash_key%PRIME_NUM);
}

static unsigned int 
get_best_and_deep(char  *quote, char *symbol)
{
	unsigned int hash_key = 0;
	char *tmp = ((struct dce_my_best_deep_quote *)(quote->mmap_addr + quote->cur_write_offset))->Contract;
	
	strncpy(symbol, tmp, MAX_SYMBOL_LEN);

	hash_key = calculate_hash_key(symbol);

	return get_hash_table_idx(hash_key);
}

typedef unsigned int(*process_quote_symbol_t) (char  *quote, char *symbol);

static process_quote_symbol_t calculate_symbol_func_ar[] = 
{
	NULL,
	get_best_and_deep_hash_key,
	get_march_price_hash_key,
	get_order_statistic_hash_key,
	get_realtime_price_hash_key,
	get_ten_entrust_hash_key,
}

char *
get_current_quote_addr(quote_struct_t *quote)
{
	quote->cur_write_offset += TIMESTAMP_LEN;
	return (quote->mmap_addr + quote->cur_write_offset);
}

int 
split_init(char *quote_path, split_node_t *node)
{
	int ret = 0, idx = 0, hash_idx = 0;
	binary_header_t 	binary_head_info;
	quote_struct_t  	*orginal_file;
	char *current_addr;
	char symbol[MAX_SYMBOL_LEN];
	memset(node, 0, sizeof(split_node_t));
	ret = parser_quote_header(node , quote_path, &binary_head_info);
	if (ret != 0){
		printf("[ERROR]: FILE:%s ,LINE:%d, FUNC:%s \n",
				__FILE__,
				__LINE__,
				__FUNCTION__);
	}

	/* orginal node store orginal binary quote file infomation*/
	orginal_file = &node->orginal_node;


	set_binary_common_head(binary_head_info);

	/* scanning original binary file */
	for (idx = 0; i < binary_head_info->sym_cnt; idx++) {

		current_addr = get_current_quote_addr(orginal_file);
		
		hash_idx = calculate_symbol_func_ar[binary_info->type](current_addr, symbol);
		if (node->node_ar[hash_idx].mmap_addr == NULL) {
			split_create(node->node_ar[hash_idx], symbol, current_addr);
		} else {
			split_append(node->node_ar[hash_idx], symbol, current_addr);
		}

	}


}