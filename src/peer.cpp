#include <peer.h>

int 						sock;			// my peer sock
struct sockaddr_in 				tracker_addr;		// tracker addr ip
struct sockaddr_in 				self_addr;		// my addr ip
struct sockaddr_in 				peer_list[100];		// my list of peers
int 						peer_num = 0;		// number of peers connected
pthread_mutex_t 				stdout_lock;		// mutex -> protect stdout
pthread_mutex_t 				peer_list_lock;		// mutex -> protect peer_list
pthread_mutex_t					mutex;
pthread_mutex_t					map_lock;
short						my_port;		// my own port

std::map<std::string, unit>			file_map;		// map of my files
std::map<std::string, unit>			dir_map;		// map of my directories


/****************************************************************************************/
/*	ACTIONS			<action>						*/
/****************************************************************************************/
/*	-spawn			fork and execute a command.				*/
/*	-add_content		open a file and write into it.				*/
/*	-create_file		create a file.						*/
/*	-remove_file		delete a file.						*/
/*	-create_dir		create a directory.					*/
/*	-remove_dir		remove a directory.					*/
/****************************************************************************************/

int	spawn(char* program, char** arg_list)
{
	pid_t	child_pid;
	
  	switch(child_pid = fork())
	{
		case -1:
			pthread_mutex_lock(&stdout_lock);
				fprintf(stderr, "error - %s\n", "unable to fork.");
			pthread_mutex_unlock(&stdout_lock);
		case 0:
     			execvp(program, arg_list);
			pthread_mutex_lock(&stdout_lock);
     				fprintf(stderr, "error - %s\n", "execvp failed.");
     			pthread_mutex_unlock(&stdout_lock);
		default:
     			return(child_pid);
  	};
}

void 	add_content(packet* pkt)
{
	std::ofstream	file(pkt->header.name, std::ios::out | std::ios::app); 
	
	if(file)
	{
		pthread_mutex_lock(&stdout_lock);
			fprintf(stdout, "info - %s %d %s\n", "receving ---", pkt->header.payload_length, "octet(s).");
		pthread_mutex_unlock(&stdout_lock);
		file << pkt->payload;
		file.close();
	}
	else
	{
		pthread_mutex_lock(&stdout_lock);
			fprintf(stderr, "error - %s %s\n", "unable to create new file ", pkt->header.name);
		pthread_mutex_unlock(&stdout_lock);
	}
}

void	create_file(packet* pkt)
{
	std::ofstream	file(pkt->header.name, std::ios::out | std::ios::trunc); 
	
	if(file)	
	{
		file.close();
	}
	else
	{
		pthread_mutex_lock(&stdout_lock);
			fprintf(stderr, "error - %s %s.\n", "unable to create new file ", pkt->header.name);
		pthread_mutex_unlock(&stdout_lock);
	}	
}

void	remove_file(char* filename)
{
	int	child_status;
	char	program[] = "rm";
	char* 	arg_list[] = {program, filename, NULL};

	spawn(program, arg_list);
	wait(&child_status);
  	if(!WIFEXITED(child_status))
   	{
		pthread_mutex_lock(&stdout_lock);
			fprintf(stderr, "error - child process ended with failure.\n");
		pthread_mutex_unlock(&stdout_lock);
	}	
}

void	create_dir(char* dirname)
{
	int	child_status;
	char	program[] = "mkdir";
	char* 	arg_list[] = {program, dirname, NULL};	

	spawn(program, arg_list);
	wait(&child_status);
  	if(!WIFEXITED(child_status))
    	{
		pthread_mutex_lock(&stdout_lock);
			fprintf(stderr, "error - child process ended with failure.\n");
		pthread_mutex_unlock(&stdout_lock);
	}	
}

