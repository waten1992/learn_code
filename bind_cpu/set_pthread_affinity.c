#define _GNU_SOURCE
#include <stdio.h>
#include <sched.h>
#include <pthread.h>

int
pthread_bind_cpu(unsigned int cpu_no)
{
	int ret = 0;
	cpu_set_t cpuset;
	pthread_t ptd_id;
	ptd_id = pthread_self();

	CPU_ZERO(&cpuset);

	if(CPU_ISSET(cpu_no, &cpuset)) {
		printf("[ERROR-SET-AFFINITY] cpu_no:%d was set affinity !\n", cpu_no);
		return -1;	
	} 

	CPU_SET(cpu_no, &cpuset);
	ret = pthread_setaffinity_np(ptd_id, sizeof(cpu_set_t), &cpuset);
	if (ret != 0) {
		printf("[ERROR-SET-AFFINITY] FILE:%s ,LINE:%d, FUNC:%s \n",
			__FILE__,
			__LINE__,
			__FUNCTION__);

		return -1;
	}

	return 0;
}

/* src like: "place,a1609,0,0,4,3756", return split count number*/
int
parser_splite_by_delim(char *src, char *ar[], char delim)
{
    int idx = 0;
    if(*src != delim) {
        ar[idx++] =  src;
    }

    while(*src) {
        if (*src == delim) {
            *src = '\0';
            ar[idx++] = src++;

        }

        src++;
    }
    return idx;
}


/*use ps -eo pid,args,psr | grep test .  look up bind cpu */

int
main()
{
	int ret = 0;
	ret = pthread_bind_cpu(1);
	if(ret != 0) {
		printf("[ERROR-SET-AFFINITY]: FILE:%s ,LINE:%d, FUNC:%s \n",
				__FILE__,
				__LINE__,
				__FUNCTION__);
		return -1;
	}


	return 0;
}