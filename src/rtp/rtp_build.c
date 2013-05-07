/*****************************************************************************
*
*rtsp.c
*============================================================================
* This file is used for formating h.264/aac  to rtsp flow
* Ver      alpha_1.0
* Author  jbx
* Shenzhen reach 2010.9.6
*============================================================================
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <memory.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/types.h>
#include <pthread.h>
#include <time.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "rtp_build.h"
#include "rtp_struct.h"


typedef struct _RTP_BUILD_INFO_
{
	unsigned int time_org;
	unsigned int seq_num;
	unsigned int ssrc;
	unsigned int payload;
	unsigned int mtu;
	
}RTP_BUILD_INFO;
typedef RTP_BUILD_INFO* RTP_BUILD_HANDLE;


typedef struct Rtp_module_info
{
	int rtp_module_falg;
	int rtp_stream_num;

	unsigned int rtp_stream_mtu;
//	unsigned int rtp_stream_payload;
//	unsigned int rtp_stream_ssrc;
		
	app_stream_output_rtp fp;

	RTP_BUILD_HANDLE rtp_stream_list;
	
}Rtp_module_info_t;

static Rtp_module_info_t g_rtp_info;


// add zhengyb
static int set_app_stream_out_rtp(app_stream_output_rtp fp)
{
	if(fp == NULL)
	{
		return RTP_RET_FAILE;
	}
	
	if(g_rtp_info.rtp_module_falg != RTP_MODULE_ON)
	{
		return RTP_RET_FAILE;
	}

	g_rtp_info.fp = fp;

	return RTP_RET_SUCESS;
}


static int set_stream_num_rtp(int str_num)
{
	if(g_rtp_info.rtp_module_falg != RTP_MODULE_ON)
	{
		return RTP_RET_FAILE;
	}

	g_rtp_info.rtp_stream_num = str_num;

	return RTP_RET_SUCESS;
	
}

static int get_stream_num_rtp(int *str_num)
{
	if(str_num == NULL)
	{
		return RTP_RET_FAILE;
	}

	if(g_rtp_info.rtp_module_falg != RTP_MODULE_ON)
	{
		return RTP_RET_FAILE;
	}

	*str_num = g_rtp_info.rtp_stream_num;

	return RTP_RET_SUCESS;
	
}

static int set_stream_mtu_rtp(int str_mtu)
{
	if(g_rtp_info.rtp_module_falg != RTP_MODULE_ON)
	{
		return RTP_RET_FAILE;
	}

	g_rtp_info.rtp_stream_mtu = str_mtu;

	return RTP_RET_SUCESS;
	
}

static int get_stream_mtu_rtp(int *str_mtu)
{
	if(str_mtu == NULL)
	{
		return RTP_RET_FAILE;
	}

	if(g_rtp_info.rtp_module_falg != RTP_MODULE_ON)
	{
		return RTP_RET_FAILE;
	}

	*str_mtu = g_rtp_info.rtp_stream_mtu;

	return RTP_RET_SUCESS;
	
}

static int reset_stream_time_rtp(int str_id)
{
	if(g_rtp_info.rtp_module_falg != RTP_MODULE_ON)
	{
		return RTP_RET_FAILE;
	}

	if(str_id < 0 || str_id > g_rtp_info.rtp_stream_num)
	{
		return RTP_RET_FAILE;
	}

	if(g_rtp_info.rtp_stream_list == NULL)
	{
		return RTP_RET_FAILE;
	}

	g_rtp_info.rtp_stream_list[str_id].time_org = 0;
	
	return RTP_RET_SUCESS;

}


/********************************************************************************************************************
发送 RTP video 数据
*********************************************************************************************************************/
static int SendMultCastVideoData(char *pData, int length,void *info)
{
//	rtp_porting_senddata(2, pData, length, 0,info);

	if(g_rtp_info.fp == NULL)
	{
		return RTP_RET_FAILE;
	}
	if(g_rtp_info.fp(pData,length,info) == RTP_RET_FAILE)
	{
		return RTP_RET_FAILE;
	}
	return RTP_RET_SUCESS;

}

