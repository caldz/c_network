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


#ifndef TCPSOCK_H
#define TCPSOCK_H


//addr format: "[\"<ip>\", <port>]"

#define BASE_INTF
void *tcpsock_create();
int tcpsock_free(void *tcpsock);
int tcpsock_set_local_addr(void *tpcsock,char *local_addr);
#undef BASE_INTF




#define TCP_SERVER_INTF
int server_tcpsock_is_has_connect_request(void *server_tcpsock);
//return accepted client addr
char *server_tcpsock_accept(void *server_tcpsock);
int server_tcpsock_send(void *server_tcpsock,char *client_addr,char *data,int data_len);
int server_tcpsock_check_data_request_client_list(void *server_tcpsock,char **request_client_list);
int server_tcpsock_recv(void *server_tcpsock,char *client_addr,char *data_buf,int buf_size);

int server_tcpsock_init(void *tcpsock);
int server_tcpsock_set_listen_limit(void *server_tcpsock,int listen_lim);
#undef TCP_SERVER_INTF




#define TCP_CLIENT_INTF
int client_tcpsock_init(void *client_tcpsock);
int client_tcpsock_connect(void *client_tcpsock,char *target_addr);
int client_tcpsock_send(void *client_tcpsock,char *data,int data_len);
int client_tcpsock_is_has_data(void *client_tcpsock);
int client_tcpsock_recv(void *client_tcpsock,char *data_buf,int buf_size);

int client_tcpsock_turn_nonblock_connect(void *client_tcpsock);
int client_tcpsock_is_connected(void *client_tcpsock);
#undef TCP_CLIENT_INTF


#define EXTEND_INTF
int tcpsock_get_fd(void *tcpsock);
#undef EXTEND_INTF















#endif//#ifndef TCPSOCK_H

