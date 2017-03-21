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
 *    manager.c
 *
/*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <ami/manager.h>
#include <ami/core.h>


ast_packet *ami_login(int sock, const char *user, const char *secret, const char *events, const char *actionID){
  ast_packet *response = NULL;
  ast_packet *ast_resp = NULL;
  char *resp = NULL;
  int send_check;

  if(!user || !strlen(user)) {
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid user");
    return response;
  }

  if(sock <= 0){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid socket");
    return response;
  }

  response = ami_add_new_packet_item(response,"Action","Login");
  if(!response){
    return NULL;
  }
  
  if(ami_sock_writable(sock,AMI_NET_TIMEOUT)){
    ami_sock_send(sock,"Action: Login\r\nUsername: ");
    ami_sock_send(sock,user);
    ami_sock_send(sock,"\r\nSecret: ");
    if(secret && strlen(secret)){ ami_sock_send(sock,secret); }
    ami_sock_send(sock,"\r\nEvents: ");
    if(events && strlen(events)){
      ami_sock_send(sock,events);
    }else{
      ami_sock_send(sock,"Off");
    }
    ami_sock_send(sock, "\r\nActionID: ");
    ami_sock_send(sock, "ami_");
    if(actionID && strlen(actionID)){
      ami_sock_send(sock, actionID);
    }
    send_check = ami_sock_send(sock,"\r\n\r\n");

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

void ami_logoff(int sock, const char *actionID){
  if(sock <= 0){ return; }
  ami_sock_send(sock,"Action: Logoff");
    ami_sock_send(sock, "\r\nActionID: ");
    ami_sock_send(sock, "ami_");
    if(actionID && strlen(actionID)){
      ami_sock_send(sock, actionID);
    }
    ami_sock_send(sock,"\r\n\r\n");
}

short int ami_set_events(int sock, short int system, short int call, short int log, short int verbose,
                         short int command, short int agent, short int user, const char *actionID){
  char *resp;
  ast_packet *ast_resp;
  short int check;
  short int first = 1;
  
  if(sock <= 0){ return 0; }

  if(ami_sock_writable(sock,AMI_NET_TIMEOUT)){
    ami_sock_send(sock,"Action: Events\r\nEventMask: ");
    if(!system && !call && !log && !verbose && !command && !agent && !user){
      ami_sock_send(sock,"Off");
    }else{
      if(system){
        ami_sock_send(sock,"system");
        first = 0;
      }
      if(call){
        if(first){ first = 0;
        }else{ ami_sock_send(sock,","); }
        ami_sock_send(sock,"call");
      }
      if(log){
        if(first){ first = 0;
        }else{ ami_sock_send(sock,","); }
        ami_sock_send(sock,"log");
      }
      if(verbose){
        if(first){ first = 0;
        }else{ ami_sock_send(sock,","); }
        ami_sock_send(sock,"verbose");
      }
      if(command){
        if(first){ first = 0;
        }else{ ami_sock_send(sock,","); }
        ami_sock_send(sock,"command");
      }
      if(agent){
        if(first){ first = 0;
        }else{ ami_sock_send(sock,","); }
        ami_sock_send(sock,"agent");
      }
      if(user){
        if(first){ first = 0;
        }else{ ami_sock_send(sock,","); }
        ami_sock_send(sock,"user");
      }
    }
    ami_sock_send(sock, "\r\nActionID: ");
    ami_sock_send(sock, "ami_");
    if(actionID && strlen(actionID)){
      ami_sock_send(sock, actionID);
    }
    check = ami_sock_send(sock,"\r\n\r\n");
    if(check < 0){
      return 0;
    }
  }else{
    return 0;
  }
  
  if(ami_sock_readable(sock,AMI_NET_TIMEOUT)){
    resp = ami_sock_receive(sock);
  }else{
    return 0;
  }
  
  if(!resp || !strlen(resp)){
    if(resp){ free(resp); }
    return 0;
  }

  ast_resp = ami_parse_packet(resp);
  free(resp);
  if(!ast_resp){
    return 0;
  }else{
    if(!strcasecmp(ami_get_packet_item_value(ast_resp,"Response"),"Events")){
      ami_destroy_packet(ast_resp);
      return 1;
    }
    ami_destroy_packet(ast_resp);
  }
  return 0;
}

short int ami_ping(int sock, const char *actionID){
  ast_packet *ast_resp;
  char *resp;
  int check;

  if(sock <= 0){ return 0; }

  if(ami_sock_writable(sock,AMI_NET_TIMEOUT)){
    ami_sock_send(sock,"Action: Ping");
    ami_sock_send(sock, "\r\nActionID: ");
    ami_sock_send(sock, "ami_");
    if(actionID && strlen(actionID)){
      ami_sock_send(sock, actionID);
    }
    check = ami_sock_send(sock,"\r\n\r\n");
    if(check < 0){
      return 0;
    }
  }else{
    return 0;
  }
  
  if(ami_sock_readable(sock,AMI_NET_TIMEOUT)){
    resp = ami_sock_receive(sock);
  }else{
    return 0;
  }
  
  if(!resp || !strlen(resp)){
    if(resp){ free(resp); }
    return 0;
  }
    printf("resp : %s\n",resp);
  ast_resp = ami_parse_packet(resp);
  free(resp);
  if(!ast_resp){
    return 0;
  }else{
   // if(!strcasecmp(ami_get_packet_item_value(ast_resp,"Response"),"Pong")){
    if(!strcasecmp(ami_get_packet_item_value(ast_resp,"Ping"),"Pong")){
      ami_destroy_packet(ast_resp);
      return 1;
    }
    ami_destroy_packet(ast_resp);
  }
  return 0;
}

ast_packet *ami_list_commands(int sock, const char *actionID){
  ast_packet *response = NULL;
  ast_packet *ast_resp = NULL;
  char *resp = NULL;
  int send_check;
  
  if(sock <= 0){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid socket");
    return response;
  }

  response = ami_add_new_packet_item(response,"Action","ListCommands");
  if(!response){
    return NULL;
  }
  
  if(ami_sock_writable(sock,AMI_NET_TIMEOUT)){
    ami_sock_send(sock,"Action: ListCommands");
    ami_sock_send(sock, "\r\nActionID: ");
    ami_sock_send(sock, "ami_");
    if(actionID && strlen(actionID)){
      ami_sock_send(sock, actionID);
    }
    send_check = ami_sock_send(sock,"\r\n\r\n");
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
