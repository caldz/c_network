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


#ifndef SMART_SELECT_H
#define SMART_SELECT_H


#define RET_SMART_SELECT_RAW_AREA
enum {
	cf_read=0x01,
	cf_write=0x01<<1,
	cf_error=0x01<<2,
} cf_type_t;
#undef RET_SMART_SELECT_RAW_AREA



#ifndef SMART_SELECT_SOURCE
void *smart_select_create();
int smart_select_free(void *smart_select);
int smart_select_work(void *smart_select,int *fd_list,int fd_count,int *change_fd_list);

int smart_select_set_timeout(void *smart_select,unsigned int timeout_s,unsigned int timeout_us);

char *smart_select_mode_read();
char *smart_select_mode_write();
int smart_select_set_mode(void *smart_select,char *mode);




int smart_select_super_set_cf_type(void *sel,int cf_type);
int smart_select_super(void *sel,int *change_fd_list,int *pchange_fd_count);
int smart_select_super_add_fd(void *sel,int fd);
int smart_select_super_del_fd(void *sel,int fd);
#endif//#ifndef SMART_SELECT_SOURCE







#endif//#ifndef SMART_SELECT_H
