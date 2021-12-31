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




#ifndef SIN_H
#define SIN_H


void *sin_create();
int sin_free(void *sin);

int sin_get_size(void *sin);
int sin_set_port(void *sin,int port);
int sin_get_port(void *sin);
int sin_set_ip(void *sin,char *ip);
char *sin_get_ip(void *sin);
int sin_get_family(void *sin);

int sin_set_ip_and_port_by_addr(void *sin,char *addr);
int sin_get_addr(void *sin,char *addr);



int addr_to_ip_and_port(char *addr,char *ip,int *port);
int addr_by_ip_and_port(char *addr,char *ip,int port);

#endif//#ifndef SIN_H