/********************************************************************************************************************
发送 RTP audio 数据
*********************************************************************************************************************/
static int SendMultCastAudioData(char *pData, int length,void *info)
{
	//PRINTF("len = %d\n",length);
//	rtp_porting_senddata(1, pData, length, 0,info);

	if(g_rtp_info.fp == NULL)
	{
		return RTP_RET_FAILE;
	}
	if(g_rtp_info.fp(pData,length,info) == RTP_RET_FAILE)
	{
		return RTP_RET_FAILE;
	}
	return RTP_RET_SUCESS;
}


static int SendRtpNalu(NALU_t *nalu , unsigned short *seq_num, unsigned long ts_current , int roomid, int end, int mtu,void *info)
{
	int 	rtp_mtu = mtu;
	char				 sendbuf[1500] = {0};
	RTP_FIXED_HEADER			*rtp_hdr;
	char                        *nalu_payload;
	FU_INDICATOR	            *fu_ind;
	FU_HEADER		            *fu_hdr;
	int                          bytes = 0;

	int total_len = 0;
	memset(sendbuf, 0, 20);
	//设置RTP HEADER，
	rtp_hdr 							= (RTP_FIXED_HEADER *)&sendbuf[0];
	rtp_hdr->payload	                = H264;
	rtp_hdr->version	                = 2;
	rtp_hdr->marker                     = 0;
	rtp_hdr->ssrc		                = htonl(10);
	rtp_hdr->timestamp		            = htonl(ts_current);

	if((nalu->len - 1) <= rtp_mtu) {
		//设置rtp M 位；

		rtp_hdr->marker = 1;
		rtp_hdr->seq_no 	= htons((*seq_num)++); //序列号，每发送一个RTP包增1
		memcpy(&sendbuf[12], nalu->buf, nalu->len);
		bytes = nalu->len + 12 ; 					//获得sendbuf的长度,为nalu的长度（包含NALU头但除去起始前缀）加上rtp_header的固定长度12字节
		//	SendRtspVideoData(sendbuf, bytes,nalu->nal_unit_type,roomid);
		SendMultCastVideoData(sendbuf, bytes,info);
		total_len = nalu->len;
	} else if((nalu->len - 1) > rtp_mtu) {
		//得到该nalu需要用多少长度为1400字节的RTP包来发送
		int k = 0, l = 0;
		int t = 0; //用于指示当前发送的是第几个分片RTP包

		l = (nalu->len - 1) % rtp_mtu; //最后一个RTP包的需要装载的字节数

		if(l == 0) {
			k = (nalu->len - 1) / rtp_mtu - 1; //需要k个1400字节的RTP包
			l = rtp_mtu;
		} else {
			k = (nalu->len - 1) / rtp_mtu; //需要k个1400字节的RTP包
		}

		while(t <= k) {
			rtp_hdr->seq_no = htons((*seq_num)++); //序列号，每发送一个RTP包增1

			if(!t) { //发送一个需要分片的NALU的第一个分片，置FU HEADER的S位
				//设置rtp M 位；
				rtp_hdr->marker = 0;
				//设置FU INDICATOR,并将这个HEADER填入sendbuf[12]
				fu_ind = (FU_INDICATOR *)&sendbuf[12]; //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；
				fu_ind->F = nalu->forbidden_bit;
				fu_ind->NRI = nalu->nal_reference_idc >> 5;
				fu_ind->TYPE = 28;

				//设置FU HEADER,并将这个HEADER填入sendbuf[13]
				fu_hdr = (FU_HEADER *)&sendbuf[13];
				fu_hdr->E = 0;
				fu_hdr->R = 0;
				fu_hdr->S = 1;
				fu_hdr->TYPE = nalu->nal_unit_type;

				nalu_payload = &sendbuf[14]; //同理将sendbuf[14]赋给nalu_payload
				memcpy(nalu_payload, nalu->buf + 1, rtp_mtu); //去掉NALU头

				//	PRINTF("%x,ser_no=%d\n",nalu_payload[0],*seq_num);
				bytes = rtp_mtu + 14;						//获得sendbuf的长度,为nalu的长度（除去起始前缀和NALU头）加上rtp_header，fu_ind，fu_hdr的固定长度14字节
				//	SendRtspVideoData(sendbuf, bytes,0,roomid);
				SendMultCastVideoData(sendbuf, bytes,info);
				total_len += rtp_mtu;
				t++;
			}
			//发送一个需要分片的NALU的非第一个分片，清零FU HEADER的S位，如果该分片是该NALU的最后一个分片，置FU HEADER的E位
			else if(k == t) { //发送的是最后一个分片，注意最后一个分片的长度可能超过1400字节（当l>1386时）。
				//设置rtp M 位；当前传输的是最后一个分片时该位置1
				rtp_hdr->marker = 1;
				//设置FU INDICATOR,并将这个HEADER填入sendbuf[12]
				fu_ind = (FU_INDICATOR *)&sendbuf[12]; //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；
				fu_ind->F = nalu->forbidden_bit;
				fu_ind->NRI = nalu->nal_reference_idc >> 5;
				fu_ind->TYPE = 28;

				//设置FU HEADER,并将这个HEADER填入sendbuf[13]
				fu_hdr = (FU_HEADER *)&sendbuf[13];
				fu_hdr->R = 0;
				fu_hdr->S = 0;
				fu_hdr->TYPE = nalu->nal_unit_type;
				fu_hdr->E = 1;

				nalu_payload = &sendbuf[14]; //同理将sendbuf[14]的地址赋给nalu_payload
				memcpy(nalu_payload, nalu->buf + t * rtp_mtu + 1, l); //将nalu最后剩余的l-1(去掉了一个字节的NALU头)字节内容写入sendbuf[14]开始的字符串。
				bytes = l + 14;		//获得sendbuf的长度,为剩余nalu的长度l-1加上rtp_header，FU_INDICATOR,FU_HEADER三个包头共14字节
				//	PRINTF("%x,ser_no=%d\n",nalu_payload[0],*seq_num);
				//		SendRtspVideoData(sendbuf, bytes,0,roomid);
				SendMultCastVideoData(sendbuf, bytes,info);
				total_len += l;
				t++;
			} else if(t < k && 0 != t) {
				//设置rtp M 位；
				rtp_hdr->marker = 0;
				//设置FU INDICATOR,并将这个HEADER填入sendbuf[12]
				fu_ind = (FU_INDICATOR *)&sendbuf[12]; //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；
				fu_ind->F = nalu->forbidden_bit;
				fu_ind->NRI = nalu->nal_reference_idc >> 5;
				fu_ind->TYPE = 28;

				//设置FU HEADER,并将这个HEADER填入sendbuf[13]
				fu_hdr = (FU_HEADER *)&sendbuf[13];
				fu_hdr->R = 0;
				fu_hdr->S = 0;
				fu_hdr->E = 0;
				fu_hdr->TYPE = nalu->nal_unit_type;

				nalu_payload = &sendbuf[14]; //同理将sendbuf[14]的地址赋给nalu_payload
				memcpy(nalu_payload, nalu->buf + t * rtp_mtu + 1, rtp_mtu); //去掉起始前缀的nalu剩余内容写入sendbuf[14]开始的字符串。
				bytes = rtp_mtu + 14;						//获得sendbuf的长度,为nalu的长度（除去原NALU头）加上rtp_header，fu_ind，fu_hdr的固定长度14字节

				//	SendRtspVideoData(sendbuf, bytes,0,roomid);
				SendMultCastVideoData(sendbuf, bytes,info);
				total_len += rtp_mtu;
				t++;
			}
		}
	}

	//if(total_len+1 != nalu->len )
	//{
	//	PRINTF("nalu send len =%d,nalu len= %d\n",total_len,nalu->len);
	//}
	return RTP_RET_SUCESS;
}

