/*/
 *  Copyright © Justin Camp 2006
 *
 *  This file is part of Asterisk Manager Interface (AMI).
 *
 *  Asterisk Manager Interface is free software; you can 
 *  redistribute it and/or modify it under the terms of the 
 *  GNU General Public License as published by the 
 *  Free Software Foundation; either version 2 of the License, or 
 * (at your option) any later version.
 *
 *  Asterisk Manager Interface is distributed in the hope that 
 *  it will be useful, but WITHOUT ANY WARRANTY; without even the 
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Asterisk Manager Interface; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *  Send bugs, patches, comments, flames, the next winning lottery ticket of any
 *  state, province, country or otherwise inhabitable area (with a lottery system)
 *  or any other ideas or information worth communicating to j@intuitivecreations.com
/*/

/*/
 *    info.c
 *    Yuesichiu modified ami_server_version since asterisk 11.15.0, OpenVox,Inc 2017-03-21 10:30
/*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ami/admin.h>
#include <ami/info.h>

/* Get asterisk version in new way since asterisk 1.4, deprecated, 2017-03-21 10:20 */
ast_packet *ami_server_version(int sock, const char *command, const char *actionID)
{
	ast_packet *response = NULL;
	int send_check;
	char *resp = NULL;
	char *ptr1 = NULL;
	char *ptr2 = NULL;
	char ver[128];

	if(sock <= 0){
		response = ami_add_new_packet_item(response, "Response","Error");
		ami_add_new_packet_item(response,"Message", "Invalid socket");
		return response;
	}

	//response = ami_add_new_packet_item(response,"Action","ServerVersion");
	response = ami_add_new_packet_item(response, "Action", "Command");
	if(!response){
		return NULL;
	}

	if(ami_sock_writable(sock, AMI_NET_TIMEOUT)){
		ami_sock_send(sock,"Action: Command\r\nCommand: core show version");
		ami_sock_send(sock, "\r\nActionID: ");
		ami_sock_send(sock, "ami_");
		if(actionID && strlen(actionID)){
			ami_sock_send(sock, actionID);
		}
		send_check = ami_sock_send(sock, "\r\n\r\n");
		if(send_check < 0){
			ami_add_new_packet_item(response, "Response", "Error");
			ami_add_new_packet_item(response, "Message", "Error communicating with the * server");
		return response;
		}
	}else{
		ami_add_new_packet_item(response, "Response", "Error");
		ami_add_new_packet_item(response, "Message", "timed out communicating with the * server");
		return response;
	}

	if(ami_sock_readable(sock, AMI_NET_TIMEOUT)){
		resp = ami_sock_receive(sock);
	}else{
		ami_add_new_packet_item(response, "Response", "Error");
		ami_add_new_packet_item(response, "Message", "timed out waiting for a response from the * server");
		return response;
	}

	if(!resp || !strlen(resp)){
		if(resp){
			free(resp); 
		}
		ami_add_new_packet_item(response,"Response","Error");
		ami_add_new_packet_item(response,"Message","received an empty response from server");
		return response;
	}

	ptr1 = strstr(resp,"--END COMMAND--");
	if(!ptr1){
		free(resp);
		return response;
	}

	ptr1 = strstr(resp, "Asterisk ");
	if(!ptr1){
		free(resp);
		ami_add_new_packet_item(response,"Response","Error");
		ami_add_new_packet_item(response,"Message","received an invalid/unknown response from server");
		return response;
	}

	ptr1 = strchr(ptr1, ' ');
	ptr1++;
	ptr2 = strchr(ptr1, ' ');

	memset(ver, '\0', sizeof(ver));
	strncpy(ver, ptr1, ptr2 - ptr1);
	ami_add_new_packet_item(response,"Response","Success");
	ami_add_new_packet_item(response,"Version",ver);
	ami_add_new_packet_item(response,"ActionID",actionID);
	free(resp);  
	return response;
}

