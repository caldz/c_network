/*
 * ===========================================================================================
 * = COPYRIGHT
 *          PAX Computer Technology(Shenzhen) CO., LTD PROPRIETARY INFORMATION
 *   This software is supplied under the terms of a license agreement or nondisclosure
 *   agreement with PAX Computer Technology(Shenzhen) CO., LTD and may not be copied or
 *   disclosed except in accordance with the terms in that agreement. 
 *     Copyright (C) YYYY-? PAX Computer Technology(Shenzhen) CO., LTD All rights reserved.
 * Description: // Detail description about the function of this module,
 *             // interfaces with the other modules, and dependencies.
 * Revision History:
 * Date	                 Author	                Action
 * 20211223 	         CaiDongZe           	Create
 * ===========================================================================================
 */


#include <errno.h>
#include <unistd.h>
#include "pax_log.h"
#include "vector.h"

#define SMART_SELECT_SOURCE
#include "smart_select.h"

typedef struct {
	char *mode;
	unsigned int timeout_s;
	unsigned int timeout_us;


	void *fd_list;
	int cf_type;
} smart_select_t;

char *smart_select_mode_read() {return "read";}
char *smart_select_mode_write() {return "write";}

void *smart_select_create()
{
	smart_select_t *sel=c_malloc(sizeof(smart_select_t));
	sel->mode=smart_select_mode_read();
	sel->timeout_s=0;
	sel->timeout_us=0;



	sel->cf_type=cf_read;
	sel->fd_list=vect_create(vect_type_link());
	return sel;
}
int smart_select_free(smart_select_t *sel)
{
	vect_free(sel->fd_list);
	c_free(sel);
	return 0;
}
int smart_select_set_timeout(smart_select_t *smart_select,unsigned int timeout_s,unsigned int timeout_us)
{
	smart_select->timeout_s=timeout_s;
	smart_select->timeout_us=timeout_us;
	return 0;
}
int smart_select_set_mode(smart_select_t *smart_select, char *mode)
{
	smart_select->mode=mode;
	return 0;
}
int smart_select_super_set_cf_type(smart_select_t *sel,int cf_type)
{
	sel->cf_type=cf_type;
	return 0;
}

int smart_select_work(smart_select_t *smart_select,int *fd_list,int fd_count,int *change_fd_list)
{
	int max_fd=0;
	fd_set fds;FD_ZERO(&fds);
	{
		int i;for (i=0;i!=fd_count;++i) {
			int fd=fd_list[i];
			if (fd>max_fd) max_fd=fd;
			FD_SET(fd,&fds);
		}
	}
	
	struct timeval timeout={.tv_sec=smart_select->timeout_s,.tv_usec=smart_select->timeout_us};
	void *pread=0, *pwrite=0, *perror=0;
	if (smart_select->mode==smart_select_mode_write()) pwrite=&fds;
	else pread=&fds;
	
	int ret=FT_ASSERT( select(max_fd+1,pread,pwrite,perror,&timeout) ,<0, return -1 );
	if (!ret) return 0;
	
	int change_fd_count=0;
	int i;for (i=0;i!=max_fd+1;++i) {
		if (FD_ISSET(i,&fds)) {
			if (change_fd_list) change_fd_list[change_fd_count]=i;
			change_fd_count++;
//			if (change_fd_count==ret) break;
		}
	}

	if (change_fd_count!=ret) {//原则上这两个会相等，先测试一段时间看是否存在不相等的场景
		ERR("change_fd_count(%d) not equal with ret(%d) of select",change_fd_count,ret);
		return change_fd_count;
	}
	
	//select返回值大于0时，含义为实际变化的fd数量
	return ret;
}





typedef struct {
	int fd;
	int cf_type;
} change_fd_item_t;
typedef struct {
	int result;
	change_fd_item_t change_fd_list[128];
	int change_fd_count;
} ret_smart_select_raw_t;
static ret_smart_select_raw_t sel_raw(int cf_type,int *fd_list,int fd_count,unsigned int timeout_s,unsigned int timeout_us)
{
	int max_fd=0;
	fd_set rfds,wfds,efds;FD_ZERO(&rfds);FD_ZERO(&wfds);FD_ZERO(&efds);
	{
		int i;for (i=0;i!=fd_count;++i) {
			int fd=fd_list[i];
			if (fd>max_fd) max_fd=fd;
			if (cf_type&cf_read) FD_SET(fd,&rfds);
			if (cf_type&cf_write) FD_SET(fd,&wfds);
			if (cf_type&cf_error) FD_SET(fd,&efds);
		}
	}
	
	struct timeval timeout={.tv_sec=timeout_s,.tv_usec=timeout_us};
	void *pread=(cf_type&cf_read) ? &rfds : 0;
	void *pwrite=(cf_type&cf_write) ? &wfds : 0;
	void *perror=(cf_type&cf_error) ? &efds : 0;
	ret_smart_select_raw_t sret={0};
	int ret=FT_ASSERT( select(max_fd+1,pread,pwrite,perror,&timeout) ,<0, sret.result=-1;return sret );
	if (!ret) return sret;
	
	int change_fd_count=0;
	int i;for (i=0;i!=max_fd+1;++i) {
		change_fd_item_t *p=&sret.change_fd_list[change_fd_count];
		if ( (pread&&FD_ISSET(i,(fd_set *)pread)) || (pwrite&&FD_ISSET(i,(fd_set *)pwrite)) || (perror&&FD_ISSET(i,(fd_set *)perror)) ) {
			if (pread && FD_ISSET(i,(fd_set *)pread)) p->cf_type|=cf_read;
			if (pwrite && FD_ISSET(i,(fd_set *)pwrite)) p->cf_type|=cf_write;
			if (perror && FD_ISSET(i,(fd_set *)perror)) p->cf_type|=cf_error;
			p->fd=i;
			++change_fd_count;
//			ERR("<fd:%d,cf_type:%p>",p->fd,p->cf_type);
		}
	}
	sret.change_fd_count=change_fd_count;
	//select返回值大于0时，含义为实际变化的fd数量
	return sret;
}



int smart_select_super_add_fd(smart_select_t *sel,int fd)
{
	vect_append_data(sel->fd_list,(void *)fd);
	return 0;
}
int smart_select_super_del_fd(smart_select_t *sel,int fd)
{
	vect_delete_data(sel->fd_list,(void *)fd);
	return 0;
}
int smart_select_super(smart_select_t *sel,int *change_fd_list,int *pchange_fd_count)
{
	int fd_list[128]={0};int fd_count=vect_get_count(sel->fd_list);
	int i;for (i=0;i!=fd_count;++i) fd_list[i]=(int)vect_get_data(sel->fd_list,i);
	
	ret_smart_select_raw_t rret=sel_raw(sel->cf_type,fd_list,fd_count,sel->timeout_s,sel->timeout_us);
	
	int change_fd_count=0;int *p=change_fd_list;{
		int i;for (i=0;i!=rret.change_fd_count;++i) {
			change_fd_item_t *item=&rret.change_fd_list[i];
			if (item->cf_type & sel->cf_type) 
				p[change_fd_count++]=item->fd;
		}
	}
	
	*pchange_fd_count=change_fd_count;
	return rret.result;
}
//int smart_select_super_print(smart_select_t *sel)
//{
//	vect_print_data(sel->fd_list);
//	return 0;
//}