//static unsigned short                           g_seq_num = 0;
//static int rtp_build_video_data(RTP_BUILD_HANDLE handle,int nLen, unsigned char *pData, int nFlag, int rtp_mtu, unsigned int nowtime, void *info)
//(int nLen,  unsigned char *pData, int nFlag,unsigned int timetick ,int *seq,int roomid,int idr_flag)
static int rtp_build_video_data(int str_id,int nLen, unsigned char *pData, int nFlag, unsigned int nowtime, void *info)
{
	if(g_rtp_info.rtp_module_falg != RTP_MODULE_ON)
	{
		return RTP_RET_FAILE;
	}

	if(g_rtp_info.fp == NULL)
	{
		return RTP_RET_FAILE;
	}
	
	if(str_id < 0 || str_id > g_rtp_info.rtp_stream_num)
	{
		return RTP_RET_FAILE;
	}

	RTP_BUILD_HANDLE handle = g_rtp_info.rtp_stream_list[str_id];
	
	if(handle == NULL)
	{
		//PRINTF("ERROR,the video handle is NULL\n");
		return RTP_RET_FAILE;
	}

	int rtp_mtu = g_rtp_info.rtp_stream_list[str_id]->mtu ;

	// ???zhengyb
	if(rtp_mtu < 228) {
		//PRINTF("Error\n");
		return RTP_RET_FAILE;
	}

	//int time_org = handle->time_org;
	
	int roomid = 0;
	unsigned int timetick = 0;
	unsigned short *seq = &(handle->seq_num);
	//printf("nowtime =%u==%u\n",nowtime,g_time_org);
	if((handle->time_org) == 0 || nowtime < (handle->time_org)) {
		(handle->time_org) = nowtime;
	}

	timetick = nowtime - (handle->time_org);

	NALU_t		nalu;
	unsigned char *pstart;

	unsigned long ts_current  = 0;
	unsigned long mytime = 0;

	unsigned char *pos = pData;
	unsigned char *ptail = pData + nLen - 4;
	unsigned char *temp1, *temp2;

	unsigned char nalu_value = 0;
	temp1 = temp2 = NULL;
	int send_total_len = 0;

	//14 = rtp head len +2
	int mtu = rtp_mtu - 14;
	int cnt = 0;

	for(;;) {
		mytime = (unsigned long)timetick;
		//ts_current = mytime * 90;
	//	printf("mytiem=%x\n",mytime);
	//	mytime +=  (0xffffffff/44-1000*120);
		ts_current =mytime * 90 ;

		if(ts_current >= 0XFFFFFFFF)
		{
				PRINTF("Warnning,the ts_current is too big =%lx\n",ts_current);
				(handle->time_org) = 0;
		}

					
	//	printf("video_time = %x\n",ts_current);
		pstart = pos;
		memset(&nalu, 0 , sizeof(nalu));

		nalu_value = 0;

		//判断头是否nalu头 0001 或者 001 ,不是退出
		if(!((*pos == 0 && *(pos + 1) == 0 && *(pos + 2) == 0 && *(pos + 3) == 1))) {
			PRINTF("read nalu header failed!\n");
		} else {
			temp1 = pos;
			nalu_value = *(pos + 4);
		}

		if(((nalu_value & 0x1f) == 7) || ((nalu_value & 0x1f) == 8)) {
			//找到下一个nalu头 0001 或者 001 , 或者到帧尾
			do {
				pos++;
			} while((*pos != 0 || *(pos + 1) != 0 || *(pos + 2) != 0 || *(pos + 3) != 1)
			        && (pos < ptail));
		} else {
			pos = ptail;
		}

		if(pos >= ptail)

		{
			//如果是到达帧尾， 则把整个剩余数据作为一个nalu单元发送
			nalu.buf = pstart + 4;
			nalu.len = pData - pstart + nLen  - 4 ;
			//PRINTF("nalu_len=%d,the len =%d,%d,%p,%p,%p\n",nalu.len,temp2-temp1 ,ptail-pstart,pstart,pos,ptail);
			send_total_len += (nalu.len + 4);
			nalu.forbidden_bit = nalu.buf[0] & 0x80;
			nalu.nal_reference_idc = nalu.buf[0] & 0x60;
			nalu.nal_unit_type = (nalu.buf[0]) & 0x1f;
			/*
			                        DEBUG(DL_FLOW,"send last nalu pkt! len = %d frame_len = %d flag = %d \
			                        pdata = 0x%x pos = 0x%x pstart = 0x%x\n", nalu.len, nLen ,nFlag\
			                        , pData , pos , pstart);
			*/
			SendRtpNalu(&nalu, (unsigned short *)seq, ts_current, roomid, 1, mtu,info);
			//PRINTF("seq =%d,time =%ld\n",seq,ts_current);
			break;
		} else {
			//发送一个nalu单元
			nalu.buf = pstart + 4;
			nalu.len = pos - pstart - 4;
			send_total_len += (nalu.len + 4);
			nalu.forbidden_bit = nalu.buf[0] & 0x80;
			nalu.nal_reference_idc = nalu.buf[0] & 0x60;
			nalu.nal_unit_type = (nalu.buf[0]) & 0x1f;
			/*
			                        DEBUG(DL_FLOW,"send nalu pkt! len = %d frame_len = %d flag = %d\
			                        pdata = 0x%x pos = 0x%x pstart = 0x%x\n", nalu.len, nLen ,nFlag\
			                        , pData , pos , pstart);
			 */
			SendRtpNalu(&nalu, (unsigned short *)seq, ts_current, roomid, 0, mtu,info);
			//PRINTF("seq =%d,time =%ld\n",seq,ts_current);
		}
	}

	if(send_total_len != nLen) {
		PRINTF("send_total_len = %d,nLen=%d\n", send_total_len, nLen);
	}

	return RTP_RET_SUCESS;
}


