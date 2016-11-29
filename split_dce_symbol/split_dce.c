#include "split_dce.h"
#include "c_quote_dce_my.h"
#include "mi_type.h"


#define DCE_MDBESTANDDEEP_QUOTE_TYPE    2
#define DCE_ARBI_QUOTE_TYPE             3
#define DCE_MDTENENTRUST_QUOTE_TYPE     4
#define DCE_MDORDERSTATISTIC_QUOTE_TYPE 5
#define DCE_MDREALTIMEPRICE_QUOTE_TYPE  6
#define DCE_MDMARCHPRICEQTY_QUOTE_TYPE  7

#define DAY_START_TIME (85000000)
#define DAY_END_TIME   (160000000)

static binary_header_t  s_g_binary_common_head_info;


void 
set_binary_common_head(binary_header_t *original_info)
{
	s_g_binary_common_head_info = *original_info;
}

void
get_binary_common_head(char *addr)
{
	binary_header_t *set_head = (binary_header_t *)(addr);
	
	set_head->type = s_g_binary_common_head_info.type;
	set_head->type_len = s_g_binary_common_head_info.type_len;
}

unsigned short 
get_quote_type_len()
{
	return (s_g_binary_common_head_info.type_len);
}


unsigned short
get_quote_type()
{
	return (s_g_binary_common_head_info.type);
}

void 
update_binary_offset(quote_struct_t *quote_file)
{
	quote_file->cur_write_offset += get_quote_type_len();
}

void 
update_binary_symbol_cnt(quote_struct_t *quote_file)
{
	*(unsigned int *)quote_file->mmap_addr = *(unsigned int *)(quote_file->mmap_addr) + 1;
}

char *
get_current_quote_addr(quote_struct_t *quote)
{
	return (quote->mmap_addr + quote->cur_write_offset);
}

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


char * 
get_best_gentime_addr(char *quote, char *gentime)
{
	return  ((struct dce_my_best_deep_quote *)(quote))->GenTime;
}

char * 
get_ten_gentime_addr(char *quote ,char *gentime)
{
	return ((struct dce_my_ten_entr_quote *)(quote))->GenTime;
}

char *
get_arbi_gentime_addr(char *quote ,char *gentime)
{
	return ((struct dce_my_best_deep_arbi_quote *)(quote))->GenTime;
}

unsigned int 
check_quote_time(char *quote, int type)
{
	unsigned int quote_time;
	char *GenTime = NULL;
	
	if (type == DCE_MDBESTANDDEEP_QUOTE_TYPE) 
		GenTime = get_best_gentime_addr(quote , GenTime);
	else if (type == DCE_MDTENENTRUST_QUOTE_TYPE) 
		GenTime =  get_ten_gentime_addr(quote , GenTime);
	else if (type == DCE_ARBI_QUOTE_TYPE)
		GenTime = get_arbi_gentime_addr(quote, GenTime);
	else
		printf("ELSE type :%d \n", type);

	quote_time = turing_time_convert(GenTime);
	
	return quote_time;
}

void 
judge_over_curr_mem(quote_struct_t *quote_file, char *quote, int type_len)
{
	char *cp;

	if ((quote_file->cur_write_offset + type_len) >= MAX_SYM_SIZE) {
		cp = calloc(1, MAX_SYM_SIZE<<1);
		if(cp == NULL) {
			printf("[ERROR]-[MEMORY]-[FAIL] FILE:%s ,LINE:%d, FUNC:%s \n",
				__FILE__,
				__LINE__,
				__FUNCTION__);
		}

		memcpy(cp, quote_file->mmap_addr, quote_file->cur_write_offset);
		free(quote_file->mmap_addr);
		quote_file->mmap_addr = cp;	
	}
}

