#include <tracker.h>


std::list<peer>			peers;		//my list of peers
int				sock;		//my server socket
pthread_mutex_t 		stdout_lock;
pthread_mutex_t 		peers_lock;

struct sockaddr_in		get_sockaddr_in(unsigned int ip, short port)
{
  	struct sockaddr_in 	addr;
  
	addr.sin_family = AF_INET;
  	addr.sin_addr.s_addr = ip;
  	addr.sin_port = htons(port);
  	return(addr);
}

/****************************************************************************************/
/*	REPLY			<action>				<for>		*/
/****************************************************************************************/
/*	-peer_create_reply	send to peer that it has been		PEER		*/
/*				created.						*/
/*	-peer_leave_reply	send to peer that it has been		PEER		*/
/*				deleted.						*/
/*	-updtate_all_peerslist	send to all peers the list of		PEERS		*/
/*				avaliables peers.					*/
/****************************************************************************************/

void	peer_create_reply(unsigned int ip, short port)
{
    	packet 			pkt;
	struct sockaddr_in	peer_addr;
	int			status;

    	pkt.header.type = 'c';
    	peer_addr = get_sockaddr_in(ip, port);
    	status = sendto(sock, &pkt, sizeof(pkt.header), 0, (struct sockaddr *)&peer_addr, sizeof(peer_addr));
    	if(-1 == status)
	{
    		pthread_mutex_lock(&stdout_lock);
    			fprintf(stderr, "error - %s\n", "when sending packet to peer.");
    		pthread_mutex_unlock(&stdout_lock);
    	}
}

void	peer_leave_reply(unsigned int ip, short port)
{
	packet			pkt;
	struct sockaddr_in	peer_addr;
	int			status;

	pkt.header.type = 'l';
    	peer_addr = get_sockaddr_in(ip, port);
    	status = sendto(sock, &pkt, sizeof(pkt.header), 0, (struct sockaddr *)&peer_addr, sizeof(peer_addr));
    	if(-1 == status)
	{
    		pthread_mutex_lock(&stdout_lock);
    			fprintf(stderr, "error - %s\n", "when sending packet to peer.");
    		pthread_mutex_unlock(&stdout_lock);
    	}
}

void	update_all_peerslist(unsigned int ip, short port, char state)
{
	int 				i = 0;
	std::list<peer>::iterator	it;
	struct sockaddr_in 		list[peers.size()];
    	packet 				update_pkt;
	int 				status;

	for(it = peers.begin(); it != peers.end(); ++it)
	{
		struct sockaddr_in peer_info = get_sockaddr_in((*it).ip, (*it).port);
		struct sockaddr_in* peer_info_ptr = &peer_info;
		memcpy((sockaddr_in*)&list[i], peer_info_ptr, sizeof(peer_info));
		i++;
	}
  	update_pkt.header.type = 'u';
	update_pkt.header.payload_length = peers.size() * sizeof(struct sockaddr_in);
	memcpy(update_pkt.payload, list, peers.size() * sizeof(struct sockaddr_in));
	for(it = peers.begin(); it != peers.end(); ++it)
	{
		struct sockaddr_in peer_addr = get_sockaddr_in((*it).ip, (*it).port);
    		status = sendto(sock, &update_pkt, sizeof(update_pkt), 0, (struct sockaddr *)&peer_addr, sizeof(peer_addr));
		if(status == -1)
		{
			pthread_mutex_lock(&stdout_lock);
				fprintf(stderr, "error - %s\n", "when sending | update peerlist request | packet to peer.");
			pthread_mutex_unlock(&stdout_lock);
		}
	}
}

/****************************************************************************************/
/*	REQUESTS		<action>				<from>		*/
/****************************************************************************************/
/*	-peer_leave_request	delete a peer from database.		PEERS		*/
/*	-peer_create_request	create a peer and store it into		PEERS		*/
/*				database.						*/
/****************************************************************************************/