//static unsigned short g_audio_seq_num = 0;
//static int rtp_build_audio_data(RTP_BUILD_HANDLE handle,int nLen, unsigned char *pData, int samplerate, int rtp_mtu, unsigned int nowtime,void *info)
static int rtp_build_audio_data(int str_id,int nLen, unsigned char *pData, int samplerate, unsigned int nowtime,void *info)
{
	if(g_rtp_info.rtp_module_falg != RTP_MODULE_ON)
	{
		return RTP_RET_FAILE;
	}

	if(g_rtp_info.fp == NULL)
	{
		return RTP_RET_FAILE;
	}

	if(str_id < 0 || str_id > g_rtp_info.rtp_stream_num)
	{
		return RTP_RET_FAILE;
	}

	RTP_BUILD_HANDLE handle = g_rtp_info.rtp_stream_list[str_id];

	if(handle == NULL)
	{
		//PRINTF("ERROR,the audio handle is NULL\n");
		return RTP_RET_FAILE;
	}
	int rtp_mtu = g_rtp_info.rtp_stream_list[str_id]->mtu ;

	int payload = handle->payload;
	int ssrc = handle->ssrc;
	
	int begin_len = 0;
	unsigned char                   sendbuf[1500] = {0};
	//unsigned char                           *sendbuf = gszAudioRtspBuf;
	int                             pLen;
	int                             offset[5] = {0};
	int                             ucFrameLenH = 0, ucFrameLenL = 0;
	int	                            bytes = 0;

	unsigned long                   mytime = 0;
	unsigned long                   ts_current_audio = 0;
	int                     			audio_sample = samplerate; //rtsp_stream_get_audio_samplerate();
	int audio_frame_len = 0;
	int temp_len = 0;

	char audio_puiNalBuf[2048] = {0};
	int  audio_nal_len = 0;
	int i = 0;
	int j = 0;
	//14 = rtp head len +2
	int mtu = rtp_mtu - 14;
	/*临时版本modify by zm  2012.04.27  */
	//int mtu = nLen + 12;
	int roomid = 0;
	unsigned int timetick = 0;
	int *seq = &(handle->seq_num);
//	printf("AUDIO nowtime =%u==%u\n",nowtime,g_time_org);
	//printf("nowtime=0x%x,g_time_org=0x%x\n",nowtime,g_time_org);
	if((handle->time_org)== 0 || nowtime < (handle->time_org)) {
		(handle->time_org) = nowtime;
	}

	timetick = nowtime - (handle->time_org);


	unsigned int framelen = 0;
	framelen = ((pData[3] & 0x03) << 9) | (pData[4] << 3) | ((pData[5] & 0xe0) >> 5);


	//PRINTF("pData[0] =%x,%x,%x,%x,len=%d\n",pData[0],pData[1],pData[2],pData[3],nLen)	;
	for(i = 0; i < nLen - 4; i++) {
		if(pData[i] == 0xff && pData[i + 1] == 0xf1)
		{
				//just 48KHZ/44.1KZH
			if((pData[i + 2] == 0x58) || (pData[i + 2] == 0x5c)||
				 (pData[i + 2] == 0x6c)|| (pData[i + 2] == 0x60)||
				 (pData[i + 2] == 0x4c)|| (pData[i + 2] == 0x50))
			{
				offset[j] = i;
				j++;
			}
		}
	}
	//	if((pData[i] == 0xff && pData[i + 1] == 0xf1 && pData[i + 2] == 0x58)
	//	   || (pData[i] == 0xff && pData[i + 1] == 0xf1 && pData[i + 2] == 0x5c)
	//	   || (pData[i] == 0xff && pData[i + 1] == 0xf1 && pData[i + 2] == 0x6c)
	//	   || (pData[i] == 0xff && pData[i + 1] == 0xf1 && pData[i + 2] == 0x60)
	//	   || (pData[i] == 0xff && pData[i + 1] == 0xf1 && pData[i + 2] == 0x4c)) {



	if(j > 1) {
		PRINTF("RtspAudioPack j=%d=%d\n", j, nLen);
		//printf("pData[0] =%x,%x,%x,%x,len=%d\n",pData[0],pData[1],pData[2],pData[3],nLen)	;
		//printf("pData[0] =%x,%x,%x,%x,len=%d\n",pData[4],pData[5],pData[6],pData[7],nLen)	;
	}

	if(framelen == nLen && j >= 1) {
		j = 1;
	}


	for(i = 0; i < j; i++) {

		pLen = offset[i + 1] - offset[i];

		if(i == j - 1) {
			pLen = nLen - offset[i];
		}

		temp_len = audio_frame_len = pLen - 7;
		//PRINTF("RtspAudioPack  temp_len = %d\n",temp_len);
		mytime = (unsigned long)timetick ;

		while(temp_len > 0) {
			if(temp_len >=  mtu) {
				//g_audio_puiNalBuf->len = mtu;
				audio_nal_len = mtu;
				//PRINTF("temp_len = %d,mtu=%d\n", temp_len, mtu);
				//PRINTF("i=%d,the len =%d=0x%x\n", i, g_audio_puiNalBuf->len, g_audio_puiNalBuf->len);
			} else {
				//g_audio_puiNalBuf->len = temp_len;
				audio_nal_len = temp_len;
			}

			ucFrameLenH = audio_frame_len / 32;
			ucFrameLenL = (audio_frame_len % 32) * 8;

		//	g_audio_puiNalBuf->buf[0] = 0;
		//	g_audio_puiNalBuf->buf[1] = 0x10;
		//	g_audio_puiNalBuf->buf[2] = ucFrameLenH;
		//	g_audio_puiNalBuf->buf[3] = ucFrameLenL;
			
			audio_puiNalBuf[0] = 0;
			audio_puiNalBuf[1] = 0x10;
			audio_puiNalBuf[2] = ucFrameLenH;
			audio_puiNalBuf[3] = ucFrameLenL;
			begin_len =  offset[i] + 7 + audio_frame_len - temp_len;
			memcpy(&audio_puiNalBuf[4], pData +begin_len , audio_nal_len);

			//intf("begin_len=%d,the len=%d\n",begin_len,g_audio_puiNalBuf->len);
			
			//temp_len -= 	g_audio_puiNalBuf->len;
			temp_len -= 	audio_nal_len;
			memset(sendbuf, 0, sizeof(sendbuf));

			//mytime += (0xffffffff/44.1-1000*120);
			
			if(audio_sample == 16000) {
				ts_current_audio = mytime * 16;
			} else if(audio_sample == 32000) {
				ts_current_audio = mytime * 32;
			} else if(audio_sample == 44100) {
				ts_current_audio = mytime * 44 +mytime/10;
			} else if(audio_sample == 96000) {
				ts_current_audio =  mytime * 96;
			} else {
				ts_current_audio = mytime * 48;
			}
		//	ts_current_audio = ts_current_audio/2;
			//printf("ts_current_audio =%lx,my=%x==%d\n",ts_current_audio,mytime,timetick);
			//if audio time is >0xffffffff,need reset 0;
			if(ts_current_audio >= 0XFFFFFFFF)
			{
				PRINTF("Warnning,the ts_current_audio is too big =%lx\n",ts_current_audio);
				(handle->time_org) = 0;
			}
			//ts_current_audio = timetick;
			//	static int g_test_audio = 0;
			//	g_test_audio ++;
			//	if(g_test_audio %40 ==0)
			//	printf("audio timetick =0x%x,the ts_current_audio  =0x%x\n",timetick,ts_current_audio);
			//bytes = g_audio_puiNalBuf->len + 16 ;
			bytes = audio_nal_len + 16 ;

			RTP_FIXED_HEADER_AUDIO *p;
			p = (RTP_FIXED_HEADER_AUDIO *)sendbuf;
			p->byte1 = 0x80;

			if(temp_len != 0) {
				p->byte2 = 0x61;
			} else {
				p->byte2 = 0xe1;
			}

			p->seq_no = htons((*seq)++);
			p->timestamp = htonl(ts_current_audio);
			p->ssrc = htonl(ssrc);
			memcpy(&sendbuf[12], audio_puiNalBuf, audio_nal_len+ 4);

			if(j == 1) {
				//		SendRtspAudioData(sendbuf,bytes,roomid);
				SendMultCastAudioData(sendbuf, bytes,info);
			} else {
				PRINTF("ERROR!!!!SendRtspAudioData not send.\n");
			}
		
		}

	}

	//printf("audio bytes =%d,the len=%d\n",bytes,nLen);
	return RTP_RET_SUCESS;
}



