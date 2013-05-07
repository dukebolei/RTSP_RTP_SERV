#ifndef _RTP_BUILD_H__
#define _RTP_BUILD_H__
/*
**************************************************************************************
函数介绍:
1.void RtpInit()
    功能介绍:
        该函数主要完成RTP 各项参数初始化
            a.初始化锁
            b.分配音频打包所需的内存空间
2.void RtpSetUdp(int UdpVideoSocket, struct sockaddr_in UdpVideoAddr,int UdpAudioSocket, struct sockaddr_in UdpAudioAddr)
    功能介绍:
        该函数设置RTP组播socket和地址，重建时间戳 
            a.设置视频组播的socket和地址
            b.设置音频组播的socket和地址
            c.重建时间戳
    参数说明:
        UdpVideoSocket:视频组播socket
        UdpVideoAddr:视频组播地址
        UdpAudioSocket:音频组播socket
        UdpAudioAddr:音频组播地址
3.RtpVideoPack(int nLen, unsigned char *pData, int nFlag, unsigned char index)
    功能介绍:
        该函数主要完成视频的rtp打包
    参数说明:
        nLen:要打包的264视频长度
        pData:要打包的264视频地址
        nFlag:帧标志，=1 I帧；=0 P帧
4.RtpAudioPack(int nLen, unsigned char * pData, int nFlag, unsigned char index)
    功能介绍:
        该函数主要完成音频的rtp打包
    参数说明:
        nLen:要打包的aac视频长度
        pData:要打包的aac视频地址
5.void RtpExit()
    功能介绍:
        退出 RTP 协议
            a.锁销毁
            b.释放分配的内存 
6.char *RtpGetVersion()
    功能介绍:
        获取当前rtp 版本号
**************************************************************************************
*/


#define	RTP_MODULE_ON					1
#define	RTP_MODULE_OFF					0

#define	RTP_RET_FAILE					-1;
#define	RTP_RET_SUCESS					0;	

#define RTP_PKG_MTU_DEFAULT				1350

typedef int (*app_stream_output_rtp)(char *,int,void *);

typedef struct Rtp_module_handle
{
 	int (*set_app_stream_out_rtp)(app_stream_output_rtp fp);		//set App_stream_out callback interface

	int (*get_stream_num_rtp)(int *str_num);						//get rtp stream max num
		
	int (*set_stream_mtu_rtp)(int str_mtu);							//set rtp stream max pkg size
	int (*get_stream_mtu_rtp)(int *str_mtu);						//get rtp stream max pkg size
	
	int (*reset_stream_time_rtp)(int str_id);  						//reset rtp stream time 
	
																	//audio data deal interface 
	int (*rtp_build_audio_data)(int str_id,int nLen, unsigned char *pData,int samplerate,unsigned int nowtime,void *info);
																	//video data deal interface
	int (*rtp_build_video_data)(int str_id,int nLen, unsigned char *pData, int nFlag,unsigned int nowtime,void *info);

	
}Rtp_module_handle_t;


Rtp_module_handle_t *Register_Rtp_module(unsigned int ssrc, unsigned int payload ,unsigned int str_num,unsigned int str_mtu);  
int Unregister_Rtp_module(Rtp_module_handle_t *rtp_module_handle);



#endif