ast_packet *ami_sip_peers(int sock, const char *actionID){
  ast_packet *response = NULL;
  ast_packet *ast_resp = NULL;
  char *resp = NULL;
  char *end_check = NULL;
  int send_check;
  int max_check = 10; /* 5 seconds timeout waiting for the end of input */
  int ccheck = 0;

  if(sock <= 0){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid socket");
    return response;
  }

  response = ami_add_new_packet_item(response,"Action","SIPpeers");
  if(!response){
    return NULL;
  }
  
  if(ami_sock_writable(sock,AMI_NET_TIMEOUT)){
    ami_sock_send(sock,"Action: SIPpeers");
    ami_sock_send(sock, "\r\nActionID: ");
    ami_sock_send(sock, "ami_");
    if(actionID && strlen(actionID)){
      ami_sock_send(sock, actionID);
    }
    send_check = ami_sock_send(sock, "\r\n\r\n");
    if(send_check < 0){
      ami_add_new_packet_item(response,"Response","Error");
      ami_add_new_packet_item(response,"Message","Error communicating with the * server");
      return response;
    }
  }else{
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","timed out communicating with the * server");
    return response;
  }
  
  if(ami_sock_readable(sock,AMI_NET_TIMEOUT)){
    resp = ami_sock_receive(sock);
  }else{
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","timed out waiting for a response from the * server");
    return response;
  }
  
  if(!resp || !strlen(resp)){
    if(resp){ free(resp); }
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","received an empty response from server");
    return response;
  }

  ast_resp = ami_parse_packet(resp);
  free(resp);
  if(ast_resp){
    ami_merge_packets(response,ast_resp);
    resp = ami_get_packet_item_value(response, "Response");
    if(resp && !strcasecmp(resp, "Error")){
      return response;
    }
    while(!end_check || (end_check && strcasecmp(end_check,"PeerlistComplete"))){
      if(!ami_sock_readable(sock,AMI_NET_TIMEOUT)){
        if(ccheck >= max_check){
          ami_destroy_packet_group(response);
          response = ami_add_new_packet_item(NULL,"Action","SIPpeers");
          ami_add_new_packet_item(response,"Response","Error");
          ami_add_new_packet_item(response,"Message","unable to retrieve entire response from server");
          return response;
        }else{
          ccheck++;
          continue;
        }
      }
      resp = ami_sock_receive(sock);
      if(!resp || !strlen(resp)){
        ami_destroy_packet_group(response);
        return NULL;
      }
      ast_resp = ami_parse_packet(resp);
      free(resp);
      ami_add_packet(response,ast_resp);
      end_check = ami_get_packet_item_value(ast_resp,"Event");
    }
  }else{
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","unable to parse response from server");
  }
  return response;
}

ast_packet *ami_sip_show_peer(int sock, const char *peer, const char *actionID){
  ast_packet *response = NULL;
  ast_packet *ast_resp = NULL;
  char *resp = NULL;
  int send_check;

  if(!peer || !strlen(peer)) {
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid peer");
    return response;
  }

  if(sock <= 0){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid socket");
    return response;
  }

  response = ami_add_new_packet_item(response,"Action","SIPshowpeer");
  if(!response){
    return NULL;
  }
  
  if(ami_sock_writable(sock,AMI_NET_TIMEOUT)){
    ami_sock_send(sock,"Action: SIPshowpeer\r\nPeer: ");
    ami_sock_send(sock,peer);
    ami_sock_send(sock, "\r\nActionID: ");
    ami_sock_send(sock, "ami_");
    if(actionID && strlen(actionID)){
      ami_sock_send(sock, actionID);
    }
    send_check = ami_sock_send(sock, "\r\n\r\n");
    if(send_check < 0){
      ami_add_new_packet_item(response,"Response","Error");
      ami_add_new_packet_item(response,"Message","Error communicating with the * server");
      return response;
    }
  }else{
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","timed out communicating with the * server");
    return response;
  }
  
  if(ami_sock_readable(sock,AMI_NET_TIMEOUT)){
    resp = ami_sock_receive(sock);
  }else{
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","timed out waiting for a response from the * server");
    return response;
  }
  
  if(!resp || !strlen(resp)){
    if(resp){ free(resp); }
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","received an empty response from server");
    return response;
  }
  ast_resp = ami_parse_packet(resp);
  free(resp);
  if(ast_resp){
    ami_merge_packets(response,ast_resp);
  }else{
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","unable to parse response from server");
  }
  /* seems to be an issue with error messages where to many '\r\n's are sent... catch them here */
  if(ami_sock_readable(sock,50)){
    resp = ami_sock_receive(sock);
    if(resp){ free(resp); }
  }
  return response;
}