void	remove_dir(char* dirname)
{
	int	child_status;
	char	program[] = "rm";
	char	argument[] = "-rf";
	char* 	arg_list[] = {program, argument, dirname, NULL};	

	spawn(program, arg_list);
	wait(&child_status);
  	if(!WIFEXITED(child_status))
   	{
		pthread_mutex_lock(&stdout_lock);
			fprintf(stderr, "error - child process ended with failure\n");
		pthread_mutex_unlock(&stdout_lock);
	}
}

/****************************************************************************************/
/*	REPLY				<action>			<from>		*/
/****************************************************************************************/
/*	-create_peer_reply		the peer has been added		TRACKER		*/
/*					into tracker's database.			*/
/*	-leave_peer_reply		the peer has been deleted	TRACKER		*/
/*					from tracker's database.			*/
/*	-peers_connection_update	the tracker has send the	TRACKER		*/
/*					list of all available peers.			*/
/*	-file_create_reply		try to create a new file	PEERS		*/
/*	-file_content_reply								*/
/*	-file_remove_reply		try to remove a file		PEERS		*/
/*	-file_update_reply		try to update a file		PEERS		*/
/*	-dir_create_reply		try to create a directory	PEERS		*/
/*	-dir_remove_reply		try to remove a directory	PEERS		*/
/****************************************************************************************/

void	create_peer_reply(packet *pkt)
{
	pthread_mutex_lock(&stdout_lock);
		fprintf(stdout, "info - %s\n", "peer created.");
	pthread_mutex_unlock(&stdout_lock);
}

void	leave_peer_reply(packet *pkt)
{
	pthread_mutex_lock(&stdout_lock);
		fprintf(stdout, "info - %s\n", "peer disconected.");
	pthread_mutex_unlock(&stdout_lock);
}

void	peers_connection_updates(packet *pkt)
{
	int	new_peer_num;

	pthread_mutex_lock(&peer_list_lock);
		new_peer_num = pkt->header.payload_length / sizeof(struct sockaddr_in);
		if(new_peer_num <= 0) 
		{
			pthread_mutex_lock(&stdout_lock);
				fprintf(stderr, "error - %s\n", "peer list missing.");
			pthread_mutex_unlock(&stdout_lock);
		}
		else 
		{
			pthread_mutex_lock(&stdout_lock);
				fprintf(stdout, "info - %s\n", "list of peers updated.");
			pthread_mutex_unlock(&stdout_lock);
			peer_num = new_peer_num;
			memcpy(peer_list, pkt->payload, peer_num * sizeof(struct sockaddr_in));
		}
	pthread_mutex_unlock(&peer_list_lock);
}

void	file_create_reply(struct sockaddr_in *sender_addr, packet *pkt)
{
 	char 					*sender_ip = inet_ntoa(sender_addr->sin_addr);
	short 					sender_port = htons(sender_addr->sin_port);
	std::map<std::string, unit>::iterator 	it;
	unit					file_key;
	struct stat 				file_stat;
	
	
	pthread_mutex_lock(&mutex);
		// before creating the new file we check if it already exist
		it = file_map.find(pkt->header.name);
		if(file_map.end() == it)	// the file does'n exist we create it
		{
			pthread_mutex_lock(&stdout_lock);
				fprintf(stdout, "info - %s:%d : asked to create file %s\n", sender_ip, sender_port, pkt->header.name);
			pthread_mutex_unlock(&stdout_lock);
			create_file(pkt);
			// then we add the new file to the map !
			if(-1 == stat(pkt->header.name, &file_stat))
			{
				pthread_mutex_lock(&stdout_lock);
					fprintf(stderr, "error - %s\n", "stat ended with failure.");
				pthread_mutex_unlock(&stdout_lock);
				file_key.mtime = 1;
			}
			else
				file_key.mtime = file_stat.st_mtime;
			file_key.exist = 1;
			file_key.state = 'A';
			pthread_mutex_lock(&map_lock);		
				file_map.insert(std::pair<std::string, unit>(pkt->header.name, file_key));
			pthread_mutex_unlock(&map_lock);
			// finaly we ask for the content of the file
			file_content_request(sender_addr, pkt->header.name);
		}
	pthread_mutex_unlock(&mutex);
}

