#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include<errno.h>
#include <ami/manager.h>
#include "net.h"
#include "app-amitest.h"

int asterisk_connect(char *host, int port)
{
	int sock;
	char buff[30];
	char welcome_string[] = "Asterisk Call Manager";
	int i = 0;
	char c;
	int bytes;
	char buff2[1024] = "";

	buff[0] = '\0';
	sock = sock_connect(host, port);

	if(sock <= 0){
		return -1;
	}

	if(ami_sock_readable(sock,NET_WAIT_TIMEOUT_MIL)){
		while((bytes = recv(sock,(void *)&c,1,0))){
			if(bytes <= 0){
				sprintf(buff2, "An error occurred reading from the asterisk socket: %s", strerror(errno));
				printf(buff2);
				sock_close(sock);
				return -1;
			}
			if(i >= sizeof(buff)){
				break;
			}
			if(c == '\n'){ 
				break;
			}

			buff[i+1] = buff[i];
			buff[i] = c;
			i++;
		} //End while

	}else{
		sprintf(buff2, "Something bad happened.. either a timeout or an error: %s",strerror(errno));
		printf(buff2);
		sock_close(sock);
		return -1;
	}

	if(strstr(buff,welcome_string) == NULL){
		sprintf(buff2, "Did not recieve the welcome message from asterisk!");
		printf(buff2);
		sock_close(sock);
		return -1;
	}
			
	return sock;
}



void asterisk_close(int sock)
{
	sock_close(sock);
}

int main(int argc, char *argv[])
{

	int sock = 0;
	char ast_host[20] = "";
	int ast_port;
	char ast_user[50] = "";
	char ast_secret[50] = "";

	ast_packet *p = NULL;
	ast_packet *resp = NULL;
	ast_packet *ptr;
	char *retval;

	int error = 0;
	char buff[1024] = "";
	char status[100] = "";     /* device's status */

	/* initial AMI values */
	strcpy(ast_host, "127.0.0.1");
	ast_port = 5038;
	strcpy(ast_user, "admin");
	strcpy(ast_secret, "111111");

	sprintf(buff, "Ast user: %s\n", ast_user);
	printf(buff);
	sprintf(buff, "Ast secret: %s\n", ast_secret);
	printf(buff);
	sprintf(buff, "Ast host: %s\n", ast_host);
	printf(buff);
	sprintf(buff, "Ast port: %d\n", ast_port);
	printf(buff);

	if (!error) {
		sock = asterisk_connect(ast_host, ast_port);
		if (sock <= 0) {
			sprintf(buff, "Unable to connect to asterisk server %s:%i\n", ast_host, ast_port);
			printf(buff);
			error = 1;
		}
	}


	if (!error) {
		p = ami_login(sock, ast_user, ast_secret, NULL, NULL);
		if (p == NULL) {
			sprintf(buff, "Error logging in\n");
			printf(buff);
			error = 1;
		}

		ami_destroy_packet(p);
		p = NULL;
	}

   /* AMI Ping-Pong test */
	short int ret = 0;
	ret = ami_ping(sock, NULL);
	printf("ret = %d\n",ret);
	if(ret > 0){
		printf("AMI ping-pong test: Connection is alive\n");
	}else{
		printf("AMI ping-pong test: Connection is offline\n");
	}

	/* get server uptime */

	if (!error) {
		if (DEBUG) {
			printf("Getting asterisk uptime\n");
		}

		resp = ami_command(sock, "core show uptime", NULL);
		if (!resp) {
			printf("ERROR: Unable to retrieve server uptime response from server\n");
			return 1;
		}
	}

		retval = ami_get_packet_item_value(resp, "Response");
		if (!retval || strcasecmp(retval, "Success") != 0) {
			sprintf(buff, "Received an error from asterisk: %s\n", ami_get_packet_item_value(resp, "Message"));
			printf(buff);
			ami_destroy_packet_group(resp);
			return 1;
		}

	char uptime[200] = "";
	strcpy(uptime, &ami_get_packet_item_value(resp, "Chunk1")[31]);
	uptime[strlen(uptime) - 2] = '\0';

	sprintf(buff, "Uptime: %s\n\n", uptime);
	printf(buff);

	if (resp) {
		ami_destroy_packet_group(resp);
	}

	/* get server version */
	
	if (!error) {
		if (DEBUG) {
			printf("Getting Asterisk server version\n");
		}

		resp = ami_server_version(sock, "core show version", NULL);
		if (!resp) {
			printf("ERROR: Unable to retrieve server version response from server\n");
			return 1;
		}
	}

	retval = ami_get_packet_item_value(resp, "Response");

	if (!retval || strcasecmp(retval, "Success") != 0) {
		sprintf(buff, "Received an error from asterisk: %s\n", ami_get_packet_item_value(resp, "Message"));
		printf(buff);
		ami_destroy_packet_group(resp);
		return 1;
	}

	char version[100] = "";
	strcpy(version, ami_get_packet_item_value(resp, "Version"));
	sprintf(buff, "Version: %s\n", version);
	printf(buff);

	if (resp) {
		ami_destroy_packet_group(resp);
	}


	/* get SIP peers */
	
	if (!error) {
		if (DEBUG) {
			printf("Initializing Peers\n");
		}

		resp = ami_sip_peers(sock, NULL);
		if (!resp) {
			printf("ERROR: Unable to retrieve sip peers list response from server\n");
			return 1;
		}
	}


	if (error) {
		printf("Aborting...\n");
		return 1;
	}

	retval = ami_get_packet_item_value(resp, "Response");

	if (!retval || strcasecmp(retval, "Success") != 0) {
		sprintf(buff, "Received an error from asterisk: %s\n", ami_get_packet_item_value(resp, "Message"));
		printf(buff);
		ami_destroy_packet_group(resp);
		return 1;
	}

	ptr = resp->next;
	while (ptr) {
		// end of data
		if (!strcasecmp(ami_get_packet_item_value(ptr, "Event"), "PeerlistComplete")) {
			break;
		}

		if (DEBUG) {
			sprintf(buff, "Event: %s\n", ami_get_packet_item_value(ptr, "Event"));
			printf(buff);
			sprintf(buff, "Channeltype: %s\n", ami_get_packet_item_value(ptr, "Channeltype"));
			printf(buff);
			sprintf(buff, "ObjectName: %s\n", ami_get_packet_item_value(ptr, "ObjectName"));
			printf(buff);
			sprintf(buff, "ChanObjectType: %s\n", ami_get_packet_item_value(ptr, "ChanObjectType"));
			printf(buff);
			sprintf(buff, "IPaddress: %s\n", ami_get_packet_item_value(ptr, "IPaddress"));
			printf(buff);
			sprintf(buff, "IPport: %s\n", ami_get_packet_item_value(ptr, "IPport"));
			printf(buff);
			sprintf(buff, "Dynamic: %s\n", ami_get_packet_item_value(ptr, "Dynamic"));
			printf(buff);
			sprintf(buff, "Natsupport: %s\n", ami_get_packet_item_value(ptr, "Natsupport"));
			printf(buff);
			sprintf(buff, "ACL: %s\n", ami_get_packet_item_value(ptr, "ACL"));
			printf(buff);
			sprintf(buff, "Status: %s\n", ami_get_packet_item_value(ptr, "Status"));
			printf(buff);
			printf("-------------\n\n\n");

		} // DEBUG

		ptr = ptr->next;
	}

	if (resp) {
		ami_destroy_packet_group(resp);
	}

	ami_logoff(sock, NULL);	

	if (sock) {
		asterisk_close(sock);
	}

	return 0;
}