ast_packet *ami_agents(int sock, const char *actionID){
  ast_packet *response = NULL;
  ast_packet *ast_resp = NULL;
  char *resp = NULL;
  char *end_check = NULL;
  int send_check;
  int max_check = 10; /* 5 seconds timeout waiting for the end of input */
  int ccheck = 0;

  if(sock <= 0){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid socket");
    return response;
  }

  response = ami_add_new_packet_item(response,"Action","Agents");
  if(!response){
    return NULL;
  }
  
  if(ami_sock_writable(sock,AMI_NET_TIMEOUT)){
    ami_sock_send(sock,"Action: Agents");
    ami_sock_send(sock, "\r\nActionID: ");
    ami_sock_send(sock, "ami_");
    if(actionID && strlen(actionID)){
      ami_sock_send(sock, actionID);
    }
    send_check = ami_sock_send(sock, "\r\n\r\n");
    if(send_check < 0){
      ami_add_new_packet_item(response,"Response","Error");
      ami_add_new_packet_item(response,"Message","Error communicating with the * server");
      return response;
    }
  }else{
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","timed out communicating with the * server");
    return response;
  }
  
  if(ami_sock_readable(sock,AMI_NET_TIMEOUT)){
    resp = ami_sock_receive(sock);
  }else{
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","timed out waiting for a response from the * server");
    return response;
  }
  
  if(!resp || !strlen(resp)){
    if(resp){ free(resp); }
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","received an empty response from server");
    return response;
  }

  ast_resp = ami_parse_packet(resp);
  free(resp);
  if(ast_resp){
    ami_merge_packets(response,ast_resp);
    resp = ami_get_packet_item_value(response, "Response");
    if(resp && !strcasecmp(resp, "Error")){
      return response;
    }
    while(!end_check || (end_check && strcasecmp(end_check,"AgentsComplete"))){
      if(!ami_sock_readable(sock,AMI_NET_TIMEOUT)){
        if(ccheck >= max_check){
          ami_destroy_packet_group(response);
          response = ami_add_new_packet_item(NULL,"Action","Agents");
          ami_add_new_packet_item(response,"Response","Error");
          ami_add_new_packet_item(response,"Message","unable to retrieve entire response from server");
          return response;
        }else{
          ccheck++;
          continue;
        }
      }
      resp = ami_sock_receive(sock);
      if(!resp || !strlen(resp)){
        ami_destroy_packet_group(response);
        return NULL;
      }
      ast_resp = ami_parse_packet(resp);
      free(resp);
      ami_add_packet(response,ast_resp);
      end_check = ami_get_packet_item_value(ast_resp,"Event");
    }
  }else{
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","unable to parse response from server");
  }
  return response;
}

ast_packet *ami_parked_calls(int sock, const char *actionID){
  ast_packet *response = NULL;
  ast_packet *ast_resp = NULL;
  char *resp = NULL;
  char *end_check = NULL;
  int send_check;
  int max_check = 10; /* 5 seconds timeout waiting for the end of input */
  int ccheck = 0;

  if(sock <= 0){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid socket");
    return response;
  }

  response = ami_add_new_packet_item(response,"Action","ParkedCalls");
  if(!response){
    return NULL;
  }
  
  if(ami_sock_writable(sock,AMI_NET_TIMEOUT)){
    ami_sock_send(sock,"Action: ParkedCalls");
    ami_sock_send(sock, "\r\nActionID: ");
    ami_sock_send(sock, "ami_");
    if(actionID && strlen(actionID)){
      ami_sock_send(sock, actionID);
    }
    send_check = ami_sock_send(sock, "\r\n\r\n");
    if(send_check < 0){
      ami_add_new_packet_item(response,"Response","Error");
      ami_add_new_packet_item(response,"Message","Error communicating with the * server");
      return response;
    }
  }else{
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","timed out communicating with the * server");
    return response;
  }
  
  if(ami_sock_readable(sock,AMI_NET_TIMEOUT)){
    resp = ami_sock_receive(sock);
  }else{
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","timed out waiting for a response from the * server");
    return response;
  }
  
  if(!resp || !strlen(resp)){
    if(resp){ free(resp); }
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","received an empty response from server");
    return response;
  }

  ast_resp = ami_parse_packet(resp);
  free(resp);
  if(ast_resp){
    ami_merge_packets(response,ast_resp);
    resp = ami_get_packet_item_value(response, "Response");
    if(resp && !strcasecmp(resp, "Error")){
      return response;
    }
    while(!end_check || (end_check && strcasecmp(end_check,"ParkedCallsComplete"))){
      if(!ami_sock_readable(sock,AMI_NET_TIMEOUT)){
        if(ccheck >= max_check){
          ami_destroy_packet_group(response);
          response = ami_add_new_packet_item(NULL,"Action","ParkedCalls");
          ami_add_new_packet_item(response,"Response","Error");
          ami_add_new_packet_item(response,"Message","unable to retrieve entire response from server");
          return response;
        }else{
          ccheck++;
          continue;
        }
      }
      resp = ami_sock_receive(sock);
      if(!resp || !strlen(resp)){
        ami_destroy_packet_group(response);
        return NULL;
      }
      ast_resp = ami_parse_packet(resp);
      free(resp);
      ami_add_packet(response,ast_resp);
      end_check = ami_get_packet_item_value(ast_resp,"Event");
    }
  }else{
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","unable to parse response from server");
  }
  return response;
}