void	file_content_reply(struct sockaddr_in *sender_addr, packet *pkt)
{
	file_send_content_request(sender_addr, pkt->header.name);
}

void	file_remove_reply(struct sockaddr_in *sender_addr, packet *pkt)
{
	char 					*sender_ip = inet_ntoa(sender_addr->sin_addr);
	short 					sender_port = htons(sender_addr->sin_port);
	std::map<std::string, unit>::iterator 	it;	
	
	pthread_mutex_lock(&mutex);
		// before removing the file we check if it exist
		it = file_map.find(pkt->header.name);
		if(file_map.end() != it)	// the file exist we can remove it
		{
			pthread_mutex_lock(&stdout_lock);
				fprintf(stdout, "info - %s:%d : asked to remove file %s\n", sender_ip, sender_port, pkt->header.name);
			pthread_mutex_unlock(&stdout_lock);
			remove_file(pkt->header.name);
			// then we delete the file from the map !
			pthread_mutex_lock(&map_lock);
				file_map.erase(it);
			pthread_mutex_unlock(&map_lock);
		}
	pthread_mutex_unlock(&mutex);
}

void	file_update_reply(struct sockaddr_in *sender_addr, packet *pkt)
{
	char 					*sender_ip = inet_ntoa(sender_addr->sin_addr);
	short 					sender_port = htons(sender_addr->sin_port);
	std::map<std::string, unit>::iterator 	it;
	unit					file_key;
	struct stat 				file_stat;
	
	pthread_mutex_lock(&stdout_lock);
		fprintf(stdout, "info - %s:%d : asked to update file %s\n", sender_ip, sender_port, pkt->header.name);
	pthread_mutex_unlock(&stdout_lock);
	pthread_mutex_lock(&map_lock);
		// before updating the file we check if it already exist
		it = file_map.find(pkt->header.name);
		if(file_map.end() == it)	// the file does'n exist so we create it
		{
			create_file(pkt);
			// then we add the new file to the map !
			if(-1 == stat(pkt->header.name, &file_stat))
			{
				pthread_mutex_lock(&stdout_lock);
					fprintf(stderr, "error - %s\n", "stat ended with failure.");
				pthread_mutex_unlock(&stdout_lock);
				file_key.mtime = 1;
			}
			else
				file_key.mtime = file_stat.st_mtime;
			file_key.exist = 1;
			file_key.state = 'A';		
			file_map.insert(std::pair<std::string, unit>(pkt->header.name, file_key));
			// finaly we ask for the content of the file
			file_content_request(sender_addr, pkt->header.name);
		}
		else	// the file exist
		{
			file_map.erase(it);
			create_file(pkt);
			// then we add the new file to the map !
			if(-1 == stat(pkt->header.name, &file_stat))
			{
				pthread_mutex_lock(&stdout_lock);
					fprintf(stderr, "error - %s\n", "stat ended with failure.");
				pthread_mutex_unlock(&stdout_lock);
				file_key.mtime = 1;
			}
			else
				file_key.mtime = file_stat.st_mtime;
			file_key.exist = 1;
			file_key.state = 'A';		
			file_map.insert(std::pair<std::string, unit>(pkt->header.name, file_key));
			// finaly we ask for the content of the file
			file_content_request(sender_addr, pkt->header.name);
		}
	pthread_mutex_unlock(&map_lock);
}

