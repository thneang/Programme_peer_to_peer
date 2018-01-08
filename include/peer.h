#ifndef			PEER_H_
	#define		PEER_H_

	#include <ftw.h>
	#include <stdio.h>
	#include <fcntl.h>
	#include <netdb.h>
	#include <signal.h>
	#include <unistd.h>
	#include <stdlib.h>
	#include <string.h>
	#include <pthread.h>
	#include <sys/stat.h>
	#include <sys/wait.h>
	#include <sys/types.h>
	#include <arpa/inet.h>
	#include <sys/socket.h>
	#include <netinet/in.h>

	#include <map>
	#include <vector>
	#include <fstream>
	#include <iostream>

	#include <packet.h>

	#define	TRACKER_PORT	8080

	typedef struct		_unit_
	{
		long int	mtime;  // last edition time (used only for files)
 		int		exist;	// 1 = exist, 0 = removed
		char		state;	// A = added, M = modified (used only for files)
	}			unit;

	/****************************************************************************************/
	/* ==================================================================================== */
	/* =	SUMMARY :								      = */
	/* ==================================================================================== */
	/* =		I- 	ACTIONS							      = */
	/* =		II- 	REPIES							      = */
	/* =		III- 	REQUESTS						      = */
	/* =		IV- 	LISTENER						      = */
	/* =		V- 	SYNCHRONISATION						      = */	
	/* ==================================================================================== */
	/****************************************************************************************/
	

	/****************************************************************************************/
	/*	I-	ACTIONS									*/
	/****************************************************************************************/
	/****************************************************************************************/
	/*											*/	
	/*	NAME	: 	spawn(char* program, char** arg_list);				*/
	/*	PARAMS 	: 	- char* program = the program (like ls) which will be called by */
	/*			execvp right after fork().					*/
	/*			- char** arg_list = the list of the args to pass for the 	*/
	/*			program.							*/
	/*	RETURN	: 	int = result of the fork.					*/
	/*	INFO 	:	used to invoque mkdir or rm inside the code.			*/
	/*											*/
	/****************************************************************************************/
	int	spawn(char* program, char** arg_list);
	/****************************************************************************************/	
	/*											*/
	/*	NAME	:	add_content							*/
	/*	PARAMS 	:	- packet* pkt = the packet sended into network wich contain all	*/
	/*			request or replies from peers or tracker.			*/
	/*	RETURN	:	void								*/
	/*	INFO 	:	open the file (the filename is stored into pkt)	and write into	*/
	/*			it the content also stored into pkt (payload).			*/
	/*											*/
	/****************************************************************************************/
	void 	add_content(packet* pkt);
	/****************************************************************************************/	
	/*											*/
	/*	NAME	:	create_file							*/
	/*	PARAMS 	:	- packet* pkt =	the packet sended into network wich contain all	*/
	/*			request or replies from peers or tracker.			*/
	/*	RETURN	:	void								*/
	/*	INFO  	:	open a new file (name stored into pkt) for creation.		*/
	/*											*/
	/****************************************************************************************/
	void	create_file(packet* pkt);
	/****************************************************************************************/	
	/*											*/
	/*	NAME	:	remove_file							*/
	/*	PARAMS 	:	- char* filename = name of the file to be removed.		*/
	/*	RETURN	:	void								*/
	/*	INFO  	:	try to remove a file.						*/
	/*											*/
	/****************************************************************************************/
	void	remove_file(char* filename);
	/****************************************************************************************/	
	/*											*/
	/*	NAME	:	create_dir							*/
	/*	PARAMS 	:	- char* dirname = name of the directory to be created.		*/
	/*	RETURN	:	void								*/
	/*	INFO  	:	try to create a new directory.					*/
	/*											*/
	/****************************************************************************************/
	void	create_dir(char* dirname);
	/****************************************************************************************/	
	/*											*/
	/*	NAME	:	remove_dir							*/
	/*	PARAMS 	:	- char* dirname = name of the directory to be removed.		*/
	/*	RETURN	:	void								*/
	/*	INFO  	:	try to remove a directory (program = rm, arguments = -rf in 	*/
	/*			case if the directory is not empty we force its removal but it	*/
	/*			may cause errors when trying to remove something which was 	*/
	/*			stored inside.							*/
	/*											*/
	/****************************************************************************************/
	void	remove_dir(char* dirname);

	/****************************************************************************************/
	/*	II-	REPLIES									*/
	/****************************************************************************************/
	/****************************************************************************************/	
	/*											*/
	/*	NAME	:	create_peer_reply						*/
	/*	PARAMS 	:	- packet *pkt = the packet sended into network wich contain all	*/
	/*			request or replies from peers or tracker.			*/
	/*	RETURN	:	void								*/
	/*	INFO  	:	reply from the tracker if our peer's ip and port have been	*/
	/*			successfuly added to its database.				*/
	/*											*/
	/****************************************************************************************/
	void	create_peer_reply(packet *pkt);
	/****************************************************************************************/	
	/*											*/
	/*	NAME	:	leave_peer_reply						*/
	/*	PARAMS 	:	- packet *pkt = the packet sended into network wich contain all	*/
	/*			request or replies from peers or tracker.			*/
	/*	RETURN	:	void								*/
	/*	INFO  	:	reply from the tracket if our peer have been successfuly	*/
	/*			deleted from its database.					*/
	/*											*/
	/****************************************************************************************/
	void	leave_peer_reply(packet *pkt);
	/****************************************************************************************/	
	/*											*/
	/*	NAME	:	peers_connection_update						*/
	/*	PARAMS 	:	- packet *pkt = the packet sended into network wich contain all	*/
	/*			request or replies from peers or tracker.			*/
	/*	RETURN	:	void								*/
	/*	INFO  	:	reply from tracker to all peers to update their list of 	*/
	/*			all availables peers in network.				*/
	/*											*/
	/****************************************************************************************/
	void	peers_connection_updates(packet *pkt);
	/****************************************************************************************/	
	/*											*/
	/*	NAME	:	file_create_reply						*/
	/*	PARAMS 	:	- struct sockaddr_in* sender_addr = address of the sender (ip	*/
	/*			and port).							*/
	/*			- packet *pkt = packet filled by the sender.			*/
	/*	RETURN	:	void								*/
	/*	INFO  	:	one of the peers connected has sent this reply to ask for	*/
	/*			creating a new file (name stored into pkt).			*/
	/*											*/
	/****************************************************************************************/
	void	file_create_reply(struct sockaddr_in *sender_addr, packet *pkt);
	/****************************************************************************************/	
	/*											*/
	/*	NAME	:	file_content_reply						*/
	/*	PARAMS 	:	- struct sockaddr_in* sender_addr = address of the sender (ip	*/
	/*			and port).							*/
	/*			- packet *pkt = packet filled by the sender.			*/
	/*	RETURN	:	void								*/
	/*	INFO  	:	one of the peers into network has created a file after our 	*/
	/*			request to create one and now it ask for us to send the 	*/
	/*			content of that file.						*/
	/*											*/
	/****************************************************************************************/
	void	file_content_reply(struct sockaddr_in *sender_addr, packet *pkt);
	/****************************************************************************************/	
	/*											*/
	/*	NAME	:	file_remove_reply						*/
	/*	PARAMS 	:	- struct sockaddr_in* sender_addr = address of the sender (ip	*/
	/*			and port).							*/
	/*			- packet *pkt = packet filled by the sender.			*/
	/*	RETURN	:	void								*/
	/*	INFO 	:	one of the peers into network asked for removing a file.	*/
	/*											*/
	/****************************************************************************************/
	void	file_remove_reply(struct sockaddr_in *sender_addr, packet *pkt);
	/****************************************************************************************/	
	/*											*/
	/*	NAME	:	file_update_reply						*/
	/*	PARAMS 	:	- struct sockaddr_in* sender_addr = address of the sender (ip	*/
	/*			and port).							*/
	/*			- packet *pkt = packet filled by the sender.			*/
	/*	RETURN	:	void								*/
	/*	INFO 	:	one of the peers into network asked for updating one file (name	*/
	/*			stored into pkt). If we don't have this file, we create it, or 	*/
	/*			if we have we recreate it and and it's new content.		*/
	/*											*/
	/****************************************************************************************/
	void	file_update_reply(struct sockaddr_in *sender_addr, packet *pkt);
	/****************************************************************************************/	
	/*											*/
	/*	NAME	:	dir_create_reply						*/
	/*	PARAMS 	:	- struct sockaddr_in* sender_addr = address of the sender (ip	*/
	/*			and port).							*/
	/*			- packet *pkt = packet filled by the sender.			*/
	/*	RETURN	:	void								*/
	/*	INFO 	:	one of the peers into network asked to create a new directory.	*/
	/*											*/
	/****************************************************************************************/
	void	dir_create_reply(struct sockaddr_in *sender_addr, packet *pkt);
	/****************************************************************************************/	
	/*											*/
	/*	NAME	:	dir_remove_reply						*/
	/*	PARAMS 	:	- struct sockaddr_in* sender_addr = address of the sender (ip	*/
	/*			and port).							*/
	/*			- packet *pkt = packet filled by the sender.			*/
	/*	RETURN	:	void								*/
	/*	INFO	:	one of the peers asked for removing a directory.		*/
	/*											*/
	/****************************************************************************************/
	void	dir_remove_reply(struct sockaddr_in *sender_addr, packet *pkt);

	/****************************************************************************************/
	/*	III-	REQUESTS								*/
	/****************************************************************************************/	
	/****************************************************************************************/	
	/*											*/
	/*	NAME	:	file_content_request						*/
	/*	PARAMS 	:	- struct sockaddr_in* sender_addr = address of the sender (ip	*/
	/*			and port).							*/
	/*			- packet *pkt = packet filled by the sender.			*/
	/*	RETURN	:	void								*/
	/*	INFO 	:	after creating a new file we ask for the sender the content	*/
	/*			of the new file created.					*/
	/*											*/
	/****************************************************************************************/
	void	file_content_request(struct sockaddr_in *sender_addr, const char* filename);
	/****************************************************************************************/	
	/*											*/
	/*	NAME	:	create_peer_request						*/
	/*	PARAMS 	:	void								*/
	/*	RETURN	:	void								*/
	/*	INFO 	:	when we start the program we want to be recognized by the 	*/
	/*			network so we ask for the tracker to store our ip address and	*/
	/*			port and to resend a new list of all available peers.		*/
	/*											*/
	/****************************************************************************************/
	void	create_peer_request(void);
	/****************************************************************************************/	
	/*											*/
	/*	NAME	:	leave_peer_request						*/
	/*	PARAMS 	:	void								*/
	/*	RETURN	:	void								*/
	/*	INFO 	:	before exiting the program we have to tell the tracker that we	*/
	/*			no longer catch replies or request form other peers.		*/
	/*											*/
	/****************************************************************************************/
	void	leave_peer_request(void);
	/****************************************************************************************/	
	/*											*/
	/*	NAME	:	file_create_request						*/
	/*	PARAMS 	:	const char* filename = name of the file to be created by all	*/
	/*			other peers into network.					*/
	/*	RETURN	:	void								*/
	/*	INFO 	:	our program has detected a new file so we ask for all peers	*/
	/*			to create it too.						*/
	/*											*/
	/****************************************************************************************/
	void	file_create_request(const char* filename);
	/****************************************************************************************/	
	/*											*/
	/*	NAME	:	file_send_content_request					*/
	/*	PARAMS 	:	- struct sockaddr_in* sender_addr = address of the sender (ip	*/
	/*			and port).							*/
	/*			- packet *pkt = packet filled by the sender.			*/
	/*	RETURN	:	void								*/
	/*	INFO 	:	after creating the new file (request from sender_addr) we ask	*/
	/*			sender_addr for its content.					*/
	/*											*/
	/****************************************************************************************/
	void	file_send_content_request(struct sockaddr_in *sender_addr, const char* filename);
	/****************************************************************************************/	
	/*											*/
	/*	NAME	:	file_remove_request						*/
	/*	PARAMS 	:	const char* filename = name of the file to be removed by all	*/
	/*			peers.								*/
	/*	RETURN	:	void								*/
	/*	INFO 	:	our program has detected that a file has been deleted so we 	*/
	/*			ask for its removal by all peers.				*/
	/*											*/
	/****************************************************************************************/
	void	file_remove_request(const char*	filename);
	/****************************************************************************************/	
	/*											*/
	/*	NAME	:	file_update_request						*/
	/*	PARAMS 	:	const char* filename = name of the file to be updated by all	*/
	/*			peers.								*/
	/*	RETURN	:	void								*/
	/*	INFO 	:	our program has detected that a file has been modified so we	*/
	/*			ask for its update by all peers.				*/
	/*											*/
	/****************************************************************************************/
	void 	file_update_request(const char* filename);
	/****************************************************************************************/	
	/*											*/
	/*	NAME	:	dir_create_request						*/
	/*	PARAMS 	:	const char* dirname = name of the directory to be created by	*/
	/*			all peers.							*/
	/*	RETURN	:	void								*/
	/*	INFO 	:	our program has detected that a new directory has been created	*/
	/*			so we has for all peers to create it too.			*/
	/*											*/
	/****************************************************************************************/
	void	dir_create_request(const char* dirname);
	/****************************************************************************************/	
	/*											*/
	/*	NAME	:	dir_remove_request						*/
	/*	PARAMS 	:	const char* dirname = name of the directory to be removed by	*/
	/*			all peers.							*/
	/*	RETURN	:	void								*/
	/*	INFO 	:	our program has detected that a directory has been deleted so	*/
	/*			we ask for all peers to remove it too.				*/
	/*											*/
	/****************************************************************************************/
	void	dir_remove_request(const char* dirname);

	/****************************************************************************************/
	/*	IV-	LISTENER								*/
	/****************************************************************************************/
	/****************************************************************************************/	
	/*											*/
	/*	NAME	:	receive_packer							*/
	/*	PARAMS 	:	void								*/
	/*	RETURN	:	void								*/
	/*	INFO 	:	this function is used to listend the network and catch every	*/
	/*			packet from it, then it extract its type and do the apropriate	*/
	/*			action.								*/
	/*											*/
	/****************************************************************************************/
	void	receive_packet(void);

	/****************************************************************************************/
	/*	V-	SYNCHRONISATION								*/
	/****************************************************************************************/
	/****************************************************************************************/	
	/*											*/
	/*	NAME	:	mark_file_deleted						*/
	/*	PARAMS 	:	void								*/
	/*	RETURN	:	void								*/
	/*	INFO 	:	mark all files stored in map_file deleted.			*/
	/*											*/
	/****************************************************************************************/
	void	mark_file_deleted(void);
	/****************************************************************************************/	
	/*											*/
	/*	NAME	:	mark_dir_deleted						*/
	/*	PARAMS 	:	void								*/
	/*	RETURN	:	void								*/
	/*	INFO 	:	mark all directories stored in map_dir deleted.			*/
	/*											*/
	/****************************************************************************************/
	void	mark_dir_deleted(void);
	/****************************************************************************************/	
	/*											*/
	/*	NAME	:	list								*/
	/*	PARAMS 	:	- const char *name = name of the directory / file found.	*/
	/*			- const struct stat* status = info about the inode of the file	*/
	/*			or directory found.						*/
	/*			int type = nothing to say about it.				*/
	/*	RETURN	:	int : 0 success, 1 failed.					*/
	/*	INFO 	:	this function is used by ftw to filter every object found on	*/
	/*			the repertory where our program started.			*/
	/*											*/
	/****************************************************************************************/
	int	list(const char *name, const struct stat *status, int type);
	/****************************************************************************************/	
	/*											*/
	/*	NAME	:	update_file							*/
	/*	PARAMS 	:	void								*/
	/*	RETURN	:	void								*/
	/*	INFO 	:	if a file has been deleted / created (exit = 0 or 1 and		*/
	/*			state = A or M in structure unit)				*/
	/*			we send a request for all peers to remove / create / update it 	*/
	/*			too.								*/
	/*											*/
	/****************************************************************************************/
	void	update_file(void);
	/****************************************************************************************/	
	/*											*/
	/*	NAME	:	update_dir							*/
	/*	PARAMS 	:	void								*/
	/*	RETURN	:	void								*/
	/*	INFO 	:	if a directory has been deleted / created (exit = 0 or 1 in 	*/
	/*			structure unit)							*/
	/*			we send a request for all peers to remove / create it too.	*/
	/*											*/
	/****************************************************************************************/
	void	update_dir(void);
	/****************************************************************************************/	
	/*											*/
	/*	NAME	:	synchronisation							*/
	/*	PARAMS 	:	NULL								*/
	/*	RETURN	:	NULL								*/
	/*	INFO 	:	ask every 4 seconds to syncronize our repertory with the other	*/
	/*			peers.								*/
	/*											*/
	/****************************************************************************************/
	void*	synchronisation(void* argument);
	/****************************************************************************************/	
	/*											*/
	/*	NAME	:	signal_handler							*/
	/*	PARAMS 	:	int signal = signal catched, we just want handle SIGINT.	*/
	/*	RETURN	:	void								*/
	/*	INFO 	:	when the user press CTRL-C we disconnect the peer from network	*/
	/*			before closing the application.					*/
	/*											*/
	/****************************************************************************************/
	void	signal_handler(int signal);
	

#endif		/* PEER_H_ */
