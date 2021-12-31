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

#include "pax_log.h"



char *base64_table()
{
	return "ABCDEFGHIJKLMNOPQRZTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
}
typedef char ret_rtable_t;
ret_rtable_t *base64_r_table()
{
	static ret_rtable_t rtable[]={
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3e,0x00,0x00,0x00,0x3f,
		0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,
		0x0f,0x10,0x11,0x00,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x00,0x00,0x00,0x00,0x00,
		0x00,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,
		0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,0x30,0x31,0x32,0x33,0x00,0x00,0x00,0x00,0x00
	};
	return rtable;
}
//ret_rtable_t *base64_r_table()
//{
//	static ret_rtable_t rtable[128]={0};
//	static int first=1;
//	if (!first) return rtable;
//	
//	char *b64_table=base64_table();
//	int b64_i;int base64_map_size=64;int asc_map_size=128;
//	for (b64_i=0;b64_i!=base64_map_size;++b64_i) {
//		int asc_i;for (asc_i=0;asc_i!=asc_map_size;++asc_i) {
//			if (asc_i==b64_table[b64_i]) rtable[asc_i]=b64_i;
//		}
//	}
//
//	first=0;
//	return rtable;
//}




#define bin_bytes_to_base64_ch(table,bin_bytes_exp) ({\
	table[bin_bytes_exp];\
})


typedef unsigned long len_t;
char *bdata_to_base64_str(char *bdata,len_t bdata_len)
{
	if (!bdata || bdata_len <=0) return 0;
	
	len_t full_group_cnt=bdata_len/3;
	int last_group_byte_cnt=bdata_len%3;
	len_t final_char_len=(!last_group_byte_cnt) ? full_group_cnt*4 : (full_group_cnt+1)*4;
	char *base64_str=c_malloc(sizeof(char)*final_char_len+1);

//	ERR("bdata_len=%d,full_group_cnt=%d,last_group_byte_cnt=%d,final_char_len=%d",bdata_len,full_group_cnt,last_group_byte_cnt,final_char_len);

	char *table=base64_table();
	
	len_t i,j;
	for (i=0,j=0;i!=bdata_len-last_group_byte_cnt;j+=4,i+=3) {
		base64_str[j+0]=table[bdata[i]>>2];
		base64_str[j+1]=table[(bdata[i]&0x03)<<4 | bdata[i+1]>>4];
		base64_str[j+2]=table[(bdata[i+1]&0x0f)<<2 | bdata[i+2]>>6];
		base64_str[j+3]=table[bdata[i+2]&0x3f];
	}

	if (last_group_byte_cnt==1) {
		base64_str[j+0]=table[bdata[i]>>2];
		base64_str[j+1]=table[(bdata[i]&0x03)<<4];
		base64_str[j+2]='=';
		base64_str[j+3]='=';
	} else if (last_group_byte_cnt==2) {
		base64_str[j+0]=table[bdata[i]>>2];
		base64_str[j+1]=table[(bdata[i]&0x03)<<4 | bdata[i+1]>>4];
		base64_str[j+2]=table[(bdata[i+1]&0x0f)<<2];
		base64_str[j+3]='=';
	}

	
	return base64_str;
}


