#ifndef _APP_STREAM_INFO_H__
#define _APP_STREAM_INFO_H__


typedef struct App_media_info
{
	int 			stream_index;
	int 			media_type;
	unsigned int 	timestamp;
	unsigned int 	rtp_time;
	int 			width;
	int 			height;
	int 			IsIframe;
	int 			recreate; 				/*ÐèÒªË¢ÐÂ  audio sdp info*/
	
	void 			*expand_space;
}App_media_info_t;

#endif
