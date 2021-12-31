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




#include "tcpsock.h"
#include <pthread.h>
#include <errno.h>
#include "smart_select.h"
#include "pax_log.h"
#include "base64.h"
#include <unistd.h>
#include "jexp.h"
#include "dict.h"
#include "sin.h"

#define LOCAL_COMPONENT_SOURCE
#include "local_component.h"

typedef struct {
	void *socket_client_manager;
	char *target_addr;
	char *transfer_addr;
	void *tcpsock;

	pthread_t handler;
	int handler_exit_flag;
	int stat;
	char *finish_message;
	void *cln_dict;
} loco_t;
void *loco_create()
{
	loco_t *loco=c_malloc(sizeof(loco_t));
	loco->tcpsock=tcpsock_create();
	loco->cln_dict=dict_create();
	loco->stat=ls_not_inited;
	return loco;
}
int loco_free(loco_t *loco)
{
	loco->handler_exit_flag=1;
	while (loco->stat==ls_handler_work) {OsSleep(1*100);}
	dict_clear(loco->cln_dict,tcpsock_free);
	dict_free(loco->cln_dict);
	tcpsock_free(loco->tcpsock);
	c_free(loco->target_addr);
	c_free(loco->transfer_addr);
	c_free(loco);
	return 0;
}



#define num_in_list(num,list_init_exp) ({\
	int __ret=0;\
	int __list[]=list_init_exp;\
	int __list_size=sizeof(__list)/sizeof(int);\
	int __i;for (__i=0;__i!=__list_size;++__i) {\
		if (num==__list[__i]) {\
			__ret=1;\
			break;\
		}\
	}\
	__ret;\
})