typedef struct {
	int result;
	char *bdata;
	len_t len;
} ret_base64_str_to_bdata_t;
ret_base64_str_to_bdata_t base64_str_to_bdata(char *base64_str)
{
	ret_base64_str_to_bdata_t error_sret={.result=-1,.bdata=0,.len=0};
	len_t string_len=strlen(base64_str);
	FT_ASSERT(  (int)string_len ,<=0 || (string_len%4)!=0, return error_sret );
	
	len_t last_group_byte_cnt=(base64_str[string_len-1]=='=') ? ((base64_str[string_len-2]=='=') ? 1 : 2) : 0;
	len_t full_group_cnt=(last_group_byte_cnt) ? string_len/4-1 : string_len/4;
	len_t bdata_len=full_group_cnt*3+last_group_byte_cnt;

	
	char *binary_data=c_malloc(bdata_len*sizeof(char));
	ret_rtable_t *rtable=base64_r_table();
	len_t i,j;len_t count=full_group_cnt*3;
	for (i=0,j=0;i!=count;i+=3,j+=4) {
		char val0=rtable[(int)base64_str[j+0]];
		char val1=rtable[(int)base64_str[j+1]];
		char val2=rtable[(int)base64_str[j+2]];
		char val3=rtable[(int)base64_str[j+3]];
		binary_data[i+0]=(val0&0b00111111)<<2 | (val1&0b00110000)>>4;
		binary_data[i+1]=(val1&0b00001111)<<4 | (val2&0b00111100)>>2;
		binary_data[i+2]=(val2&0b00000011)<<6 | (val3&0b00111111)>>0;
	}
	
	if (last_group_byte_cnt==1) {
		char val0=rtable[(int)base64_str[j+0]];
		char val1=rtable[(int)base64_str[j+1]];
		binary_data[i+0]=(val0&0b00111111)<<2 | (val1&0b00110000)>>4;
	} else if (last_group_byte_cnt==2) {
		char val0=rtable[(int)base64_str[j+0]];
		char val1=rtable[(int)base64_str[j+1]];
		char val2=rtable[(int)base64_str[j+2]];
		binary_data[i+0]=(val0&0b00111111)<<2 | (val1&0b00110000)>>4;
		binary_data[i+1]=(val1&0b00001111)<<4 | (val2&0b00111100)>>2;
	}

	ret_base64_str_to_bdata_t sret={.result=0,.bdata=binary_data,.len=bdata_len};
	return sret;
}


typedef struct {
	void *p;
} base64_mgr_t;
void *base64_mgr_create()
{
	base64_mgr_t *base64_mgr=c_malloc(sizeof(base64_mgr_t));
	return base64_mgr;
}
int base64_mgr_free(base64_mgr_t *base64_mgr)
{
	c_free(base64_mgr->p);
	c_free(base64_mgr);
	return 0;
}
char *base64_mgr_encode(base64_mgr_t *base64_mgr,char *bdata,int bdata_len)
{
	c_free(base64_mgr->p);
	base64_mgr->p=bdata_to_base64_str(bdata,bdata_len);
	return base64_mgr->p;
}
char *base64_mgr_decode(base64_mgr_t *base64_mgr,char *base64_str,int *bdata_len)
{
	c_free(base64_mgr->p);
	ret_base64_str_to_bdata_t sret=base64_str_to_bdata(base64_str);
	if (!sret.result) {
		*bdata_len=sret.len;
	}
	base64_mgr->p=sret.bdata;
	return base64_mgr->p;
}




int base64_test()
{
	void *base64_mgr=base64_mgr_create();
	{char *bdata="abcde";int len=strlen(bdata);ERR("%s->%s",get_buf_str(bdata,len),base64_mgr_encode(base64_mgr,bdata,len));}
	{char *bdata="1";int len=strlen(bdata);ERR("%s->%s",get_buf_str(bdata,len),base64_mgr_encode(base64_mgr,bdata,len));}
	{char *bdata="12";int len=strlen(bdata);ERR("%s->%s",get_buf_str(bdata,len),base64_mgr_encode(base64_mgr,bdata,len));}
	{char *bdata="123";int len=strlen(bdata);ERR("%s->%s",get_buf_str(bdata,len),base64_mgr_encode(base64_mgr,bdata,len));}
	{char *bdata="1234";int len=strlen(bdata);ERR("%s->%s",get_buf_str(bdata,len),base64_mgr_encode(base64_mgr,bdata,len));}
	{char b64str[]="YWJjZGU=";int len=0;char *bdata=base64_mgr_decode(base64_mgr,b64str,&len);ERR("%s->%s",b64str,get_buf_str(bdata,len));}
	{char b64str[]="MQ==";int len=0;char *bdata=base64_mgr_decode(base64_mgr,b64str,&len);ERR("%s->%s",b64str,get_buf_str(bdata,len));}
	{char b64str[]="MTI=";int len=0;char *bdata=base64_mgr_decode(base64_mgr,b64str,&len);ERR("%s->%s",b64str,get_buf_str(bdata,len));}
	{char b64str[]="MTIz";int len=0;char *bdata=base64_mgr_decode(base64_mgr,b64str,&len);ERR("%s->%s",b64str,get_buf_str(bdata,len));}
	{char b64str[]="MTIzNA==";int len=0;char *bdata=base64_mgr_decode(base64_mgr,b64str,&len);ERR("%s->%s",b64str,get_buf_str(bdata,len));}
	base64_mgr_free(base64_mgr);
	return 0;
	
}

