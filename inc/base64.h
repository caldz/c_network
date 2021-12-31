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
 

#ifndef BASE64_H
#define BASE64_H


void *base64_mgr_create();
int base64_mgr_free(void *base64_mgr);
char *base64_mgr_encode(void *base64_mgr,char *bdata,int bdata_len);
char *base64_mgr_decode(void *base64_mgr,char *base64_str,int *bdata_len);

#endif//BASE64_H