static int init_rtp_module(unsigned int ssrc, unsigned int payload,unsigned int str_num,unsigned int str_mtu)
{
	int index = 0;
	memset(&g_rtp_info , 0, sizeof(Rtp_module_info_t));
	
	g_rtp_info.rtp_module_falg = RTP_MODULE_ON;
	g_rtp_info.fp = NULL;

	if(str_mtu == 0)
	{
		g_rtp_info.rtp_stream_mtu = RTP_PKG_MTU_DEFAULT;
	}

	
	g_rtp_info.rtp_stream_num = str_num;
	g_rtp_info.rtp_stream_list = (RTP_BUILD_HANDLE)malloc(sizeof(RTP_BUILD_INFO) * str_num);

	if(g_rtp_info.rtp_stream_list == NULL)
	{
		return RTP_RET_FAILE;
	}
	for(index = 0; index < str_num ;index ++)
	{
		g_rtp_info.rtp_stream_list[index]->mtu 		= g_rtp_info.rtp_stream_mtu;
		g_rtp_info.rtp_stream_list[index]->time_org	= 0;
		g_rtp_info.rtp_stream_list[index]->seq_num	= 0;
		g_rtp_info.rtp_stream_list[index]->ssrc		= ssrc;
		g_rtp_info.rtp_stream_list[index]->payload	= payload;
	}

	return RTP_RET_SUCESS;
}