void	dir_create_reply(struct sockaddr_in *sender_addr, packet *pkt)
{
	char 					*sender_ip = inet_ntoa(sender_addr->sin_addr);
	short 					sender_port = htons(sender_addr->sin_port);
	std::map<std::string, unit>::iterator 	it;
	unit					dir_state;
	
	pthread_mutex_lock(&map_lock);
		it = dir_map.find(pkt->header.name);
		if(dir_map.end() == it)		// verify if the dir is already created
		{
			pthread_mutex_lock(&stdout_lock);
				fprintf(stdout, "info - %s:%d : asked to create directory %s\n", sender_ip, sender_port, pkt->header.name);
			pthread_mutex_unlock(&stdout_lock);
			create_dir(pkt->header.name);
			dir_state.exist = 1;
			dir_map.insert(std::pair<std::string, unit>(pkt->header.name, dir_state));
		}
	pthread_mutex_unlock(&map_lock);
}

void	dir_remove_reply(struct sockaddr_in *sender_addr, packet *pkt)
{
	char 					*sender_ip = inet_ntoa(sender_addr->sin_addr);
	short 					sender_port = htons(sender_addr->sin_port);
	std::map<std::string, unit>::iterator 	it;

	pthread_mutex_lock(&mutex);
		it = dir_map.find(pkt->header.name);
		if(dir_map.end() != it)		// verify if the dir is already created
		{
			pthread_mutex_lock(&stdout_lock);
				fprintf(stdout, "info - %s:%d : asked to remove directory %s\n", sender_ip, sender_port, pkt->header.name);
			pthread_mutex_unlock(&stdout_lock);
			remove_dir(pkt->header.name);
			dir_map.erase(it);
		}
	pthread_mutex_unlock(&mutex);
}


/************************************************************************************************/
/*	REQUEST				<action>			<for>		<option>*/
/************************************************************************************************/
/*	-file_content_request		ask to the peer to send		PEER		<o>	*/
/*					the content of the file.				*/
/*	-create_peer_request		ask to the tracker to add	TRACKER		<c>	*/
/*					my peer.						*/
/*	-leave_peer_request		ask to the tracker to 		TRACKER 	<l>	*/
/*					remove my peer.						*/
/*	-create_file_request		ask to all peers for creating	PEERS		<f>	*/
/*					a new file.						*/
/*	-file_send_content_request	give the content of file to	PEER		<a>	*/
/*					peer.							*/
/*	-file_remove_request		ask to all peers for removing	PEERS		<r>	*/
/*					a file.							*/
/*	-file_update_request		ask to all peers for updating	PEERS		<m>	*/
/*					a file.							*/
/*	-dir_create_request		ask to all peers to create	PEERS		<d>	*/
/*					a new directory.					*/
/*	-dir_remove_request		ask to all peers to remove	PEERS		<r>	*/
/*					the directory.						*/
/************************************************************************************************/

void	file_content_request(struct sockaddr_in *sender_addr, const char* filename)
{
	packet	pkt;
	int	status;

	pkt.header.type = 'o';
	pkt.header.name_length = strlen(filename) + 1;
	memcpy(pkt.header.name, filename, pkt.header.name_length);
	status = sendto(sock, &pkt, sizeof(pkt.header) + pkt.header.name_length, 0, (struct sockaddr *)sender_addr, sizeof(struct sockaddr_in));
	if(-1 == status)
	{
		pthread_mutex_lock(&stdout_lock);
			fprintf(stderr, "error - %s\n", "when sending | add file content request | packet to peer");
		pthread_mutex_unlock(&stdout_lock);
	}
}

void	create_peer_request(void)
{
	packet 	pkt;
	int	status;

	pkt.header.type = 'c';
	status = sendto(sock, &pkt, sizeof(pkt.header), 0, (struct sockaddr *)&tracker_addr, sizeof(struct sockaddr_in));
	if(-1 == status)
	{
		pthread_mutex_lock(&stdout_lock);
			fprintf(stderr, "error - %s\n", "when sending | peer create request | packet to tracker");
		pthread_mutex_unlock(&stdout_lock);
	}
}