ast_packet *ami_channels(int sock, const char *actionID){
  ast_packet *response = NULL;
  ast_packet *ast_resp = NULL;
  char *resp = NULL;
  char *end_check = NULL;
  int send_check;
  int max_check = 10; /* 5 seconds timeout waiting for the end of input */
  int ccheck = 0;

  if(sock <= 0){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid socket");
    return response;
  }

  response = ami_add_new_packet_item(response,"Action","Status");
  if(!response){
    return NULL;
  }
  
  if(ami_sock_writable(sock,AMI_NET_TIMEOUT)){
    ami_sock_send(sock,"Action: Status");
    ami_sock_send(sock, "\r\nActionID: ");
    ami_sock_send(sock, "ami_");
    if(actionID && strlen(actionID)){
      ami_sock_send(sock, actionID);
    }
    send_check = ami_sock_send(sock, "\r\n\r\n");
    if(send_check < 0){
      ami_add_new_packet_item(response,"Response","Error");
      ami_add_new_packet_item(response,"Message","Error communicating with the * server");
      return response;
    }
  }else{
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","timed out communicating with the * server");
    return response;
  }
  
  if(ami_sock_readable(sock,AMI_NET_TIMEOUT)){
    resp = ami_sock_receive(sock);
  }else{
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","timed out waiting for a response from the * server");
    return response;
  }
  
  if(!resp || !strlen(resp)){
    if(resp){ free(resp); }
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","received an empty response from server");
    return response;
  }

  ast_resp = ami_parse_packet(resp);
  free(resp);
  if(ast_resp){
    ami_merge_packets(response,ast_resp);
    resp = ami_get_packet_item_value(response, "Response");
    if(resp && !strcasecmp(resp, "Error")){
      return response;
    }
    while(!end_check || (end_check && strcasecmp(end_check,"StatusComplete"))){
      if(!ami_sock_readable(sock,AMI_NET_TIMEOUT)){
        if(ccheck >= max_check){
          ami_destroy_packet_group(response);
          response = ami_add_new_packet_item(NULL,"Action","Status");
          ami_add_new_packet_item(response,"Response","Error");
          ami_add_new_packet_item(response,"Message","unable to retrieve entire response from server");
          return response;
        }else{
          ccheck++;
          continue;
        }
      }
      resp = ami_sock_receive(sock);
      if(!resp || !strlen(resp)){
        ami_destroy_packet_group(response);
        return NULL;
      }
      ast_resp = ami_parse_packet(resp);
      free(resp);
      ami_add_packet(response,ast_resp);
      end_check = ami_get_packet_item_value(ast_resp,"Event");
    }
  }else{
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","unable to parse response from server");
  }
  return response;
}


