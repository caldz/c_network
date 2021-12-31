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
 * 20211214  	         CaiDongZe           	Create
 * ===========================================================================================
 */


#include "pax_log.h"
#include "dict.h"
#include "vector.h"
#include "sin.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include "smart_select.h"



int read_select(int *fd_list,int fd_count,unsigned int timeout_s,unsigned int timeout_us,int *change_fd_list);





typedef struct {
	void *sin;
	int fd;
	void *client_dict;
	void *ret_data_request_client_addr_list;
	int listen_limit;
	void *r_select;
	void *w_select;
} tcpsock_t;
void *tcpsock_create()
{
	tcpsock_t *tcpsock=c_malloc(sizeof(tcpsock_t));
	tcpsock->sin=sin_create();
	tcpsock->client_dict=dict_create();
	tcpsock->ret_data_request_client_addr_list=vect_create(vect_type_link());	
	tcpsock->r_select=smart_select_create();
	tcpsock->w_select=smart_select_create();smart_select_set_mode(tcpsock->w_select,smart_select_mode_write());
	tcpsock->listen_limit=5;
	return tcpsock;
}
static int fd_close(void *fd) {close((int)fd);return 0;}
int tcpsock_free(tcpsock_t *tcpsock)
{
	if (tcpsock->fd>0) close(tcpsock->fd);
	dict_clear(tcpsock->client_dict,fd_close);
	dict_free(tcpsock->client_dict);
	sin_free(tcpsock->sin);
	vect_clear(tcpsock->ret_data_request_client_addr_list,c_free);
	vect_free(tcpsock->ret_data_request_client_addr_list);
	smart_select_free(tcpsock->r_select);
	smart_select_free(tcpsock->w_select);
	c_free(tcpsock);
	return 0;
}
//local_addr: "[\"<ip>\",<port>]"
int tcpsock_set_local_addr(tcpsock_t *tcpsock,char *local_addr)
{
	FT_ASSERT( sin_set_ip_and_port_by_addr(tcpsock->sin,local_addr); ,<0, return -1 );
	return 0;
}
static int tcpsock_base_init(tcpsock_t *tcpsock)
{
	int protocal=SOCK_STREAM;
	tcpsock->fd=FT_ASSERT( socket(sin_get_family(tcpsock->sin),protocal,0) ,<=0, return -1 );
	return 0;
}
#define TCPSOCK_EXTEND_AREA
#ifdef TCPSOCK_EXTEND_AREA
int tcpsock_get_fd(tcpsock_t *tcpsock)
{
	return tcpsock->fd;
}
#endif//#ifdef TCPSOCK_EXTEND_AREA




#define SERVER_TCPSOCK_AREA
int server_tcpsock_init(tcpsock_t *tcpsock)
{
	FT_ASSERT( tcpsock_base_init(tcpsock); ,<0, return -1 );
	int fd=tcpsock->fd;
	int on=1;
	FT_ASSERT( setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)) ,<0, close(fd);return -1 );
	FT_ASSERT( bind(fd,tcpsock->sin,sin_get_size(tcpsock->sin)) ,<0, close(fd);return -1 );
	FT_ASSERT( listen(fd,tcpsock->listen_limit) ,<0, close(fd);return -1; );
	return 0;
}
int server_tcpsock_set_listen_limit(tcpsock_t *tcpsock,int listen_lim)
{
	tcpsock->listen_limit=listen_lim;
	return 0;
}
int server_tcpsock_is_has_connect_request(tcpsock_t *server_tcpsock)
{
	int change_fd_count=FT_ASSERT( smart_select_work(server_tcpsock->r_select,&server_tcpsock->fd,1,0); ,<0, );
	return change_fd_count>0 ? 1 : 0;
}
//return:{client_addr:"[\"<ip>\", <port>]", 0:accept失败}"
char *server_tcpsock_accept(tcpsock_t *tcpsock)
{
	int fd=tcpsock->fd;
	void *dict=tcpsock->client_dict;
	void *sin=sin_create();socklen_t len=sin_get_size(sin);
	int read_sock=FT_ASSERT( accept(fd,sin,&len); ,<0, sin_free(sin);return 0; );
	char client_addr[64]={0};sin_get_addr(sin,client_addr);
	sin_free(sin);
	FT_ASSERT( dict_append_value(dict,client_addr,(void *)read_sock); ,<0, return 0);
	return dict_get_key_by_value(dict,(void *)read_sock);
}
int server_tcpsock_send(tcpsock_t *server_tcpsock,char *client_addr,char *data,int data_len)
{
	int fd=(int)FT_ASSERT( (int)dict_get_value(server_tcpsock->client_dict,client_addr) ,<=0, return -1);
	return FT_ASSERT( send(fd,data,data_len,0); ,<=0, return -1);
}
int server_tcpsock_check_data_request_client_list(tcpsock_t *tcpsock,char **request_client_list)
{
	void *dict=tcpsock->client_dict;
	if (dict_get_count(dict)==0) return 0;
	int test_fd_list[128]={0};int test_fd_count=0;
	dict_foreach(dict,
		test_fd_list[test_fd_count++]=(int)value;
	);
	int changed_fd_list[128]={0};
	int change_fd_count=FT_ASSERT( smart_select_work(tcpsock->r_select,test_fd_list,test_fd_count,changed_fd_list); ,<0, return -1 );
	if (change_fd_count<=0) return change_fd_count;
	
	//构造检查到的端口客户端列表并返回
	int match_addr_change_fd=0;
	vect_clear(tcpsock->ret_data_request_client_addr_list,c_free);
	int i;for (i=0;i!=change_fd_count;++i) {
		char *request_client_addr=(void *)FT_ASSERT( (int)dict_get_key_by_value(dict,(void *)changed_fd_list[i]) ,==0, return -1 );
		char *new_request_client_addr=c_malloc_str(request_client_addr);
		vect_append_data(tcpsock->ret_data_request_client_addr_list,new_request_client_addr);
		request_client_list[match_addr_change_fd++]=new_request_client_addr;
	}

	return match_addr_change_fd;
}
int server_tcpsock_recv(tcpsock_t *tcp_sock,char *client_addr,char *data_buf,int buf_size)
{
	void *dict=tcp_sock->client_dict;
	int fd=(int)dict_get_value(dict,client_addr);
	int ret=recv(fd,data_buf,buf_size,0);
	if (ret<=0) {
		close(fd);
		dict_delete_value(dict,client_addr);
	}
	return ret;
}
#undef SERVER_TCPSOCK_AREA





