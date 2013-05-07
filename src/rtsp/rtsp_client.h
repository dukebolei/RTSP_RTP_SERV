#ifndef _RTSP_CLIENT_H__
#define _RTSP_CLIENT_H__

#include "Mid_linux_platform.h"

enum 
{
	INVALID = 0,
	STOP,
	PLAY
	
};


typedef struct Clit_info
{
	
	int 					clit_id;						// client user id
	mid_plat_socket 		general_clit_fd;				// msg fd or data fd
	mid_plat_socket 		data_clit_fd;					// data fd
	mid_plat_sockaddr_in 	general_clit_fd;				// client addr info

	int 					clit_type;						// the type of client 		( vlc /qt /...)
	int 					data_treaty_type;				// Data transfer mode	(tcp + RTP / udp + RTP)
	int						data_source_index;			// Data source type   		( index : 1/ 2/ 3 ... )

	int						clit_state;					// State of the client	 	( invalid / play / stop)

	int 					
}Clit_info_t;





#endif