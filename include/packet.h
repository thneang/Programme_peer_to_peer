#ifndef			PACKET_H_
	#define		PACKET_H_

	typedef struct 		_message_header_ 
	{
		char		type;
		unsigned int	name_length;		
		unsigned int 	payload_length;
		char		name[128];
	} 			message_header;

	typedef struct 		_packet_ 
	{
		message_header	header;
		char 		payload[512];
	} 			packet;

#endif		/* PACKET_H_ */
