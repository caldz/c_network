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
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct sockaddr_in sin_t;
#define SIN_EXTEND 
SIN_EXTEND void *sin_create()
{
	sin_t *sin=c_malloc(sizeof(sin_t));
	sin->sin_family=PF_INET;
	sin->sin_addr.s_addr=INADDR_ANY;
	//  sin->sin_addr.s_addr=inet_addr("192.168.43.75");//chad-test
	return sin;
}

SIN_EXTEND int sin_free(sin_t *sin)
{
	c_free(sin);
	return 0;
}

SIN_EXTEND int sin_get_size(sin_t *sin) {return sizeof(sin_t);}
SIN_EXTEND int sin_set_port(sin_t *sin,int port) {sin->sin_port=htons(port);return 0;}
SIN_EXTEND int sin_get_port(sin_t *sin) {return ntohs(sin->sin_port);}
SIN_EXTEND int sin_set_ip(sin_t *sin,char *ip) {return inet_aton(ip,&(sin->sin_addr));}
SIN_EXTEND char *sin_get_ip(sin_t *sin) {return inet_ntoa(sin->sin_addr);}
SIN_EXTEND int sin_get_family(sin_t *sin) {return sin->sin_family;}
int sin_clear_warning_gvxcvxfdghufggwdSHG() 
{
	void *sin=sin_create();
	sin_get_size(sin),sin_set_port(sin,12345),sin_get_port(sin),sin_set_ip(sin,"127.0.0.1"),sin_get_ip(sin),sin_get_family(sin);
	sin_free(sin);
	return 0;
}




#define INET_TO_ADDR_STR_AREA
int addr_to_ip_and_port(char *addr,char *ip,int *port)
{
	int ret=FT_ASSERT( sscanf(addr,"[%[^,^ ],%d]",ip,port); ,<0, return -1 );
	return ret;
}
int addr_by_ip_and_port(char *addr,char *ip,int port)
{
	int ret=FT_ASSERT( sprintf(addr,"[%s, %d]",ip,port); ,<0, return -1 );
	return ret;
}
int sin_set_ip_and_port_by_addr(void *sin,char *addr)
{
	char ip[64]={0};int port=0;
	FT_ASSERT( addr_to_ip_and_port(addr,ip,&port) ,<0, return -1 );
	sin_set_ip(sin,ip);
	sin_set_port(sin,port);
	return 0;
}
int sin_get_addr(void *sin,char *addr)
{
	FT_ASSERT( addr_by_ip_and_port(addr,sin_get_ip(sin),sin_get_port(sin)); ,<0, return -1 );
	return 0;
}
#undef INET_TO_ADDR_STR_AREA




int sin_test()
{
	{
		char addr[]="[\"127.0.0.1\", 12345]";
		char ip[64]={0};int port=0;addr_to_ip_and_port(addr,ip,&port);ERR("ip=%s,port=%d",ip,port);
	}
	{
		char addr[]="[\"127.0.0.1\",12345]";
		char ip[64]={0};int port=0;addr_to_ip_and_port(addr,ip,&port);ERR("ip=%s,port=%d",ip,port);
	}
	{
		char addr[]="[127.0.0.1, 12345]";
		char ip[64]={0};int port=0;addr_to_ip_and_port(addr,ip,&port);ERR("ip=%s,port=%d",ip,port);
	}
	{	
		char addr[]="[127.0.0.1,12345]";
		char ip[64]={0};int port=0;addr_to_ip_and_port(addr,ip,&port);ERR("ip=%s,port=%d",ip,port);
	}
	{
		char *ip="127.0.0.1";int port=12345;char addr[128]={0};
		addr_by_ip_and_port(addr,ip,port);ERR("addr=%s",addr);
	}
	return 0;
}


