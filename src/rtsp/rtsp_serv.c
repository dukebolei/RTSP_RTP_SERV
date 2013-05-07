#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <memory.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <math.h>
#include "rtp_build.h"
#include "rtsp_serv.h"

// The internal data structure type in the RTSP_RTP_SERV module

typedef struct Rtsp_net_info
{
	int 					Rtsp_serv_post;
	mid_plat_socket			Rtsp_serv_fd;
	mid_plat_sockaddr_in	Rtsp_serv_addr;
	
}Rtsp_net_info_t;


static Rtsp_module_info_t g_rtst_info;

//the external interface in the RTSP_RTP_SERV module

static int Get_Rtsp_module_flag(int *rtsp_module_flag)
{
	rtsp_module_flag = g_rtst_info.rtsp_module_falg ;
}

static int Set_ClitNum_Rtsp(int rtsp_clit_num)
{
	if(g_rtst_info.rtsp_module_falg != RTSP_MODULE_ON)
	{
		return RTSP_RET_FAILE;
	}

	g_rtst_info.rtsp_client_num = rtsp_clit_num;
}
static int Get_ClitNum_Rtsp(int *rtsp_clit_num)
{
	if(rtsp_clit_num == NULL)
	{
		return RTSP_RET_FAILE;
	}
	rtsp_clit_num = g_rtst_info.rtsp_client_num;
}
static int Set_per_Rtsp(Rtsp_AUTN_t *rtsp_autn_info)
{
	int ret = RTSP_RET_FAILE;

	if(g_rtst_info.rtsp_module_falg != RTSP_MODULE_ON)
	{
		return RTSP_RET_FAILE;
	}
	
	if(rtsp_autn_info == NULL)
	{
		return RTSP_RET_FAILE;
	}

	if(rtsp_autn_info->rtsp_permissions == RTSP_PER_OFF)
	{
		g_rtst_info.rtsp_autn_info.rtsp_permissions = rtsp_autn_info->rtsp_permissions;
		ret = RTSP_RET_SUCESS;
	}
	else if(rtsp_autn_info->rtsp_permissions == RTSP_PER_ON)
	{
		g_rtst_info.rtsp_autn_info.rtsp_permissions = rtsp_autn_info->rtsp_permissions;
		if(strlen(rtsp_autn_info->rtsp_name) != 0 && 
			strlen(rtsp_autn_info->rtsp_pass) != 0 &&
			strlen(rtsp_autn_info->rtsp_realm) != 0)
		{
			memcpy(g_rtst_info.rtsp_autn_info.rtsp_name,rtsp_autn_info->rtsp_name ,RTSP_NAME_MAX_LEN);
			memcpy(g_rtst_info.rtsp_autn_info.rtsp_name,rtsp_autn_info->rtsp_pass ,RTSP_PASS_MAX_LEN);
			memcpy(g_rtst_info.rtsp_autn_info.rtsp_name,rtsp_autn_info->rtsp_realm,RTSP_REALM_MAX_LEN);

			ret = RTSP_RET_SUCESS;
		}
	}
	else
	{
		ret = RTSP_RET_FAILE;
	}
	return ret;
	
}
static int Get_per_Rtsp(Rtsp_AUTN_t *rtsp_autn_info)
{
	if(rtsp_autn_info == NULL)
	{
		return RTSP_RET_FAILE;
	}

	memcpy(rtsp_autn_info->rtsp_name ,g_rtst_info.rtsp_autn_info.rtsp_name,RTSP_NAME_MAX_LEN);
	memcpy(rtsp_autn_info->rtsp_pass ,g_rtst_info.rtsp_autn_info.rtsp_name,RTSP_NAME_MAX_LEN);
	memcpy(rtsp_autn_info->rtsp_realm ,g_rtst_info.rtsp_autn_info.rtsp_name,RTSP_NAME_MAX_LEN);
	rtsp_autn_info->rtsp_permissions = g_rtst_info.rtsp_autn_info.rtsp_permissions;

	return RTSP_RET_SUCESS;
}