void
cp_quote_struct_and_update(quote_struct_t *quote_file, char *quote)
{
	/* copy orginal file info to new file */
	char 	*cur_addr;
	unsigned short type_len;
	unsigned int type, ret = 1, gentime = 0;

	type_len = get_quote_type_len();
	type = get_quote_type();
	if (type == DCE_MDBESTANDDEEP_QUOTE_TYPE || type == DCE_MDTENENTRUST_QUOTE_TYPE || type == DCE_ARBI_QUOTE_TYPE) {
		gentime = check_quote_time(quote + TIMESTAMP_LEN, type);
		if (gentime >= DAY_START_TIME && gentime <= DAY_END_TIME)
			ret = 1;
		else
			ret = 0;
	}

	if (ret) {
		judge_over_curr_mem(quote_file, quote, type_len);
		cur_addr = get_current_quote_addr(quote_file);
		memcpy(cur_addr, quote, type_len);
		update_binary_offset(quote_file);
		update_binary_symbol_cnt(quote_file);
	}
	
}

int 
split_create(quote_struct_t *quote_file, char *symbol, char *quote)
{
	char *cur_addr;
	unsigned int type_len;
	/* malloc some memory store split contract info, default 10M for every contract*/
	quote_file->mmap_addr = calloc(1, MAX_SYM_SIZE);
	if (quote_file->mmap_addr == NULL) {
		printf("[ERROR]-[MEMORY]-[FAIL] FILE:%s ,LINE:%d, FUNC:%s \n",
				__FILE__,
				__LINE__,
				__FUNCTION__);
		return -1;
	}
	/*set binary header offset info*/
	quote_file->cur_write_offset += BINARY_HEAD_CNT;
	
	strncpy(quote_file->symbol, symbol, sizeof(quote_file->symbol));
	get_binary_common_head(quote_file->mmap_addr);

	cp_quote_struct_and_update(quote_file, quote);
	
	return 0;
}

int 
split_append(quote_struct_t *quote_file, char *symbol, char *quote)
{
	int ret = 0;
	quote_struct_t *tmp_p, *new_quote_node;
	tmp_p =  quote_file;
	/*select the same contract */
	for ( ;tmp_p->next != NULL ; tmp_p = tmp_p->next ) {
		if (!strcmp(tmp_p->symbol, symbol)) {
			ret = 1;
			break;
		}
	} 
	/* ret != 1 , it's means need create new node*/
	if(ret != 1) {
		new_quote_node = calloc(1, sizeof(quote_struct_t));
		if (new_quote_node == NULL) {
			printf("[ERROR]-[MEMORY]-[FAIL] FILE:%s ,LINE:%d, FUNC:%s \n",
				__FILE__,
				__LINE__,
				__FUNCTION__);
			return -1;
		}
		
		ret = split_create(new_quote_node, symbol, quote);
		if (ret != 0) {
			printf("[ERROR]-[CREATE]-[FAIL] FILE:%s ,LINE:%d, FUNC:%s \n",
				__FILE__,
				__LINE__,
				__FUNCTION__);
			return -1;
		}
		tmp_p->next = new_quote_node;
		cp_quote_struct_and_update(new_quote_node, quote);
	}else {
		cp_quote_struct_and_update(tmp_p, quote);		
	} 
	
	return 0;
}