ast_packet *ami_queue_status(int sock, const char *queue, const char *actionID){
  ast_packet *response = NULL;
  ast_packet *ast_resp = NULL;
  char *resp = NULL;
  char *end_check = NULL;
  int send_check;
  int max_check = 10; /* 5 seconds timeout waiting for the end of input */
  int ccheck = 0;

  if(sock <= 0){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid socket");
    return response;
  }

  response = ami_add_new_packet_item(response,"Action","QueueStatus");
  if(!response){
    return NULL;
  }
  
  if(ami_sock_writable(sock,AMI_NET_TIMEOUT)){
    ami_sock_send(sock,"Action: QueueStatus");
    if(queue && strlen(queue)){
      ami_sock_send(sock,"\r\nQueue: ");
      ami_sock_send(sock,queue);
    }
    ami_sock_send(sock, "\r\nActionID: ");
    ami_sock_send(sock, "ami_");
    if(actionID && strlen(actionID)){
      ami_sock_send(sock, actionID);
    }
    send_check = ami_sock_send(sock, "\r\n\r\n");
    if(send_check < 0){
      ami_add_new_packet_item(response,"Response","Error");
      ami_add_new_packet_item(response,"Message","Error communicating with the * server");
      return response;
    }
  }else{
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","timed out communicating with the * server");
    return response;
  }
  
  if(ami_sock_readable(sock,AMI_NET_TIMEOUT)){
    resp = ami_sock_receive(sock);
  }else{
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","timed out waiting for a response from the * server");
    return response;
  }
  
  if(!resp || !strlen(resp)){
    if(resp){ free(resp); }
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","received an empty response from server");
    return response;
  }

  ast_resp = ami_parse_packet(resp);
  free(resp);
  if(ast_resp){
    ami_merge_packets(response,ast_resp);
    resp = ami_get_packet_item_value(response, "Response");
    if(resp && !strcasecmp(resp, "Error")){
      return response;
    }
    while(!end_check || (end_check && strcasecmp(end_check,"QueueStatusComplete"))){
      if(!ami_sock_readable(sock,AMI_NET_TIMEOUT)){
        if(ccheck >= max_check){
          ami_destroy_packet_group(response);
          response = ami_add_new_packet_item(NULL,"Action","QueueStatus");
          ami_add_new_packet_item(response,"Response","Error");
          ami_add_new_packet_item(response,"Message","unable to retrieve entire response from server");
          return response;
        }else{
          ccheck++;
          continue;
        }
      }
      resp = ami_sock_receive(sock);
      if(!resp || !strlen(resp)){
        ami_destroy_packet_group(response);
        return NULL;
      }
      ast_resp = ami_parse_packet(resp);
      free(resp);
      ami_add_packet(response,ast_resp);
      end_check = ami_get_packet_item_value(ast_resp,"Event");
    }
  }else{
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","unable to parse response from server");
  }
  return response;
}


ast_packet *ami_extension_state(int sock, const char *exten, const char *context, const char *actionID){
  ast_packet *response = NULL;
  ast_packet *ast_resp = NULL;
  char *resp = NULL;
  int send_check;

  if(!exten || !strlen(exten)) {
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid extension");
    return response;
  }
  if(!context || !strlen(context)) {
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid context");
    return response;
  }

  if(sock <= 0){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid socket");
    return response;
  }

  response = ami_add_new_packet_item(response,"Action","ExtensionState");
  if(!response){
    return NULL;
  }
  
  if(ami_sock_writable(sock,AMI_NET_TIMEOUT)){
    ami_sock_send(sock,"Action: ExtensionState\r\nExten: ");
    ami_sock_send(sock,exten);
    ami_sock_send(sock,"\r\nContext: ");
    ami_sock_send(sock,context);
    ami_sock_send(sock, "\r\nActionID: ");
    ami_sock_send(sock, "ami_");
    if(actionID && strlen(actionID)){
      ami_sock_send(sock, actionID);
    }
    send_check = ami_sock_send(sock, "\r\n\r\n");

    if(send_check < 0){
      ami_add_new_packet_item(response,"Response","Error");
      ami_add_new_packet_item(response,"Message","Error communicating with the * server");
      return response;
    }
  }else{
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","timed out communicating with the * server");
    return response;
  }
  
  if(ami_sock_readable(sock,AMI_NET_TIMEOUT)){
    resp = ami_sock_receive(sock);
  }else{
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","timed out waiting for a response from the * server");
    return response;
  }
  
  if(!resp || !strlen(resp)){
    if(resp){ free(resp); }
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","received an empty response from server");
    return response;
  }

  ast_resp = ami_parse_packet(resp);
  free(resp);
  if(ast_resp){
    ami_merge_packets(response,ast_resp);
  }else{
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","unable to parse response from server");
  }
  return response;
}