void	leave_peer_request(void)
{
	packet 	pkt;
	int	status;

	pkt.header.type = 'l';
	status = sendto(sock, &pkt, sizeof(pkt.header), 0, (struct sockaddr *)&tracker_addr, sizeof(struct sockaddr_in));
	if(-1 == status)
	{
		pthread_mutex_lock(&stdout_lock);
			fprintf(stderr, "error - %s\n", "when sending | peer leave request | packet to tracker");
		pthread_mutex_unlock(&stdout_lock);
	}
}

void	file_create_request(const char* filename)
{
	packet 	pkt;
	short	port;
	int	i;
	int	status;

	pkt.header.type = 'f';
	pkt.header.name_length = strlen(filename) + 1;
	memcpy(pkt.header.name, filename, pkt.header.name_length);
	pthread_mutex_lock(&peer_list_lock);
		for(i = 0; i < peer_num; ++i)
		{
			port = htons(peer_list[i].sin_port);
			if(port != my_port)	// we dont send this request for ourself
			{
				status = sendto(sock, &pkt, 
						sizeof(pkt.header) + pkt.header.name_length, 
						0, 
						(struct sockaddr *)&(peer_list[i]), 
						sizeof(struct sockaddr_in));
				if(status == -1) 
				{
					pthread_mutex_lock(&stdout_lock);
						fprintf(stderr, "error - %s %d\n", "when sending | file create request | packet to peers", i);
					pthread_mutex_unlock(&stdout_lock);
				}
			}
		}
	pthread_mutex_unlock(&peer_list_lock);
}

void	file_send_content_request(struct sockaddr_in *sender_addr, const char* filename)
{
	packet 					pkt;
	int					status;
	std::string				line;
	std::string				part;
	std::vector<std::string>		parts;
	std::vector<std::string>::iterator	it;
	unsigned int				cursor = 0;
	unsigned int				len = 0;
	
	pkt.header.type = 'a';
	pkt.header.name_length = strlen(filename) + 1;
	memcpy(pkt.header.name, filename, pkt.header.name_length);
	
	pthread_mutex_lock(&peer_list_lock);
	std::ifstream 	file(filename, std::ios::in);
	if(file)
	{	
		std::getline(file, line, '\0');
		len = line.length();
		do
		{
			part = line.substr(cursor, 512);
			parts.push_back(part);
			cursor += part.length();
		} while(cursor < len);
		for(it = parts.begin(); it != parts.end(); ++it)
		{
			memset(pkt.payload, 0, 512);
			pkt.header.payload_length = (*it).length();
			memcpy(pkt.payload, (*it).c_str(), pkt.header.payload_length);
			status = sendto(sock, 
					&pkt, 
					sizeof(pkt.header) + pkt.header.name_length + pkt.header.payload_length, 
					0, 
					(struct sockaddr *)sender_addr, 
					sizeof(struct sockaddr_in));
			if(status == -1) 
			{
				pthread_mutex_lock(&stdout_lock);
					fprintf(stderr, "error - %s\n", "when sending | file create request | packet to peer.");
				pthread_mutex_unlock(&stdout_lock);
			}
		}
	}
	else
	{
		pthread_mutex_lock(&stdout_lock);
			fprintf(stderr, "error - %s %s.\n", "when opening file", filename);
		pthread_mutex_unlock(&stdout_lock);
	}
	pthread_mutex_unlock(&peer_list_lock);
}

void	file_remove_request(const char*	filename)
{
	packet 	pkt;
	short	port;
	int	i;
	int	status;

	pkt.header.type = 's';
	pkt.header.name_length = strlen(filename) + 1;
	memcpy(pkt.header.name, filename, pkt.header.name_length);
	pthread_mutex_lock(&peer_list_lock);
		for(i = 0; i < peer_num; ++i)
		{
			port = htons(peer_list[i].sin_port);
			if(port != my_port)	// we dont send this request for ourself
			{
				status = sendto(sock, &pkt, 
						sizeof(pkt.header) + pkt.header.name_length, 
						0, 
						(struct sockaddr *)&(peer_list[i]), 
						sizeof(struct sockaddr_in));
				if(status == -1) 
				{
					pthread_mutex_lock(&stdout_lock);
						fprintf(stderr, "error - %s %d\n", "when sending | file remove request | packet to peers", i);
					pthread_mutex_unlock(&stdout_lock);
				}
			}
		}
	pthread_mutex_unlock(&peer_list_lock);
}

