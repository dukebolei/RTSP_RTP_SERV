#ifndef _RTSP_SERVER_H__
#define _RTSP_SERVER_H__

#include <pthread.h>
#include "rtp_build.h"
#include "mid_linux_platfrom.h"
#include "app_stream_info.h"



// The default value max len in the RTSP_RTP_SERV module 

#define	RTSP_MEDIA_AUDIO				1
#define	RTSP_MEDIA_VIDEO				2

#define RTSP_REALM_MAX_LEN    		128
#define RTSP_NAME_MAX_LEN				128
#define RTSP_PASS_MAX_LEN 			128

#define	RTSP_URL_MAL_LEN				128
#define RTSP_MD5_MAX_LEN				33
#define	RTSP_ADDR_MAX_LEN				16

#define	RTSP_PER_ON					1
#define	RTSP_PER_OFF					0	

#define	RTSP_MODULE_ON					1
#define	RTSP_MODULE_OFF				0

#define	RTSP_RET_FAILE					-1;
#define	RTSP_RET_SUCESS				0;	

enum{
	OPTION=1,
	DESCRIBE,
	SETUP,
	PLAY,
	PAUSE,
	TEARDOWN,
	ANNOUNCE,
	GET_PARAMETER,
	SET_PARAMETER,
	MAX_OPERATE
};


// The default value in the RTSP_RTP_SERV module 
#define RTSP_SERV_PORT					554
#define	RTSP_CLIT_NUM					10

#define	RTSP_NAME_DEFAULT				"admin"
#define	RTSP_PASS_DEFAULT				"admin"
#define	RTSP_REALM_DEFAULT			"rstp_serv"

// The internal data structure type in the RTSP_RTP_SERV module

typedef struct Rtsp_AUTN
{
	char 	rtsp_name[RTSP_NAME_MAX_LEN];					// rtsp Certification authority name
	char 	rtsp_pass[RTSP_PASS_MAX_LEN];					// rtsp Certification authority password
	char 	rtsp_realm[RTSP_REALM_MAX_LEN];				// rtsp Certification authority realm

	int 	rtsp_permissions;								// rtsp Certification authority permissions	(on : 1 / off : 0)
	
}Rtsp_AUTN_t;

typedef struct Rtsp_AUTN_info
{
	char 	rtsp_nonce[RTSP_MD5_MAX_LEN];					// stream of (name + password + realm + mad5 +url ) by md5 
	char 	rtsp_url[RTSP_URL_MAL_LEN];					// url of the rtsp server 
	
}Rtsp_AUTN_Info_t;

typedef struct Rtsp_module_info
{
	int		rtsp_module_falg;								// rtsp module status falg (on : 1 / off : 0)
	
	int 	rtsp_serv_port;								// rtsp module server port (default : 554 )
	int 	rtsp_client_num;								// rtsp module client max num (default : 10)
	int		rtsp_stream_num;								// rtsp module stream max num 
	char 	rtsp_serv_addr[RTSP_ADDR_MAX_LEN];				// rtsp module server addr (Necessary parameters)					

	Rtsp_AUTN_t rtsp_autn_info;							// rtsp module Certification authority info
	pthread_t 	rtsp_pthread_handle;						// rtsp module thread handle
	
	

	Rtp_module_handle_t	*rtp_module_info;
		
}Rtsp_module_info_t;


// The external data structure type in the RTSP_RTP_SERV module
typedef struct Rtsp_clit_info
{	
	int 					clit_id;						// client user id
	mid_plat_socket 		general_clit_fd;				// msg fd or data fd
	mid_plat_socket 		data_clit_fd;					// data fd
	mid_plat_sockaddr_in 	general_clit_fd;				// client addr info

	int 					clit_type;						// the type of client 		( vlc /qt /...)

}Rtsp_clit_info_t;


typedef int (*add_user_Rtps)(Rtsp_clit_info_t *);
typedef int (*del_user_Rtps)(Rtsp_clit_info_t *);


typedef struct Rtsp_module_handle
{

	int (*Set_per_Rtsp)(Rtsp_AUTN_t *rtsp_autn_info);			// set Certification authority info
	int (*Get_per_Rtsp)(Rtsp_AUTN_t *rtsp_autn_info);			// get Certification authority info
	int (*Get_Rtsp_module_flag)(int *rtsp_module_flag);		// get rtsp module status flag

	int (*Set_ClitNum_Rtsp)(int rtsp_clit_num);				// set client max num (default : 10)
	int (*Get_ClitNum_Rtsp)(int *rtsp_clit_num);				// get client max num
	int (*Set_Clit_Treaty_Rtsp)(int rtsp_clit_treaty);		// set client deal data type (default : UDP+RTSP  (or TCP+RTSP))	 
	int (*Get_Clit_Treaty_Rtsp)(int *rtsp_clit_treaty);  		// get client deal data type 
	int (*Set_app_add_user_Rtsp)(add_user_Rtps fp);			// set Add a user callback interface
	int (*Set_app_del_user_Rtsp)(del_user_Rtps fp);			// set Del a user callback interface
	
	int (*Set_StreamNum_Rtsp)(int rtsp_str_num);				// set stream max num (default : 2) 
	int (*Get_StreamNum_Rtsp)(int *rtsp_str_num);				// get stream max num
	int (*Set_App_stream_out)(app_stream_output_rtp fp);		// set Del rtp stream callback interface
	
	int (*app_stream_media_output)(unsigned char *buff, int len, App_media_info_t *info);
	
//	int (*Set_add_user_Rtsp)(int (*Add_user_Rtps)(Rtsp_clit_info_t *));
//	int (*Set_del_user_Rtsp)(int (*Del_user_Rtsp)(Rtsp_clit_info_t *));

	
}Rtsp_module_handle_t;


//the interface in the RTSP_RTP_SERV module

// rtsp_serv_addr not be NULL       rtsp_serv_portNot less than zero (if rtsp_serv_port == 0 defalt is 554)
Rtsp_module_handle_t *Register_Rtsp_module(char *rtsp_serv_addr ,int rtsp_serv_port,int rtsp_str_num);  
void Unregister_Rtsp_module(Rtsp_module_handle_t * rtsp_module_handle);

#endif