ast_packet *ami_mailbox_count(int sock, int mailbox, const char *context, const char *actionID){
  ast_packet *response = NULL;
  ast_packet *ast_resp = NULL;
  char *resp = NULL;
  char ibuff[16];
  int send_check;

  if(mailbox < 0) {
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid mailbox");
    return response;
  }
  if(!context || !strlen(context)) {
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid context");
    return response;
  }

  if(sock <= 0){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid socket");
    return response;
  }

  response = ami_add_new_packet_item(response,"Action","MailboxCount");
  if(!response){
    return NULL;
  }
  
  snprintf(ibuff,sizeof(ibuff),"%i",mailbox);
  
  if(ami_sock_writable(sock,AMI_NET_TIMEOUT)){
    ami_sock_send(sock,"Action: MailboxCount\r\nMailbox: ");
    ami_sock_send(sock,ibuff);
    ami_sock_send(sock,"@");
    ami_sock_send(sock,context);
    ami_sock_send(sock, "\r\nActionID: ");
    ami_sock_send(sock, "ami_");
    if(actionID && strlen(actionID)){
      ami_sock_send(sock, actionID);
    }
    send_check = ami_sock_send(sock, "\r\n\r\n");

    if(send_check < 0){
      ami_add_new_packet_item(response,"Response","Error");
      ami_add_new_packet_item(response,"Message","Error communicating with the * server");
      return response;
    }
  }else{
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","timed out communicating with the * server");
    return response;
  }
  
  if(ami_sock_readable(sock,AMI_NET_TIMEOUT)){
    resp = ami_sock_receive(sock);
  }else{
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","timed out waiting for a response from the * server");
    return response;
  }
  
  if(!resp || !strlen(resp)){
    if(resp){ free(resp); }
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","received an empty response from server");
    return response;
  }

  ast_resp = ami_parse_packet(resp);
  free(resp);
  if(ast_resp){
    ami_merge_packets(response,ast_resp);
  }else{
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","unable to parse response from server");
  }
  return response;
}

ast_packet *ami_mailbox_status(int sock, int mailbox, const char *context, const char *actionID){
  ast_packet *response = NULL;
  ast_packet *ast_resp = NULL;
  char *resp = NULL;
  char ibuff[16];
  int send_check;

  if(mailbox < 0) {
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid mailbox");
    return response;
  }
  if(!context || !strlen(context)) {
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid context");
    return response;
  }

  if(sock <= 0){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid socket");
    return response;
  }

  response = ami_add_new_packet_item(response,"Action","MailboxStatus");
  if(!response){
    return NULL;
  }
  
  snprintf(ibuff,sizeof(ibuff),"%i",mailbox);
  
  if(ami_sock_writable(sock,AMI_NET_TIMEOUT)){
    ami_sock_send(sock,"Action: MailboxStatus\r\nMailbox: ");
    ami_sock_send(sock,ibuff);
    ami_sock_send(sock,"@");
    ami_sock_send(sock,context);
    ami_sock_send(sock, "\r\nActionID: ");
    ami_sock_send(sock, "ami_");
    if(actionID && strlen(actionID)){
      ami_sock_send(sock, actionID);
    }
    send_check = ami_sock_send(sock, "\r\n\r\n");

    if(send_check < 0){
      ami_add_new_packet_item(response,"Response","Error");
      ami_add_new_packet_item(response,"Message","Error communicating with the * server");
      return response;
    }
  }else{
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","timed out communicating with the * server");
    return response;
  }
  
  if(ami_sock_readable(sock,AMI_NET_TIMEOUT)){
    resp = ami_sock_receive(sock);
  }else{
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","timed out waiting for a response from the * server");
    return response;
  }
  
  if(!resp || !strlen(resp)){
    if(resp){ free(resp); }
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","received an empty response from server");
    return response;
  }

  ast_resp = ami_parse_packet(resp);
  free(resp);
  if(ast_resp){
    ami_merge_packets(response,ast_resp);
  }else{
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","unable to parse response from server");
  }
  return response;
}