void 	file_update_request(const char* filename)
{
	packet 	pkt;
	short	port;
	int	i;
	int	status;

	pkt.header.type = 'm';
	pkt.header.name_length = strlen(filename) + 1;
	memcpy(pkt.header.name, filename, pkt.header.name_length);
	pthread_mutex_lock(&peer_list_lock);
		for(i = 0; i < peer_num; ++i)
		{
			port = htons(peer_list[i].sin_port);
			if(port != my_port)	// we dont send this request for ourself
			{
				status = sendto(sock, &pkt, 
						sizeof(pkt.header) + pkt.header.name_length, 
						0, 
						(struct sockaddr *)&(peer_list[i]), 
						sizeof(struct sockaddr_in));
				if(status == -1) 
				{
					pthread_mutex_lock(&stdout_lock);
						fprintf(stderr, "error - %s %d\n", "when sending | file update request | packet to peers", i);
					pthread_mutex_unlock(&stdout_lock);
				}
			}
		}
	pthread_mutex_unlock(&peer_list_lock);
}

void	dir_create_request(const char* dirname)
{
	packet 	pkt;
	short	port;
	int	i;
	int	status;

	pkt.header.type = 'd';
	memcpy(pkt.header.name, dirname, strlen(dirname) + 1);
	pthread_mutex_lock(&peer_list_lock);
	for (i = 0; i < peer_num; i++)
	{
		port = htons(peer_list[i].sin_port);
		if(port != my_port)
		{
			status = sendto(sock, &pkt, sizeof(pkt.header), 0, (struct sockaddr *)&(peer_list[i]), sizeof(struct sockaddr_in));
			if(status == -1) 
			{
				pthread_mutex_lock(&stdout_lock);
					fprintf(stderr, "error - %s %d\n", "when sending | create dir request | packet to peers", i);
				pthread_mutex_unlock(&stdout_lock);
			}
		}
	}
	pthread_mutex_unlock(&peer_list_lock);
}

void	dir_remove_request(const char* dirname)
{
	packet 	pkt;
	short	port;
	int	i;
	int	status;

	pkt.header.type = 'r';
	memcpy(pkt.header.name, dirname, strlen(dirname) + 1);
	pthread_mutex_lock(&peer_list_lock);
	for(i = 0; i < peer_num; i++)
	{
		port = htons(peer_list[i].sin_port);
		if(port != my_port)
		{
			status = sendto(sock, &pkt, sizeof(pkt.header), 0, (struct sockaddr *)&(peer_list[i]), sizeof(struct sockaddr_in));
			if(status == -1) 
			{
				pthread_mutex_lock(&stdout_lock);
					fprintf(stderr, "error - %s %d\n", "when sending | remove dir request | packet to peers", i);
				pthread_mutex_unlock(&stdout_lock);
			}
		}
	}
	pthread_mutex_unlock(&peer_list_lock);
}

/****************************************************************************************/
/*	LISTENER		<action>						*/
/****************************************************************************************/
/*	-receive_packet		receive packet from network and do an action.		*/
/****************************************************************************************/

