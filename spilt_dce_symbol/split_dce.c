#include "split_dce.h"

void
set_log_file_len(log_info_t *handle, unsigned int buf_size)
{
	handle->file_len = buf_size;
}

int 
split_create(quote_struct_t *quote_file)
{

}

int 
parser_quote_header(split_node_t *node , char *quote_path)
{
	int idx = node->node_idx;
	node->node_ar[idx].fd = open(path, O_RDWR);
	if (-1 == node->node_ar[idx].fd) 
	{  
		printf("[ERROR] :open(%s) got error, reason: %s! FILE:%s ,LINE:%d, FUNC:%s\n", 
				path,
				strerror(errno),
				__FILE__,
				__LINE__,
				__FUNCTION__);

		return -1;
	}

	node->node_ar[idx].mmap_addr= mmap(NULL, handle->file_len, PROT_READ | PROT_WRITE, MAP_SHARED, handle->fd, 0);
	if (MAP_FAILED == node->node_ar[idx].mmap_addr)
	{
		close(node->node_ar[idx].fd);
		printf("[ERROR]: mmap() got error, reason:%s ! FILE:%s ,LINE:%d, FUNC:%s\n",
				strerror(errno),
				__FILE__,
				__LINE__,
				__FUNCTION__);
		return -1;
	}

	node->node_ar[idx].quote_head = *(binary_header_t *)(node->node_ar[idx].mmap_addr);

	return 0;
}

int 
split_init(char *quote_path, split_node_t *node)
{
	int ret = 0, idx = 0;
	binary_header_t binary_head_info;
	memset(node, 0, sizeof(split_node_t));
	ret = parser_quote_header(node , quote_path);
	if (ret != 0) 
	{

	}

	binary_head_info = node->node_ar[node->node_idx].quote_head;

	for (idx = 0; i < binary_head_info->sym_cnt; idx++)
	{
		
	}


}