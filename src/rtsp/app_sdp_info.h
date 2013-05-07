#ifndef _APP_SDP_INFO_H__
#define _APP_SDP_INFO_H__


#define MAX_SPS_INFO_LEN 64
#define MAX_PPS_INFO_LEN MAX_SPS_INFO_LEN

#define H264_HEADER_LEN  0x40

typedef struct RTSP_VIDEO_SDP_INFO_T {

	unsigned int 		roomid;     			//区分第几路编码器 0/1/2/3
	unsigned int 		width;
	unsigned int 		heigh;

	unsigned int 		sdp_flag ;             //是否有sdp info
	unsigned char 		sps_info[MAX_SPS_INFO_LEN];
	unsigned int 		sps_info_len;
	unsigned char 		pps_info[MAX_SPS_INFO_LEN];
	unsigned int 		pps_info_len;
} RTSP_SDP_VINFO;

typedef struct audio_aac_adts_fixed_header_t {
	int syncword;
	unsigned char 	id;
	unsigned char 	layer;
	unsigned char 	profile;
	unsigned char 	sampling_frequency_index;
	unsigned char 	channel_configuration;
} audio_aac_adts_fixed_header_s;

typedef struct RTSP_AUDIO_SDP_INFO_T {

	unsigned int 	roomid;     	//区分第几路编码器 0/1/2/3
	unsigned int  	channel;  		//默认为2
	unsigned int  	samplerate; 	//音频采样率

	unsigned int  	audio_config ; // 是否有audio config
} RTSP_SDP_AINFO;


typedef struct Rtst_sdp_audio_info
{
	unsigned int stream_index;
	unsigned int 
	
		
}Rtst_sdp_audio_info_t;

typedef struct Rtst_sdp_video_info
{
	
	
}Rtst_sdp_video_info_t;

#endif