enum {
	sa_none,
	sa_add,
	sa_del,
	sa_error,
} sel_act_t;
typedef struct {
	int act;
	int fd;
} proc_recv_transfer_data_ret_t;
static proc_recv_transfer_data_ret_t loco_m_proc_recv_data_from_transfer_server(loco_t *loco,char *data)
{
	proc_recv_transfer_data_ret_t sret={.act=sa_none,.fd=0};
	void *jexp=jexp_create();
	void *base64_mgr=base64_mgr_create();
	
	do {
		char *cmd=(char*)FT_ASSERT( (int)jexp_load_value(jexp,data,0,"cmd"); ,==0, break );
		char address[128]={0};{
			char *ip=(char *)FT_ASSERT( (int)jexp_load_value(jexp,data,0,"client_address/0"); ,==0, break );
			int port=(int)FT_ASSERT( (int)jexp_load_value(jexp,data,(void *)-1,"client_address/1"); ,==-1, break );
			sprintf(address,"[%s, %d]",ip,port);
		}

		if (str_mat(cmd,"sc_send")) {
			void *tcpsock=(void *)FT_ASSERT( (int)dict_get_value(loco->cln_dict,address); ,==0, break );
			char *base64_str=(char*)FT_ASSERT( (int)jexp_load_value(jexp,data,0,"base64_data"); ,==0, break );
			int bdata_len=0;
			char *bdata=(char *)FT_ASSERT( (int)base64_mgr_decode(base64_mgr,base64_str,&bdata_len); ,==0, break );
//			ERR("base64_str=%s,bdata=%s",base64_str,get_buf_str(bdata,bdata_len));
			client_tcpsock_send(tcpsock,bdata,bdata_len);
		}
		else if (str_mat(cmd,"sc_connect")) {
//			ERR("%s connect",address);
			void *tcpsock=tcpsock_create();
			FT_ASSERT( client_tcpsock_init(tcpsock); ,<0, tcpsock_free(tcpsock);break );
			FT_ASSERT( client_tcpsock_turn_nonblock_connect(tcpsock) ,<0, break );
			FT_ASSERT( client_tcpsock_connect(tcpsock,loco->target_addr); ,<0, tcpsock_free(tcpsock);break );
			{
				ERR("start connect");
				ST_TIMER timer_raw={0};ST_TIMER *timer=&timer_raw;unsigned long timeout=10*1000;
				OsTimerSet(timer,timeout);int connect_success_flag=0;
				while (1) {
					if (!OsTimerCheck(timer)) break;
					if (client_tcpsock_is_connected(tcpsock)) {
						connect_success_flag=1;
						break;
					}
				}
				ERR("connect finish:%d",connect_success_flag);
				if (!connect_success_flag) {
					sret.act=sa_error;
					break;
				}
			}
			sret.fd=tcpsock_get_fd(tcpsock);
			sret.act=sa_add;
			dict_append_value(loco->cln_dict,address,tcpsock);
		}
		else if (str_mat(cmd,"sc_disconnect")) {
//			ERR("%s disconnect",address);
			void *tcpsock=(void *)FT_ASSERT( (int)dict_get_value(loco->cln_dict,address); ,==0, break );
			sret.fd=tcpsock_get_fd(tcpsock);
			sret.act=sa_del;
			tcpsock_free(tcpsock);
			dict_delete_value(loco->cln_dict,address);
		}
	} while (0);

	base64_mgr_free(base64_mgr);
	jexp_free(jexp);
	
	return sret;
}
static int loco_m_proc_send_data_to_transfer_server(loco_t *loco,char *client_address,char *recv_data,int data_len)
{
	int ret=-1;
	void *base64_mgr=base64_mgr_create();
	void *jexp=jexp_create();
	
	do {
		char ip[64]={0};int port=0;
		FT_ASSERT( addr_to_ip_and_port(client_address,ip,&port); ,<0, break );
		char json_client_address_str[128]={0};sprintf(json_client_address_str,"[\"%s\", %d]",ip,port);
		char *base64_str=(char *)FT_ASSERT( (int)base64_mgr_encode(base64_mgr,recv_data,data_len); ,==0, break );
		char dict_data[4096]={0};
		sprintf(dict_data,"{\"cmd\":\"sc_recv\", \"base64_data\":\"%s\",\"client_address\":%s}",base64_str,json_client_address_str);
		client_tcpsock_send(loco->tcpsock,dict_data,strlen(dict_data));
		ret=0;
	} while (0);

	jexp_free(jexp);
	base64_mgr_free(base64_mgr);
	return ret;
}
static int loco_m_get_tcpsock_and_address_by_fd(loco_t *loco,int fd,void **ptcpsock,char **pclient_address)
{
	int ret=-1;
	dict_foreach(loco->cln_dict,
		if (tcpsock_get_fd(value)==fd) {
			if (ptcpsock) *ptcpsock=value;
			if (pclient_address) *pclient_address=key;
			ret=0;
			break;
		}
	);
	return ret;
}
static void *loco_handler(void *args)
{
	ERR("in");
	int handler_flag=1;
	loco_t *loco=args;
	int fd_main=tcpsock_get_fd(loco->tcpsock);
	void *sel=smart_select_create();
	smart_select_set_timeout(sel,1,0);
	smart_select_super_add_fd(sel,fd_main);
	loco->stat=ls_handler_work;
	while (!loco->handler_exit_flag && handler_flag) {
		int change_fd_list[128]={0};int change_fd_count=0;
		int ret=smart_select_super(sel,change_fd_list,&change_fd_count);
		if (ret<0) {
			ERR("select fail, error:%s",strerror(errno));
			break;
		}
		else if (change_fd_count>0) {
			int i;for (i=0;i!=change_fd_count;++i) {
				int fd=change_fd_list[i];
				if (fd==fd_main) {
					char recv_data[4096]={0};int buf_size=sizeof(recv_data);
					int ret=client_tcpsock_recv(loco->tcpsock,recv_data,buf_size);
					if (ret<=0) {
						loco->finish_message="disconnected by transfer server";
						handler_flag=0;
						break;
					}
//					ERR("recv from transfer server:%s",recv_data);
					proc_recv_transfer_data_ret_t sret=loco_m_proc_recv_data_from_transfer_server(loco,recv_data);
					if (sret.act==sa_add) smart_select_super_add_fd(sel,sret.fd);
					else if (sret.act==sa_del) smart_select_super_del_fd(sel,sret.fd);
					else if (sret.act==sa_error) {
						loco->finish_message="fail to connect target server";
						handler_flag=0;
						break;
					}
				} else {
					void *tcpsock=0;char *client_address=0;
					FT_ASSERT( loco_m_get_tcpsock_and_address_by_fd(loco,fd,&tcpsock,&client_address); ,<0, continue );

					char recv_data[4096]={0};int buf_size=sizeof(recv_data);
					int ret=client_tcpsock_recv(tcpsock,recv_data,buf_size);
					if (ret<=0) {
						loco->finish_message="disconnected by target server";
						handler_flag=0;
						break;
					}
					else {
						int recv_data_len=ret;
//						ERR("recv %s from target to %s",get_buf_str(recv_data,recv_data_len),client_address);
						loco_m_proc_send_data_to_transfer_server(loco,client_address,recv_data,recv_data_len);
					}
				}
			}
		}
		OsSleep(1*100);
	}

	smart_select_free(sel);
	loco->stat=ls_handler_exit;
	ERR("exit");
	return 0;
}
static int loco_init_with_transfer_server(loco_t *loco)
{
	FT_ASSERT( client_tcpsock_connect(loco->tcpsock,loco->transfer_addr); ,<0, return -1 );
	char *data="{\"cmd\":\"reg\"}";int data_len=strlen(data);
	client_tcpsock_send(loco->tcpsock,data,data_len);
	return 0;
}
int loco_init(loco_t *loco)
{
	FT_ASSERT( client_tcpsock_init(loco->tcpsock); ,<0, return -1 );
	FT_ASSERT( loco_init_with_transfer_server(loco); ,<0, return -1 );
	FT_ASSERT( pthread_create(&loco->handler,0,loco_handler,loco); ,<0, return -1 );
	return 0;
}
int loco_set_target_addr(loco_t *loco,char *target_addr)
{
	if (loco->target_addr) c_free(loco->target_addr);
	loco->target_addr=c_malloc_str(target_addr);
	return 0;
}
int loco_set_transfer_addr(loco_t *loco,char *transfer_addr)
{
	if (loco->transfer_addr) c_free(loco->transfer_addr);
	loco->transfer_addr=c_malloc_str(transfer_addr);
	return 0;
}










int loco_get_state(loco_t *loco)
{
	return loco->stat;
}
char *loco_get_finish_message(loco_t *loco)
{
	return loco->finish_message;
}


