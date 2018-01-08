#ifndef			TRACKER_H_
	#define		TRACKER_H_

	#include <math.h>
	#include <time.h>
	#include <errno.h>
	#include <fcntl.h>
	#include <netdb.h>
	#include <stdio.h>
	#include <signal.h>
	#include <string.h>
	#include <limits.h>
	#include <locale.h>
	#include <unistd.h>
	#include <stdlib.h>
	#include <pthread.h>
	#include <sys/stat.h>
	#include <sys/types.h>
	#include <netinet/in.h>
	#include <sys/socket.h>

	#include <list>
	#include <iostream>

	#include <packet.h>

	#define	MAX_PEERS	10
	#define	PORT		8080

	typedef struct		_peer_ 
	{
  		unsigned int	ip;		//store ip adress
		short		port;		//store port number
  		short 		status;		//1 = alive, 0 = dead
	}			peer;

	/****************************************************************************************/
	/* ====================================================================================	*/
	/* =	SUMMARY									      = */
	/* ==================================================================================== */
	/* =		I-	REPLIES							      = */
	/* =		II-	REQUESTS						      = */
	/* ====================================================================================	*/
	/****************************************************************************************/

	/****************************************************************************************/
	/*	I- REPLIES									*/
	/****************************************************************************************/
	/****************************************************************************************/
	/*											*/
	/*	NAME	:	peer_create_reply						*/
	/*	PARAMS	:	unsigned int ip = ip of the peer who asked to be added in list.	*/
	/*			short port = port of the peer who asked to be added in list.	*/ 
	/*	RETURN	:	void								*/
	/*	INFO	:	send a request to the peer wo asked to be added.		*/
	/*											*/
	/****************************************************************************************/
	void	peer_create_reply(unsigned int ip, short port);
	/****************************************************************************************/
	/*											*/
	/*	NAME	:	peer_leave_reply						*/
	/*	PARAMS	:	unsigned int ip = ip of the peer who asks to leave.		*/
	/*			short port = port of the peer who asks to leave.		*/
	/*	RETURN	:	void								*/
	/*	INFO	:	send a request to the peer wo asked to leave.			*/
	/*											*/
	/****************************************************************************************/
	void	peer_leave_reply(unsigned int ip, short port);
	/****************************************************************************************/
	/*											*/
	/*	NAME	:	update_all_peerslist						*/
	/*	PARAMS	:	unsigned int ip = ip of the peer added.				*/
	/*			short port = port of the peer added.				*/
	/*	RETURN	:	void								*/
	/*	INFO	:	add the new peer and send to all other the new list of peers	*/
	/*			updated.							*/
	/*											*/
	/****************************************************************************************/
	void	update_all_peerslist(unsigned int ip, short port, char state);
	
	/****************************************************************************************/
	/*	II- REQUESTS									*/
	/****************************************************************************************/
	/****************************************************************************************/
	/*											*/
	/*	NAME	:	peer_leave_request						*/
	/*	PARAMS	:	- unsigned int ip = ip address of the peer wo want to be	*/ 
	/*			removed from list.						*/
	/*			- short port = port of the peer wo want to be removed from 	*/
	/*			list.								*/
	/*	RETURN	:	void								*/
	/*	INFO	:	the tracker try to remove a peer from its list.			*/
	/*											*/
	/****************************************************************************************/
	void	peer_leave_request(unsigned int ip, short port);
	/****************************************************************************************/
	/*											*/
	/*	NAME	:	peer_create_request						*/
	/*	PARAMS	:	- unsigned int ip = ip address of the peer wo want to be added	*/
	/*			in list.							*/
	/*			- short port = port of the peer wo want to be added int list.	*/
	/*	RETURN	:	void								*/
	/*	INFO	:	the tracker try to add a new peer into its list of peers.	*/
	/*											*/
	/****************************************************************************************/
	void	peer_create_request(unsigned int ip, short port);

#endif		/* TRACKER_H_ */