static int uninit_rtp_module(void)
{
	g_rtp_info.rtp_module_falg = RTP_MODULE_OFF;
	if(g_rtp_info.rtp_stream_list != NULL)
	{
		free(g_rtp_info.rtp_stream_list)
		g_rtp_info.rtp_stream_list = NULL;	
	}
	return RTP_RET_SUCESS;
}


Rtp_module_handle_t *Register_Rtp_module(unsigned int ssrc, unsigned int payload,unsigned int str_num,unsigned int str_mtu)
{
	if(str_num == 0)
	{
		return NULL;
	}

	Rtp_module_handle_t *rtp_handle = NULL;
	
	if(g_rtp_info.rtp_module_falg == RTP_MODULE_ON)
	{
		return NULL;
	}

	rtp_handle = malloc(sizeof(Rtp_module_handle_t));

	if(rtp_handle == NULL)
	{
		return NULL;
	}

	rtp_handle->set_app_stream_out_rtp = set_app_stream_out_rtp;

	rtp_handle->set_stream_mtu_rtp = set_stream_mtu_rtp;
	rtp_handle->get_stream_mtu_rtp = get_stream_mtu_rtp;
	rtp_handle->get_stream_num_rtp = get_stream_num_rtp;

	rtp_handle->reset_stream_time_rtp 	= reset_stream_time_rtp;
	rtp_handle->rtp_build_audio_data 	= rtp_build_audio_data;
	rtp_handle->rtp_build_video_data 	= rtp_build_video_data;

	if(init_rtp_module(ssrc,payload,str_num,str_mtu) != RTP_RET_SUCESS)
	{
		free(rtp_handle);
		rtp_handle = NULL;
	}
	
	return rtp_handle;
	
}


int Unregister_Rtp_module(Rtp_module_handle_t *rtp_module_handle)
{
	if(rtp_module_handle == NULL)
	{
		return RTP_RET_FAILE;
	}

	uninit_rtp_module();

	free(rtp_module_handle);
	rtp_module_handle = NULL;

	return RTP_RET_SUCESS;
	
}