int 
parser_quote_header(split_node_t *node , char *quote_path, binary_header_t *binary_info)
{
	int idx = 0;
	
	node->orginal_node.fd = open(quote_path, O_RDWR);
	if (-1 == node->orginal_node.fd) 
	{  
		printf("[ERROR] :open(%s) got error, reason: %s! FILE:%s ,LINE:%d, FUNC:%s\n", 
				quote_path,
				strerror(errno),
				__FILE__,
				__LINE__,
				__FUNCTION__);

		return -1;
	}
	
	node->orginal_node.file_len = lseek(node->orginal_node.fd, 0, SEEK_END);
	
	node->orginal_node.mmap_addr= mmap(NULL, node->orginal_node.file_len, PROT_READ | PROT_WRITE, MAP_SHARED, node->orginal_node.fd, 0);
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

unsigned int
calculate_arbi_hash_key(char *symbol)
{
	unsigned int hash_key;
	int idx = 0;

	while (symbol[idx] != '\0' ) {
		hash_key += symbol[idx];
		idx++;
		
		/*use '_' replace  ' ' (data link escape character) ,SP a1701&m1705 ----> SP_a1701&m1705 */
		if ( *(char *)(symbol+idx) == 32 ) {
			symbol[idx] = '_';
		}
		/*use '_' replace  '&' ,SP_a1701&m1705 ----> SP_a1701_m1705 */
		if( *(char *)(symbol+idx) == 38) {
			symbol[idx] = '_';
		}
	}

	return hash_key;
}

int 
get_hash_table_idx(unsigned int hash_key)
{
	return (hash_key%PRIME_NUM);
}

static unsigned int 
get_best_and_deep_hash_key(char  *quote, char *symbol)
{
	unsigned int hash_key = 0;
	char *tmp = ((struct dce_my_best_deep_quote *)(quote + TIMESTAMP_LEN))->Contract;
	
	strncpy(symbol, tmp, MAX_SYMBOL_LEN);

	hash_key = calculate_hash_key(symbol);

	return get_hash_table_idx(hash_key);
}

static unsigned int 
get_march_price_hash_key(char  *quote, char *symbol)
{
	unsigned int hash_key = 0;
	char *tmp = ((struct dce_my_march_pri_quote *)(quote + TIMESTAMP_LEN))->ContractID;
	
	strncpy(symbol, tmp, MAX_SYMBOL_LEN);

	hash_key = calculate_hash_key(symbol);

	return get_hash_table_idx(hash_key);
}

static unsigned int 
get_order_statistic_hash_key(char  *quote, char *symbol)
{
	unsigned int hash_key = 0;
	char *tmp = ((struct dce_my_ord_stat_quote *)(quote + TIMESTAMP_LEN))->ContractID;
	
	strncpy(symbol, tmp, MAX_SYMBOL_LEN);

	hash_key = calculate_hash_key(symbol);

	return get_hash_table_idx(hash_key);

}

static unsigned int 
get_realtime_price_hash_key(char  *quote, char *symbol)
{
	unsigned int hash_key = 0;
	char *tmp = ((struct  dce_my_rt_pri_quote *)(quote + TIMESTAMP_LEN))->ContractID;
	
	strncpy(symbol, tmp, MAX_SYMBOL_LEN);

	hash_key = calculate_hash_key(symbol);

	return get_hash_table_idx(hash_key);

}

static unsigned int 
get_ten_entrust_hash_key(char  *quote, char *symbol)
{
	unsigned int hash_key = 0;
	char *tmp = ((struct dce_my_ten_entr_quote *)(quote + TIMESTAMP_LEN))->Contract;
	
	strncpy(symbol, tmp, MAX_SYMBOL_LEN);

	hash_key = calculate_hash_key(symbol);

	return get_hash_table_idx(hash_key);

}

static unsigned int
get_arbi_hash_key(char *quote, char *symbol)
{
	unsigned int hash_key = 0;
	char *tmp = ((struct dce_my_best_deep_arbi_quote *)(quote + TIMESTAMP_LEN))->Contract;
	
	strncpy(symbol, tmp, MAX_SYMBOL_LEN);

	hash_key = calculate_arbi_hash_key(symbol);

	return get_hash_table_idx(hash_key);

}

typedef unsigned int(*process_quote_symbol_t) (char  *quote, char *symbol);

static process_quote_symbol_t calculate_symbol_func_ar[] = 
{
	NULL,
	NULL,
	get_best_and_deep_hash_key,
	get_arbi_hash_key,
	get_ten_entrust_hash_key,
	get_order_statistic_hash_key,
	get_realtime_price_hash_key,
	get_march_price_hash_key,
};


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
				
		return -1;
	}

	/* orginal node store orginal binary quote file infomation*/
	orginal_file = &node->orginal_node;


	set_binary_common_head(&binary_head_info);

	/* scanning original binary file */
	for (idx = 0; idx < binary_head_info.sym_cnt; idx++) {

		current_addr = get_current_quote_addr(orginal_file);
		
		hash_idx = calculate_symbol_func_ar[binary_head_info.type](current_addr, symbol);
		if(symbol == NULL) {
			printf("[ERROR]: FILE:%s ,LINE:%d, FUNC:%s \n",
				__FILE__,
				__LINE__,
				__FUNCTION__);
			return -1;
		}
#if 0	
		static unsigned long debug_idx= 0;
		if (strncmp(symbol,"c1701", 5) == 0){
			debug_idx++;
			if (debug_idx == 52440) {
				printf("stop here ! \n");
			}	
		}
#endif 		
		if (node->node_ar[hash_idx].mmap_addr == NULL) {
			split_create(&node->node_ar[hash_idx], symbol, current_addr);
		} else {
			split_append(&node->node_ar[hash_idx], symbol, current_addr);
		}

		update_binary_offset(orginal_file);
	}

	return 0;
}