static int app_stream_output_rtsp(char *pkg_buf,int pkg_len,void *info)
{
	;
}

int app_stream_video_output(unsigned char *buff, int len, App_media_info_t *info)
{
	if(g_rtst_info.rtsp_module_falg != RTSP_MODULE_ON)
	{
		return RTSP_RET_FAILE;
	}

	if(len / (1024) > 1024)
	{
		printf("len= %d\n", len / (1024));
	}
	
	int width = info->width;
	int height = info->height;
	unsigned int timestamp = info->timestamp;
	unsigned int rtp_time = info->rtp_time;
	int IsIframe = info->IsIframe;
	int mtu = 0;
	int rtsp_mtu = 0;
	int channel = 0;

	APP_MEDIA_INFO media_info ;
	memset(&media_info, 0, sizeof(APP_MEDIA_INFO));
	media_info.width = width;
	media_info.height = height;
	media_info.stream_channel = info->stream_channel;
	channel = media_info.stream_channel;


	/*if it is iframe,i will update the video sdp info*/
	if(IsIframe == 1) {
		rtsp_porting_video_filter_sdp_info(buff, len, width, height, channel);
	}

	if(mult_rtp_get_active(channel) == 1) {
		mtu = mult_get_direct_rtp_mtu(channel);
		rtsp_mtu = stream_get_rtsp_mtu();

		if(rtsp_mtu < mtu) {
			mtu = rtsp_mtu;
		}
		g_rtst_info.rtp_module_info->rtp_build_video_data(len, buff, IsIframe, mtu, rtp_time, &media_info);
	} 
	else
	{
		//	rtp_build_reset_time();
	}
	
}
int app_stream_audio_output(unsigned char *buff, int len, App_media_info_t *info)
{
	if(g_rtst_info.rtsp_module_falg != RTSP_MODULE_ON)
	{
		return RTSP_RET_FAILE;
	}

	int recreate = info->recreate;
	int samplerate = info->samplerate ;
	int channel = info->channel;
	unsigned int timestamp = info->timestamp;
	unsigned int rtp_time = info->rtp_time;
	int mtu = 0;
	int rtsp_mtu = 0;

	int achannel = 0;
	APP_MEDIA_INFO media_info ;
	memset(&media_info, 0, sizeof(APP_MEDIA_INFO));
	media_info.sample = info->samplerate;

	media_info.stream_channel = info->stream_channel;
	achannel = info->stream_channel;

	if(len > 1300) {
		printf("Warnning,the len = %d\n", len);
	}

	if(recreate == 1) {
		if(rtsp_porting_audio_filter_sdp_info(buff, channel, samplerate, achannel) < 0)
		{
			return -1;
		}
	}

	if(mult_rtp_get_active(achannel) == 1) {
		mtu = mult_get_direct_rtp_mtu(achannel);
		rtsp_mtu = stream_get_rtsp_mtu();

		if(rtsp_mtu < mtu) {
			mtu = rtsp_mtu;
		}
		g_rtst_info.rtp_module_info->rtp_build_audio_data(len, buff, timestamp, mtu, rtp_time, &media_info);

	}
	return 0;
}

int app_stream_media_output(unsigned char *buff, int len, App_media_info_t *info)
{
	int ret = RTSP_RET_FAILE;
	if(g_rtst_info.rtsp_module_falg != RTSP_MODULE_ON)
	{
		return RTSP_RET_FAILE;
	}
	if(info->media_type == RTSP_MEDIA_AUDIO)
	{
		if(app_stream_audio_output(buff,len,info) != RTSP_RET_FAILE)
		{
			ret = RTSP_RET_SUCESS;
		}
	}
	else if(info->media_type == RTSP_MEDIA_VIDEO)
	{
		if(app_stream_video_output(buff,len,info) != RTSP_RET_FAILE)
		{
			ret = RTSP_RET_SUCESS;
		}
	}
	else
	{
		printf("error!\n");
	}
	return ret;
}