void	receive_packet(void) 
{
	struct sockaddr_in 	sender_addr;
	socklen_t 		addrlen = 10;
	packet 			pkt;
	int 			status;

	while(1)
	{
		status = recvfrom(sock, &pkt, sizeof(pkt), 0, (struct sockaddr *)&sender_addr, &addrlen);
		if(status == -1) 
		{
			pthread_mutex_lock(&stdout_lock);
				fprintf(stderr, "error - %s\n", "when receiving a packet, ignoring.");
			pthread_mutex_unlock(&stdout_lock);
			continue;
		}
		switch(pkt.header.type)
		{
			case 'c':
				create_peer_reply(&pkt);
				break;
			case 'l':
				leave_peer_reply(&pkt);
				break;
			case 'u': 
				peers_connection_updates(&pkt);
				break;
			case 'f':
				file_create_reply(&sender_addr, &pkt);
				break;
			case 'a':
				add_content(&pkt);
				break;
			case 'o':
				file_content_reply(&sender_addr, &pkt);
				break;
			case 's':
				file_remove_reply(&sender_addr, &pkt);
				break;
			case 'm':
				file_update_reply(&sender_addr, &pkt);
				break;
			case 'd':
				dir_create_reply(&sender_addr, &pkt);
				break;
			case 'r':
				dir_remove_reply(&sender_addr, &pkt);
				break;
			default:
				pthread_mutex_lock(&stdout_lock);
					fprintf(stderr, "error - %s %c\n", "received packet with unknown type.", pkt.header.type);
				pthread_mutex_unlock(&stdout_lock);
				break;
		}
	}
}

/****************************************************************************************/
/*	SYNCHRONISATION									*/
/****************************************************************************************/

void	mark_file_deleted(void)
{
	std::map<std::string, unit>::iterator 	it;

	pthread_mutex_lock(&mutex);
		for(it = file_map.begin(); it != file_map.end(); ++it)
			(*it).second.exist = 0;
	pthread_mutex_unlock(&mutex);
}

void	mark_dir_deleted(void)
{
	std::map<std::string, unit>::iterator 	it;

	pthread_mutex_lock(&mutex);
		for(it = dir_map.begin(); it != dir_map.end(); ++it)
			(*it).second.exist = 0;
	pthread_mutex_unlock(&mutex);
}

int	list(const char *name, const struct stat *status, int type)
{
	std::map<std::string, unit>::iterator 	fit;
	std::map<std::string, unit>::iterator 	dit;
	unit					file_key;
	unit					dir_key;

	if(type == FTW_NS)
  		return(1);
 	if((type == FTW_F) && (0 != strcmp("./peer", name)))
  	{
		pthread_mutex_lock(&mutex);
			fit = file_map.find(name);
			if(file_map.end() != fit)
			{
				(*fit).second.exist = 1;
				if(status->st_mtime != (*fit).second.mtime)
				{
					(*fit).second.mtime = status->st_mtime;
					(*fit).second.state = 'M';
					pthread_mutex_lock(&stdout_lock);				
						fprintf(stdout, "info - file %s %s\n", name, "modified.");
					pthread_mutex_unlock(&stdout_lock);
				}
			}
			else
			{
				file_key.mtime = status->st_mtime;
				file_key.exist = 1;
				file_key.state = 'A';
				file_map.insert(std::pair<std::string, unit>(name, file_key));
				pthread_mutex_lock(&stdout_lock);				
					fprintf(stdout, "info - file %s %s\n", name, "added.");
				pthread_mutex_unlock(&stdout_lock);
			}
		pthread_mutex_unlock(&mutex);
	}
 	if((type == FTW_D) && (0 != strcmp(".", name)))
  	{
		pthread_mutex_lock(&mutex);
			dit = dir_map.find(name);
			if(dir_map.end() != dit)
				(*dit).second.exist = 1;
			else
			{
				dir_key.exist = 1;
				dir_map.insert(std::pair<std::string, unit>(name, dir_key));
				pthread_mutex_lock(&stdout_lock);
					fprintf(stdout, "info - %s %s %s\n", "directory", name, "created.");
				pthread_mutex_unlock(&stdout_lock);
			}
		pthread_mutex_unlock(&mutex);
	}
 	return(0);
}

