#include "split_dce.h"

int
main(int argc, char const *argv[])
{
	int ret = 0;
	char *path;
	split_node_t node;
	path = (char *)argv[1];

	if (argc != 2) {
		printf("Please input like: ./execute_able original_file \n");
		return -1;
	}

	ret = split_init(path, &node);
	if(ret != 0) {
		printf("[ERROR] FILE:%s ,LINE:%d, FUNC:%s \n",
			__FILE__,
			__LINE__,
			__FUNCTION__);
		
		return -1;
	}

	ret = split_destory(&node);
	if(ret != 0) {
		printf("[ERROR] FILE:%s ,LINE:%d, FUNC:%s \n",
			__FILE__,
			__LINE__,
			__FUNCTION__);
		
		return -1;
	}

	return 0;
}