#define CLIENT_TCPSOCK_AREA
static int tcpsock_check_is_nonblock_mode(tcpsock_t *tcpsock)
{
	int flag=FT_ASSERT( fcntl(tcpsock->fd,F_GETFL,0); ,<0, return -1);
	return flag&O_NONBLOCK ? 1 : 0;
}
int client_tcpsock_init(tcpsock_t *client_tcpsock)
{
	FT_ASSERT( tcpsock_base_init(client_tcpsock); ,<0, return -1 );
	return 0;
}
int client_tcpsock_connect(tcpsock_t *client_tcpsock,char *target_addr)
{
	void *sin=sin_create();
	sin_set_ip_and_port_by_addr(sin,target_addr);
	char client_addr[64]={0};sin_get_addr(sin,client_addr);

	//check whether it is nonblock mode
	int is_nonblock=FT_ASSERT( tcpsock_check_is_nonblock_mode(client_tcpsock); ,<0, return -1 );
//	ERR("fd=%d,addr=%s,block_mode=%s",client_tcpsock->fd,client_addr,is_nonblock?"nonblock":"block");
	int ret=is_nonblock ? connect(client_tcpsock->fd,sin,sin_get_size(sin)) :
		FT_ASSERT( connect(client_tcpsock->fd,sin,sin_get_size(sin)); ,<0, );
//	if (ret<0) ERR("[%d][%d]%s",ret,errno,strerror(errno));
	if (is_nonblock && ret<0 && errno==EINPROGRESS) ret=0;
	sin_free(sin);
	return ret;
}
int client_tcpsock_send(tcpsock_t *client_tcpsock,char *data,int data_len)
{
	return FT_ASSERT( send(client_tcpsock->fd,data,data_len,0); ,<0, );
}
int client_tcpsock_is_has_data(tcpsock_t *client_tcpsock)
{
	return FT_ASSERT( smart_select_work(client_tcpsock->r_select,&client_tcpsock->fd,1,0); ,<0, );
}
int client_tcpsock_recv(tcpsock_t *client_tcpsock,char *data_buf,int buf_size)
{
	return FT_ASSERT( recv(client_tcpsock->fd,data_buf,buf_size,0); ,<0, );
}

//用于非阻塞connect的情况
int client_tcpsock_turn_nonblock_connect(tcpsock_t *client_tcpsock)
{
	int flag=FT_ASSERT( fcntl(client_tcpsock->fd,F_GETFL,0); ,<0, return -1 );
	FT_ASSERT( fcntl(client_tcpsock->fd,F_SETFL,flag|O_NONBLOCK) ,<0, return -1 );
	return 0;
}
int client_tcpsock_is_connected(tcpsock_t *client_tcpsock)
{
	int fd=client_tcpsock->fd;
	FT_ASSERT( tcpsock_check_is_nonblock_mode(client_tcpsock); ,<=0, return 1 );
	int ret=FT_ASSERT( smart_select_work(client_tcpsock->w_select,&fd,1,0); ,<0, return -1 );
	if (ret>0) {
		int error=0;socklen_t len=sizeof(error);
		FT_ASSERT( getsockopt(fd,SOL_SOCKET,SO_ERROR,&error,&len); ,<0, return -1 );
		if (error==0) return 1;
		else if (error>0) {
			ERR("error while connecting: %s",strerror(error));
			return  -1;
		}
	}
	return ret>0 ? 1 : 0;
}
#undef CLIENT_TCPSOCK_AREA