void	update_file(void)
{
	std::map<std::string, unit>::iterator 	it;

	for(it = file_map.begin(); it != file_map.end(); ++it)
	{
		if(0 == (*it).second.exist)	// the file was removed
		{
			pthread_mutex_lock(&stdout_lock);
				fprintf(stdout, "info - file %s %s\n", (*it).first.c_str(), "removed.");
			pthread_mutex_unlock(&stdout_lock);
			// we send to all peers to remove the file
			file_remove_request((*it).first.c_str());
			file_map.erase(it);
			continue;		
		}		
		if('A' == (*it).second.state)	// the file was added
		{
			// we send to all peers to create the file
			file_create_request((*it).first.c_str());
		}
		if('M' == (*it).second.state)	// the file was modified
 		{
			// we send to all peers to update the file
			file_update_request((*it).first.c_str());
			(*it).second.state = 'A';
		}
	}
}

void	update_dir(void)
{
	std::map<std::string, unit>::iterator 		it;
	std::map<std::string, unit>::reverse_iterator 	rit;

	pthread_mutex_lock(&mutex);
	for(it = dir_map.begin(); it != dir_map.end(); ++it)
	{
		if(1 == (*it).second.exist)
			dir_create_request((*it).first.c_str());	
 	}
	for(rit = dir_map.rbegin(); rit != dir_map.rend(); ++rit)
	{
		if(0 == (*rit).second.exist)
		{
			pthread_mutex_lock(&stdout_lock);
				fprintf(stdout, "info - directory %s %s\n", (*rit).first.c_str(), "removed.");
			pthread_mutex_unlock(&stdout_lock);
			// send to all peers to remove the dir
			dir_remove_request((*rit).first.c_str());		
		}
	}
	for(it = dir_map.begin(); it != dir_map.end(); ++it)
	{
		if(0 == (*it).second.exist)
			dir_map.erase(it);
	}
	pthread_mutex_unlock(&mutex);
}


void*	synchronisation(void* argument)
{
	while(1)
	{
		mark_dir_deleted();
		mark_file_deleted();
		ftw(".", list, 1);
		update_dir();
		update_file();
		sleep(4);
	}	
	return(NULL);
}

void	signal_handler(int signal)
{
	switch(signal)
	{
		case SIGINT:
			leave_peer_request();
			exit(0);
			break;
		default:
			return;
	}
}

int		main(int ac, char** av)
{
	struct sigaction 	sa;
	pthread_t 		input_thread;

	if(ac < 3)
	{
		std::cerr << "Usage : ./peer <TRACKER IP> <PORT>" << std::endl;	
		return(1);
	}
	self_addr.sin_family = AF_INET; 
	self_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	self_addr.sin_port = htons(atoi(av[2]));
	my_port = atoi(av[2]);
	tracker_addr.sin_family = AF_INET;
	if(inet_aton(av[1], &tracker_addr.sin_addr) == 0)
	{
		fprintf(stderr, "error - %s\n", "when parsing tracker ip.");
		return(EXIT_FAILURE);
	}
	tracker_addr.sin_port = htons(TRACKER_PORT);
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock < 0)
	{
		fprintf(stderr, "error - %s\n", "when creating socket.");
		return(EXIT_FAILURE);
	}
	if(bind(sock, (struct sockaddr *)&self_addr, sizeof(self_addr)))
	{
		fprintf(stderr, "error - %s\n", "when binding.");
		return(EXIT_FAILURE);
	}
	pthread_create(&input_thread, NULL, synchronisation, NULL);
	pthread_detach(input_thread);
	// try to send our ip and port
	create_peer_request();
	// signal handler
	memset(&sa, 0, sizeof(sa));
  	sa.sa_handler = &signal_handler;
  	if(-1 == sigaction(SIGINT, &sa, NULL))
	{
		fprintf(stderr, "error - %s\n", "cannot handle signal SIGINT.");
		return(EXIT_FAILURE);
	}
	// loop
	receive_packet();
	return(EXIT_SUCCESS);
}