int 
orginal_binary_destory(quote_struct_t *quote)
{
	int ret = 0;
	if (quote->mmap_addr) {
		msync(quote->mmap_addr, quote->file_len, MS_ASYNC);
		
		ret = munmap(quote->mmap_addr, quote->file_len);
		if (-1 == ret) {
			printf("[ERROR]: FILE:%s ,LINE:%d, FUNC:%s got error\n",
					__FILE__,
					__LINE__,
					__FUNCTION__);
			close(quote->fd);
			return -1;
		}

		quote->mmap_addr = NULL;
		close(quote->fd);
	}
	return 0;
}

int 
flush_data_to_disk(quote_struct_t *quote)
{
	char *quote_type_name[] = { NULL, NULL,"_BestAndDeepQuote", "_Arbi","_TenEntrust", "_OrderStatistic","_RealTimePrice","_MarchPriceQty"};
	char quote_name_path[FLUSH_NAME_LEN];
	unsigned short type = get_quote_type();
	FILE *fd;
	quote_struct_t *tmp_p = quote;
#if 0
	if (type == 5) {
		type = 6;
	}else if (type == 6) {
		type = 5;
	}
#endif
	/*Traversal hash index all node */
	for ( ;tmp_p->next != NULL ; tmp_p = tmp_p->next ) {
	
		snprintf(quote_name_path, FLUSH_NAME_LEN,"%s%s.dat",tmp_p->symbol,quote_type_name[type]);

		fd = fopen(quote_name_path,"w+");
		if (fd == NULL) {
			printf("[ERROR]: FILE:%s ,LINE:%d, FUNC:%s got error\n",
					__FILE__,
					__LINE__,
					__FUNCTION__);
			return -1;
		}

		fwrite(tmp_p->mmap_addr, tmp_p->cur_write_offset, 1, fd);
		fflush(fd);
		fclose(fd);
		fd = NULL;	
	}

	return 0;
}

int 
split_destory(split_node_t *node)
{
	int ret = 0, idx = 0;
	/*the first destory original binary file*/
	quote_struct_t  	*orginal_file, *node_ar;
	orginal_file = &node->orginal_node;
	node_ar = (quote_struct_t *)&node->node_ar;

	ret = orginal_binary_destory(orginal_file);
	if (ret != 0) {
		printf("[ERROR]: FILE:%s ,LINE:%d, FUNC:%s got error\n",
					__FILE__,
					__LINE__,
					__FUNCTION__);
	}

	for (idx = 0; idx < MAX_SYM_IDX; idx++) {
		if(node_ar[idx].mmap_addr != NULL) {
			ret = flush_data_to_disk(&node_ar[idx]);
			if (ret != 0) {
				printf("[ERROR]: FILE:%s ,LINE:%d, FUNC:%s got error\n",
					__FILE__,
					__LINE__,
					__FUNCTION__);
			}
		}
	}
}
