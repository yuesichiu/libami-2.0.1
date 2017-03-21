#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "manager.h"

#define MAX_MSG_SIZE 512
#define SERVER_ADDR	 "127.0.0.1"
#define CLIENT_ADDR 	 "127.0.0.1"
#define SERVER_PORT	5038
#define CLIENT_PORT	5038

int main()
{
	int sockfd;
	int rec_len = 0;
	ast_packet *resp = NULL;
	char *retval = NULL;
	char version[100] = {0};
	
	struct sockaddr_in serveraddr, clientaddr;
	char msg[MAX_MSG_SIZE] = {0};
	bzero((char *)&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
	serveraddr.sin_port = htons(SERVER_PORT);

	bzero((char *)&clientaddr, sizeof(clientaddr));
	clientaddr.sin_family = AF_INET;
	clientaddr.sin_addr.s_addr = INADDR_ANY;
	clientaddr.sin_port = htons(CLIENT_PORT);


	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	printf("Create socket...\n");
	bind(sockfd, (struct sockaddr *)&clientaddr, sizeof(clientaddr));

	printf("Binding successfuly...\n");
	connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	printf("Connected...\n");
	
	
	ast_packet * p = NULL;
	printf("ami_login before \n");
	p = ami_login(sockfd, "admin", "111111", NULL, NULL);
	printf("ami_login\n");
	if (p == NULL) {
      	 	 printf("Error logging in\n");
       		 goto errout;
    	} else {
       	 	ami_destroy_packet(p);
     	   	p = NULL;
    	}


	
	resp = ami_server_version(sockfd, NULL);
  	if (!resp) {
        	printf("ERROR: Unable to retrieve server version response from server\n");
        	goto errout;
    	}

	retval = ami_get_packet_item_value(resp, "Response");
	if (!retval || strcasecmp(retval, "Success") != 0) {
	    printf("Received an error from asterisk: %s\n", ami_get_packet_item_value(resp, "Message"));
	    ami_destroy_packet_group(resp);
	    goto errout;
	}

	printf("retval : %s\n", retval);

   	 strcpy(version, ami_get_packet_item_value(resp, "Version"));
   	 printf("Asterisk version: %s\n", version);
/*
	if(bind(sockfd, (struct sockaddr *)&clientaddr, sizeof(clientaddr)) < 0);
	{
		printf("bind failed\n");
		goto errout;
	}
	printf("Binding successfuly...\n");

	connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	printf("Connected...\n");

	snprintf(msg, MAX_MSG_SIZE, "Action: Login\r\nUsername: admin\r\nSecret:111111\r\nActionID: 1\r\n\r\n");
	send(sockfd, msg, strlen(msg) + 1, 0);
	printf("msg : %s\n", msg);

	bzero(msg, 0);
	if((rec_len = recv(sockfd, msg, MAX_MSG_SIZE, 0)) == -1) {  
      		printf("recv error\n");  
       		goto errout;  
    	}  

	msg[rec_len]  = '\0';  
    	printf("Received : %s ",msg);  */

errout:
	close(sockfd);
	return 0;
	
	
	
}

