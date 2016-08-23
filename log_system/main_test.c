#include <pthread.h>
#include <sched.h>
#include "my_log.h"

#define ATOMIC (1)


void *
hanle_log_thread(void *argv)
{
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	log_info_t *handle = (log_info_t *)(argv);
	atomic_log_info(handle, "multiple thread call info! \n");
	atomic_log_error(handle, "multiple thread call error! \n");
	
	return NULL;
}


int 
main()
{
	int ret = 0;
	pthread_t pth_id;
	log_info_t test;
	set_log_file_len(&test, 4096);
	ret = log_init( "./test.log", &test);
	if (ret != 0)
	{
		printf("log_init got error!\n");
	}

#ifdef ATOMIC
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
	pthread_create(&pth_id, &attr, hanle_log_thread, &test);
	pthread_attr_destroy(&attr);
	atomic_log_info(&test, "Master thread call info! \n");
	atomic_log_error(&test, "Master thread call error! \n");

#else 	
	log_info(&test, "my name is waten\n");
	log_error(&test, "I want buy telsa\n");
#endif
	sleep(3);
	log_destory(&test);
	return 0;
}