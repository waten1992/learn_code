#include "my_log.h"


int 
main()
{
	int ret = 0;
	log_info_t test;
	set_log_file_len(&test, 4096);
	ret = log_init( "./test.log", &test);
	if (ret != 0)
	{
		printf("log_init got error!\n");
	}

	log_info(&test, "my name is waten\n");
	log_error(&test, "I want buy telsa\n");
	log_destory(&test);
	return 0;
}