void	peer_leave_request(unsigned int ip, short port)
{
	std::list<peer>::iterator	it;
	bool				erased = false;

	for(it = peers.begin(); it != peers.end(); ++it)
	{
		if(((*it).port == port) && ((*it).ip == ip))
		{
			peers.erase(it);
			erased = true;
			break;
		}
	}
	if(erased)
	{
    		pthread_mutex_lock(&stdout_lock);
    			fprintf(stdout, "info - peer %d %d %s.\n", ip, port, "deleted");
		pthread_mutex_unlock(&stdout_lock);
		peer_leave_reply(ip, port);		
		update_all_peerslist(ip, port, 'u');
	}
	else
	{
    		pthread_mutex_lock(&stdout_lock);
      			fprintf(stderr, "error - %s\n", "peer not stored in list.");
      		pthread_mutex_unlock(&stdout_lock);
	}
}

void	peer_create_request(unsigned int ip, short port)
{
	peer				new_peer;
	bool				exist = false;
	std::list<peer>::iterator	it;

	for(it = peers.begin(); it != peers.end(); ++it)
	{
		if(((*it).port == port) && ((*it).ip == ip))
		{
			exist = true;
			break;
		}
	}
	if(!exist)
	{
		//create peer
		new_peer.ip = ip;
		new_peer.port = port;
		new_peer.status = 1;
		//add new peer in list
  		pthread_mutex_lock(&peers_lock);
			peers.push_back(new_peer);
    		pthread_mutex_unlock(&peers_lock);
    		pthread_mutex_lock(&stdout_lock);
    			fprintf(stdout, "info - peer %d %d %s.\n", new_peer.ip, new_peer.port, "created");
		pthread_mutex_unlock(&stdout_lock);
		peer_create_reply(ip, port);
		update_all_peerslist(ip, port, 'u');
	}
	else
	{
    		pthread_mutex_lock(&stdout_lock);
      			fprintf(stderr, "error - %s\n", "peer already stored in list.");
      		pthread_mutex_unlock(&stdout_lock);
	}
}

void	signhandler(int signal)
{
	std::cout << "Tracker shuting down." << std::endl;
	exit(0);
}

int	main(int ac, char** av)
{
  	struct sockaddr_in	self_addr;
	
	fprintf(stdout, "info - Starting server on ports --- %d\n", PORT);
  	//setup UDP sockets
  	sock = socket(AF_INET, SOCK_DGRAM, 0);
  	if(sock < 0)
	{
    		fprintf(stderr, "error - %s\n", "when creating sock.");
    		return(EXIT_FAILURE);
 	}
  	self_addr.sin_family = AF_INET; 
  	self_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  	self_addr.sin_port = htons(PORT);
  	if(bind(sock, (struct sockaddr *)&self_addr, sizeof(self_addr)))
	{
    		fprintf(stderr, "error - %s\n", "when binding sock.");
    		return(EXIT_FAILURE);
  	}
	signal(SIGINT, &signhandler);
	//we capture all packets
	socklen_t		addrlen = 10;
  	struct sockaddr_in	sender_addr;
  	packet 			recv_pkt;
  	int 			recv_status;
  	
	while(1)
	{
    		recv_status = recvfrom(sock, &recv_pkt, sizeof(recv_pkt), 0, (struct sockaddr *)&sender_addr, &addrlen);
    		if(-1 == recv_status)
		{
      			pthread_mutex_lock(&stdout_lock);
      				fprintf(stderr, "error - %s\n", "when receiving a packet, ignoring.");
      			pthread_mutex_unlock(&stdout_lock);
    		}
		else
		{
      			unsigned int	ip = sender_addr.sin_addr.s_addr;
      			short 		port = htons(sender_addr.sin_port);
      			switch(recv_pkt.header.type)
			{
        			case 'c': 
					peer_create_request(ip, port);
          				break;
        			case 'l':
          				peer_leave_request(ip, port);
          				break;
        			default:
          				pthread_mutex_lock(&stdout_lock);
          				fprintf(stderr, "error - %s\n", "received packet type unknown, ignoring.");
          				pthread_mutex_unlock(&stdout_lock);
          				break;
      			}
    		}
  	}
	return(EXIT_SUCCESS);
}