//the internal interface in the RTSP_RTP_SERV module

static void *RtspTask(void *args)
{
	mid_plat_sockaddr_in		ServAddr;
	mid_plat_sockaddr_in		ClitAddr;
	int							Serv_fd;
	int							Clit_fd;
	Rtsp_net_info_t				Rtsp_net_info;
CONTINUE:
			
}


static int init_rtsp_module(char *rtsp_addr,int rtsp_port,int str_num)
{
 	int ret = RTSP_RET_FAILE;
	memset(&g_rtst_info , 0, sizeof(Rtsp_module_info_t));

	if(rtsp_port == 0)
	{
		g_rtst_info.rtsp_serv_port 	= RTSP_SERV_PORT;
	}

	g_rtst_info.rtsp_module_falg 	= RTSP_MODULE_ON;
	g_rtst_info.rtsp_client_num 	= RTSP_CLIT_NUM;
	g_rtst_info.rtsp_serv_port 	= rtsp_port;
	
	memcpy(g_rtst_info.rtsp_serv_addr ,rtsp_addr,RTSP_ADDR_MAX_LEN);
		
	g_rtst_info.rtsp_autn_info.rtsp_permissions = RTSP_PER_OFF;

	strcpy(g_rtst_info.rtsp_autn_info.rtsp_name,RTSP_NAME_DEFAULT);
	strcpy(g_rtst_info.rtsp_autn_info.rtsp_pass,RTSP_PASS_DEFAULT);
	strcpy(g_rtst_info.rtsp_autn_info.rtsp_realm,RTSP_REALM_DEFAULT);	

	g_rtst_info.rtsp_pthread_handle = 0;

	ret = pthread_create(&(g_rtst_info.rtsp_pthread_handle), NULL, (void *)RtspTask, NULL);
	if(ret != RTSP_RET_SUCESS)
	{
		fprintf(stderr, "start_task pthread_create failed, ret = %d, errmsg = %s\n", ret, strerror(errno));
		return RTSP_RET_FAILE;
	}

	// init rtsp sdp info




	// register rtp module
	g_rtst_info.rtp_module_info = Register_Rtp_module(0,0,str_num,0);
	if(g_rtst_info.rtp_module_info == NULL)
	{
		return RTSP_RET_FAILE;
	}
	// set app_stream_output_rtp 
	if(g_rtst_info.rtp_module_info->set_app_stream_out_rtp(app_stream_output_rtsp) == RTP_RET_FAILE)
	{	
		return RTSP_RET_FAILE;
	}
		
	return RTSP_RET_SUCESS;
}


Rtsp_module_handle_t *Register_Rtsp_module(char *rtsp_serv_addr,int rtsp_serv_port ,int rtsp_str_num)
{
	Rtsp_module_handle_t *rtsp_handle = NULL;
		
	if(g_rtst_info.rtsp_module_falg == RTSP_MODULE_ON)
	{
		return NULL;
	}
	if(rtsp_serv_addr == NULL || rtsp_serv_port < 0)
	{
		return NULL;
	}
	rtsp_handle = malloc(sizeof(Rtsp_module_handle_t));
	
	if(rtsp_handle == NULL)
	{
		return NULL;
	}

	rtsp_handle->Set_ClitNum_Rtsp = Set_ClitNum_Rtsp;
	rtsp_handle->Set_per_Rtsp = Set_per_Rtsp;
	rtsp_handle->Get_ClitNum_Rtsp = Get_ClitNum_Rtsp;
	rtsp_handle->Get_per_Rtsp = Get_per_Rtsp;
	rtsp_handle->Get_Rtsp_module_flag = Get_Rtsp_module_flag;

	if(init_rtsp_module(rtsp_serv_addr,rtsp_serv_port,rtsp_str_num) != RTSP_RET_SUCESS)
	{
		free(rtsp_handle);
		rtsp_handle = NULL;
	}
	
	return rtsp_handle;
	
}




