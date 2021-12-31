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
 * 20211229 	         CaiDongZe           	Create
 * ===========================================================================================
 */

#ifndef LOCAL_COMPONENT_H
#define LOCAL_COMPONENT_H

enum {
	ls_not_inited,
	ls_handler_work,
	ls_handler_exit,
} loco_stat_t;


#ifndef LOCAL_COMPONENT_SOURCE
void *loco_create();
int loco_free(void *loco);
int loco_init(void *loco);
int loco_set_target_addr(void *loco,char *target_addr);
int loco_set_transfer_addr(void *loco,char *transfer_addr);



int loco_get_state(void *loco);
char *loco_get_finish_message(void *loco);
#endif//#ifndef LOCAL_COMPONENT_SOURCE

#endif//#ifndef LOCAL_COMPONENT_H