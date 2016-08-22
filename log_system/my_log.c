#include "my_log.h"

#define MEMCPY  (1)

void
set_log_file_len(log_info_t *handle, unsigned int buf_size)
{
	handle->file_len = buf_size;
}

int 
log_init(char *path, log_info_t *handle)
{
	handle->fd = open(path, O_RDWR | O_CREAT);
	if (-1 == handle->fd) {  
		printf("[ERROR] :open(%s) got error, reason: %s! FILE:%s ,LINE:%d, FUNC:%s\n", 
				path,
				strerror(errno),
				__FILE__,
				__LINE__,
				__FUNCTION__);

		return -1;
	}

	if (-1 == ftruncate(handle->fd, handle->file_len)) {
		close(handle->fd);
		printf("[ERROR]:ftruncate() got error , reason:%s ! FILE:%s ,LINE:%d, FUNC:%s\n",
				strerror(errno),
				__FILE__,
				__LINE__,
				__FUNCTION__);
		return -1;
	}

	handle->mmap_addr = mmap(NULL, handle->file_len, PROT_READ | PROT_WRITE, MAP_SHARED, handle->fd, 0);
	if (MAP_FAILED == handle->mmap_addr) {
		close(handle->file_len);
		printf("[ERROR]: mmap() got error, reason:%s ! FILE:%s ,LINE:%d, FUNC:%s\n",
				strerror(errno),
				__FILE__,
				__LINE__,
				__FUNCTION__);
		return -1;
	}

	handle->offset = 0;
	return 0;

}


int 
log_destory(log_info_t *handle)
{
	int ret = 0;
	msync(handle->mmap_addr, handle->file_len, MS_ASYNC);
	if (handle->mmap_addr != NULL) {
		ret = munmap(handle->mmap_addr, handle->file_len);
		if (-1 == ret) {
			printf("[ERROR]: FILE:%s ,LINE:%d, FUNC:%s got error\n",
					__FILE__,
					__LINE__,
					__FUNCTION__);
			close(handle->fd);
			return -1;
		}

		handle->mmap_addr = NULL;
		close(handle->fd);
	}
}

/*if you want more fast speed write memory, then you need special format write memory
* like struct {
	char 	data[128];	
}
*
*/

void
log_info(log_info_t *handle, char *content)
{
	/*snprintf() will cost more cyclecs, please use strlen() and memecpy instead */
	int ret = 0;
#ifdef MEMCPY
	ret =  strlen(content);
	memcpy((handle->mmap_addr + handle->offset), "[INFO]: ", sizeof("[INFO]: "));
	memcpy((handle->mmap_addr + handle->offset), content, ret);
	handle->offset = (ret +sizeof("[INFO]: "));
#else 	
	ret = snprintf((handle->mmap_addr + handle->offset), (handle->file_len - handle->offset),"[INFO]: %s\n",content);
	handle->offset +=  ret;
#endif 


}

void
log_error(log_info_t *handle, char *content)
{
	int ret = 0;
#ifdef MEMCPY
	ret =  strlen(content);
	memcpy((handle->mmap_addr + handle->offset), "[ERROR]: ", sizeof("[ERROR]: "));
	memcpy((handle->mmap_addr + handle->offset), content, ret);
	handle->offset =  (ret + sizeof("[ERROR]: "));
#else 
	ret = snprintf((handle->mmap_addr + handle->offset), (handle->file_len - handle->offset),"[ERROR]: %s\n",content);
	handle->offset +=  ret;
#endif	
	

}

