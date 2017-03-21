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
 *    admin.c
 *
/*/

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ami/admin.h>
#include <ami/core.h>

ast_packet *ami_dbshow(int sock, const char *actionID){
  char *resp = NULL;
  int str_i = 1;
  int kv_i = 0;
  short int skip = 0;
  short int isFamily = 1;
  short int isKey = 0;
  short int familyReady = 0;
  char family[MAX_AST_PACKET_ITEM_NAME_SIZE];
  char key[MAX_AST_PACKET_ITEM_NAME_SIZE];
  char val[MAX_AST_PACKET_ITEM_VALUE_SIZE];
  ast_packet *response = NULL;
  int ret;

  if(sock <= 0){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid socket");
    return response;
  }
  
  response = ami_add_new_packet_item(response,"Action","DBShow");
  if(!response){
    return NULL;
  }

  if(ami_sock_writable(sock,AMI_NET_TIMEOUT)){
    ami_sock_send(sock,"Action: Command\r\nCommand: database show");
    ami_sock_send(sock, "\r\nActionID: ");
    ami_sock_send(sock, "ami_");
    if(actionID && strlen(actionID)){
      ami_sock_send(sock, actionID);
    }
    ret = ami_sock_send(sock, "\r\n\r\n");
    if(ret < 0){
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
    ami_add_new_packet_item(response,"Message","timed out waiting for a response");
    return response;
  }
  
  if(!resp || !strlen(resp)){
    if(resp){ free(resp); }
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","received an empty response from server");
    return response;
  }
  
  family[0] = '\0';
  key[0] = '\0';
  val[0] = '\0';
  ami_add_new_packet_item(response,"Response","Success");
  while(str_i < strlen(resp)){
    if(resp[str_i] == '\n'){
      isFamily = 1;
      isKey = 0;
      familyReady = 0;
      str_i+=1;
      kv_i = 0;
      skip = 0;
      ami_trim(family);
      ami_trim(key);
      ami_trim(val);
      if(strlen(family) && strlen(key)){
        if(!ami_add_new_packet_item(response,"Family",(family+1))){
          ami_destroy_packet(response);
          return NULL;
        }
        if(!ami_add_new_packet_item(response,"Key",key)){
          ami_destroy_packet(response);
          return NULL;
        }
        if(!ami_add_new_packet_item(response,"Value",val)){
          ami_destroy_packet(response);
          return NULL;
        }
        family[0] = '\0';
        key[0] = '\0';
        val[0] = '\0';
      }
      continue;
    }
    if(skip){
      str_i++;
      continue;
    }
    if(kv_i == 0 && isFamily && resp[str_i] != '/'){
      skip = 1;
      str_i++;
      continue;
    }
    if(!isFamily && isKey && resp[str_i] == ':'){
      isKey = 0;
      kv_i = 0;
      str_i++;
      continue;
    }
    if(isFamily && familyReady && resp[str_i] == '/'){
      isFamily = 0;
      isKey = 1;
      kv_i = 0;
      str_i++;
      continue;
    }
    if(isFamily){
      if(kv_i >= sizeof(family)){
        family[0] = '\0';
        skip = 1;
      }else{
        family[kv_i] = resp[str_i];
        family[kv_i+1] = '\0';
        if(!familyReady && resp[str_i] != '/'){
          familyReady = 1;
        }
      }
    }else if(isKey){
      if(kv_i >= sizeof(key)){
        key[0] = '\0';
        skip = 1;
      }else{
        key[kv_i] = resp[str_i];
        key[kv_i+1] = '\0';
      }
    }else{
      if(kv_i >= sizeof(val)){
        val[0] = '\0';
        skip = 1;
      }else{
        val[kv_i] = resp[str_i];
        val[kv_i+1] = '\0';
      }
    }
    kv_i++;
    str_i++;
  }

  ami_add_new_packet_item(response,"ActionID",actionID);

  return response;
}

ast_packet *ami_dbget(int sock, const char *family, const char *key, const char *actionID){
  ast_packet *response = NULL;
  ast_packet *ast_resp = NULL;
  ast_packet_item *pi = NULL;
  ast_packet_item *tmp_pi = NULL;
  char *resp = NULL;
  int send_check;

  if(!family || !strlen(family)) {
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid family");
    return response;
  }
  if (!key || !strlen(key)){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid key");
    return response;
  }
  
  if(sock <= 0){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid socket");
    return response;
  }

  response = ami_add_new_packet_item(response,"Action","DBGet");
  if(!response){
    return NULL;
  }
  
  if(ami_sock_writable(sock,AMI_NET_TIMEOUT)){
    ami_sock_send(sock,"Action: DBGet\r\nFamily: ");
    ami_sock_send(sock,family);
    ami_sock_send(sock,"\r\nKey: ");
    ami_sock_send(sock,key);
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
  if(ast_resp){
    if(!strcasecmp(ami_get_packet_item_value(ast_resp,"Response"),"Success")){
      free(resp);
      ami_destroy_packet(ast_resp);
      if(ami_sock_readable(sock,AMI_NET_TIMEOUT)){
        resp = ami_sock_receive(sock);
      }else{
        ami_add_new_packet_item(response,"Response","Error");
        ami_add_new_packet_item(response,"Message","timed out waiting for a response from the * server");
        return response;
      }
      ast_resp = ami_parse_packet(resp);
      ami_add_new_packet_item(response,"Response","Success");
      pi = ast_resp->first_item;
      while(pi){
        if(!strcmp(pi->name,"Family") || !strcmp(pi->name,"Key") || !strcmp(pi->name,"Val")){
          ami_add_packet_item(response->first_item,pi);
          tmp_pi = pi->next;
          ami_unlink_packet_item(ast_resp,pi);
          pi = tmp_pi;
          continue;
        }
        pi = pi->next;
      }
      ami_destroy_packet(ast_resp);
    }else{
      ami_merge_packets(response,ast_resp);
    }
  }else{
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","missing part of the response from server");
  }
  free(resp);
  if(!response){
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","unable to parse response from server");
    return response;
  }
  return response;
}

ast_packet *ami_dbput(int sock, const char *family, const char *key, const char *val, const char *actionID){
  ast_packet *response = NULL;
  ast_packet *ast_resp = NULL;
  char *resp = NULL;
  int send_check;

  if(!family || !strlen(family)) {
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid family");
    return response;
  }
  if (!key || !strlen(key)){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid key");
    return response;
  }
  if (!val){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid val");
    return response;
  }
  
  if(sock <= 0){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid socket");
    return response;
  }

  response = ami_add_new_packet_item(response,"Action","DBPut");
  if(!response){
    return NULL;
  }
  
  if(ami_sock_writable(sock,AMI_NET_TIMEOUT)){
    ami_sock_send(sock,"Action: DBPut\r\nFamily: ");
    ami_sock_send(sock,family);
    ami_sock_send(sock,"\r\nKey: ");
    ami_sock_send(sock,key);
    ami_sock_send(sock,"\r\nVal: ");
    ami_sock_send(sock,val);
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
  if(!ast_resp){
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Unable to parse response from server");
  }else{
    ami_merge_packets(response,ast_resp);
  }
  return response;
}

ast_packet *ami_dbdel(int sock, const char *family, const char *key, const char *actionID){
  ast_packet *response = NULL;
  char *resp = NULL;
  char *char_ptr;
  int send_check;

  if(!family || !strlen(family)) {
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid family");
    return response;
  }
  if (!key || !strlen(key)){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid key");
    return response;
  }
  
  if(sock <= 0){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid socket");
    return response;
  }

  response = ami_add_new_packet_item(response,"Action","DBDel");
  if(!response){
    return NULL;
  }
  
  if(ami_sock_writable(sock,AMI_NET_TIMEOUT)){
    ami_sock_send(sock,"Action: Command\r\nCommand: database del ");
    ami_sock_send(sock,family);
    ami_sock_send(sock," ");
    ami_sock_send(sock,key);
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
  
  char_ptr = strstr(resp,"--END COMMAND--");
  if(!char_ptr){
    free(resp);
    return response;
  }
  char_ptr--;
  *char_ptr = '\0';
  char_ptr = strrchr(resp,'\n');
  char_ptr++;
  if(strstr(char_ptr,"removed")){
    ami_add_new_packet_item(response,"Response","Success");
  }else{
    ami_add_new_packet_item(response,"Response","Error");
  }
  ami_add_new_packet_item(response,"Message",char_ptr);
  ami_add_new_packet_item(response,"ActionID",actionID);
  free(resp);

  return response;
}

ast_packet *ami_dbdeltree(int sock, const char *family, const char *key, const char *actionID){
  ast_packet *response = NULL;
  char *resp = NULL;
  char *char_ptr;
  int send_check;

  if(!family || !strlen(family)) {
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid family");
    return response;
  }
  
  if(sock <= 0){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid socket");
    return response;
  }

  response = ami_add_new_packet_item(response,"Action","DBDeltree");
  if(!response){
    return NULL;
  }
  
  if(ami_sock_writable(sock,AMI_NET_TIMEOUT)){
    ami_sock_send(sock,"Action: Command\r\nCommand: database deltree ");
    ami_sock_send(sock,family);
    if(key){
      ami_sock_send(sock," ");
      ami_sock_send(sock,key);
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

  char_ptr = strstr(resp,"--END COMMAND--");
  char_ptr--;
  *char_ptr = '\0';
  char_ptr = strrchr(resp,'\n');
  char_ptr++;
  if(strstr(char_ptr,"removed")){
    ami_add_new_packet_item(response,"Response","Success");
  }else{
    ami_add_new_packet_item(response,"Response","Error");
  }
  ami_add_new_packet_item(response,"Message",char_ptr);
  ami_add_new_packet_item(response,"ActionID",actionID);
  free(resp);

  return response;
}

ast_packet *ami_queue_add(int sock, const char *interface, const char *queue, const char *actionID){
  ast_packet *response = NULL;
  ast_packet *ast_resp = NULL;
  char *resp = NULL;
  int send_check;

  if(!queue || !strlen(queue)) {
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid queue");
    return response;
  }
  if (!interface || !strlen(interface)){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid interface");
    return response;
  }
  
  if(sock <= 0){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid socket");
    return response;
  }

  response = ami_add_new_packet_item(response,"Action","QueueAdd");
  if(!response){
    return NULL;
  }
  
  if(ami_sock_writable(sock,AMI_NET_TIMEOUT)){
    ami_sock_send(sock,"Action: QueueAdd\r\nQueue: ");
    ami_sock_send(sock,queue);
    ami_sock_send(sock,"\r\nInterface: ");
    ami_sock_send(sock,interface);
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

ast_packet *ami_queue_remove(int sock, const char *interface, const char *queue, const char *actionID){
  ast_packet *response = NULL;
  ast_packet *ast_resp = NULL;
  char *resp = NULL;
  int send_check;

  if(!queue || !strlen(queue)) {
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid queue");
    return response;
  }
  if (!interface || !strlen(interface)){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid interface");
    return response;
  }
  
  if(sock <= 0){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid socket");
    return response;
  }

  response = ami_add_new_packet_item(response,"Action","QueueRemove");
  if(!response){
    return NULL;
  }
  
  if(ami_sock_writable(sock,AMI_NET_TIMEOUT)){
    ami_sock_send(sock,"Action: QueueRemove\r\nQueue: ");
    ami_sock_send(sock,queue);
    ami_sock_send(sock,"\r\nInterface: ");
    ami_sock_send(sock,interface);
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

ast_packet *ami_queue_pause(int sock, const char *interface, const char *queue, short int paused, const char *actionID){
  ast_packet *response = NULL;
  ast_packet *ast_resp = NULL;
  char *resp = NULL;
  int send_check;

  if (!interface || !strlen(interface)){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid interface");
    return response;
  }
  
  if(sock <= 0){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid socket");
    return response;
  }

  response = ami_add_new_packet_item(response,"Action","QueuePause");
  if(!response){
    return NULL;
  }
  
  if(ami_sock_writable(sock,AMI_NET_TIMEOUT)){
    ami_sock_send(sock,"Action: QueuePause\r\nInterface: ");
    ami_sock_send(sock,interface);
    if(paused){
      ami_sock_send(sock,"\r\nPaused: 1");
    }else{
      ami_sock_send(sock,"\r\nPaused: 0");
    }
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
  }else{
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","unable to parse response from server");
  }
  return response;
}

ast_packet *ami_agent_callback_login(int sock, int agent_id, const char *exten, const char *context, short int ack, int wrapup, const char *actionID){
  ast_packet *response = NULL;
  ast_packet *ast_resp = NULL;
  char *resp = NULL;
  char ibuff[16];
  int send_check;

  if(agent_id <= 0) {
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid agent_id");
    return response;
  }
  if (!exten || !strlen(exten)){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid extension");
    return response;
  }
  
  if(sock <= 0){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid socket");
    return response;
  }

  response = ami_add_new_packet_item(response,"Action","AgentCallbackLogin");
  if(!response){
    return NULL;
  }
  
  
  if(ami_sock_writable(sock,AMI_NET_TIMEOUT)){
    snprintf(ibuff,sizeof(ibuff),"%i",agent_id);
    ami_sock_send(sock,"Action: AgentCallbackLogin\r\nAgent: ");
    ami_sock_send(sock,ibuff);
    ami_sock_send(sock,"\r\nExten: ");
    ami_sock_send(sock,exten);
    if(context && strlen(context)){
      ami_sock_send(sock,"\r\nContext: ");
      ami_sock_send(sock,context);
    }
    if(ack){
      ami_sock_send(sock,"\r\nAckCall: 1");
    }
    if(wrapup > 0){
      snprintf(ibuff,sizeof(ibuff),"%i",wrapup);
      ami_sock_send(sock,"\r\nWrapupTime: ");
      ami_sock_send(sock,ibuff);
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
  }else{
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","unable to parse response from server");
  }
  return response;
}

ast_packet *ami_agent_logoff(int sock, int agent_id, short int soft, const char *actionID){
  ast_packet *response = NULL;
  ast_packet *ast_resp = NULL;
  char *resp = NULL;
  char ibuff[16];
  int send_check;

  if(agent_id <= 0) {
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid agent_id");
    return response;
  }
  
  if(sock <= 0){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid socket");
    return response;
  }

  response = ami_add_new_packet_item(response,"Action","AgentLogoff");
  if(!response){
    return NULL;
  }
  
  snprintf(ibuff,sizeof(ibuff),"%i",agent_id);
  
  if(ami_sock_writable(sock,AMI_NET_TIMEOUT)){
    ami_sock_send(sock,"Action: AgentLogoff\r\nAgent: ");
    ami_sock_send(sock,ibuff);
    if(soft){
      ami_sock_send(sock,"\r\nSoft: 1");
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
  }else{
    ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","unable to parse response from server");
  }
  return response;
}

ast_packet *ami_command(int sock, const char *command, const char *actionID){
  char *resp = NULL;
  int send_check;
  ast_packet *response;
  int chunk = 1;
  char *cptr;
  char chunkNameBuff[32];
  char chunkBuff[MAX_AST_PACKET_ITEM_VALUE_SIZE];
  
  if (!command || !strlen(command)){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid command");
    return response;
  }
  
  if(sock <= 0){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid socket");
    return response;
  }

  if(ami_sock_writable(sock,AMI_NET_TIMEOUT)){
    ami_sock_send(sock,"Action: Command\r\nCommand: ");
    ami_sock_send(sock,command);
    ami_sock_send(sock, "\r\nActionID: ");
    ami_sock_send(sock, "ami_");
    if(actionID && strlen(actionID)){
      ami_sock_send(sock, actionID);
    }
    send_check = ami_sock_send(sock, "\r\n\r\n");
    if(send_check < 0){
      return NULL;
    }
  }else{
    return NULL;
  }
  
  if(ami_sock_readable(sock,AMI_NET_TIMEOUT)){
    resp = ami_sock_receive(sock);
    if(!resp || !strlen(resp)){
      if(resp){ free(resp); }
      return NULL;
    }
  }else{
    return NULL;
  }
  
  response = ami_add_new_packet_item(NULL,"Action","Command");
  if(!response){ return NULL; }
  ami_add_new_packet_item(response,"Response","Success");
  ami_add_new_packet_item(response,"Message", "Command successfully sent");
  if(resp && strlen(resp)){
    cptr = strstr(resp,"--END COMMAND--");
    *cptr = '\0';
    cptr = strstr(resp,"\r\n");
    cptr = strstr(cptr+1,"\r\n");
    cptr += 2;
    while(strlen(cptr) > (MAX_AST_PACKET_ITEM_VALUE_SIZE - 1)){
      strncpy(chunkBuff,cptr,(MAX_AST_PACKET_ITEM_VALUE_SIZE - 1));
      chunkBuff[(MAX_AST_PACKET_ITEM_VALUE_SIZE - 1)] = '\0';
      snprintf(chunkNameBuff,sizeof(chunkNameBuff),"Chunk%i",chunk);
      ami_add_new_packet_item(response,chunkNameBuff,chunkBuff);
      chunk++;
      cptr += (MAX_AST_PACKET_ITEM_VALUE_SIZE - 1);
    }
    strncpy(chunkBuff,cptr,(MAX_AST_PACKET_ITEM_VALUE_SIZE - 1));
    chunkBuff[(MAX_AST_PACKET_ITEM_VALUE_SIZE - 1)] = '\0';
    snprintf(chunkNameBuff,sizeof(chunkNameBuff),"Chunk%i",chunk);
    ami_add_new_packet_item(response,chunkNameBuff,chunkBuff);
    free(resp);
  }

  return response;
}

ast_packet *ami_custom(int sock, ast_packet *cmd){
  ast_packet *response = NULL;
  ast_packet *ast_resp = NULL;
  ast_packet_item *iptr;
  char *resp = NULL;
  
  if (!cmd){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid command");
    return response;
  }
  
  if(sock <= 0){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid socket");
    return response;
  }

  response = ami_add_new_packet_item(response,"Action","Custom");
  if(!response){
    return NULL;
  }
  
  if(ami_sock_writable(sock,AMI_NET_TIMEOUT)){
    for(iptr=cmd->first_item; iptr; iptr=iptr->next){
      ami_sock_send(sock, iptr->name);
      ami_sock_send(sock, ": ");
      ami_sock_send(sock, iptr->value);
      ami_sock_send(sock, "\r\n");
    }
    ami_sock_send(sock, "\r\n");
  }else{
    return NULL;
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

ast_packet *ami_custom_free_form(int sock, ast_packet *cmd){
  char *resp = NULL;
  ast_packet_item *iptr;
  ast_packet *response;
  int chunk = 1;
  char *cptr;
  char chunkNameBuff[32];
  char chunkBuff[MAX_AST_PACKET_ITEM_VALUE_SIZE];
  
  if (!cmd){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid command");
    return response;
  }
  
  if(sock <= 0){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid socket");
    return response;
  }

  if(ami_sock_writable(sock,AMI_NET_TIMEOUT)){
    for(iptr=cmd->first_item; iptr; iptr=iptr->next){
      ami_sock_send(sock, iptr->name);
      ami_sock_send(sock, ": ");
      ami_sock_send(sock, iptr->value);
      ami_sock_send(sock, "\r\n");
    }
    ami_sock_send(sock, "\r\n");
  }else{
    return NULL;
  }
  
  if(ami_sock_readable(sock,AMI_NET_TIMEOUT)){
    resp = ami_sock_receive(sock);
    if(!resp || !strlen(resp)){
      if(resp){ free(resp); }
      return NULL;
    }
  }else{
    return NULL;
  }
  
  response = ami_add_new_packet_item(NULL,"Action","CustomFreeForm");
  if(!response){ return NULL; }
  ami_add_new_packet_item(response,"Response","Success");
  ami_add_new_packet_item(response,"Message", "Command successfully sent");
  if(resp && strlen(resp)){
    cptr = strstr(resp,"--END COMMAND--");
    if(cptr){ *cptr = '\0'; }
    cptr = strstr(resp,"\r\n");
    cptr = strstr(cptr+1,"\r\n");
    cptr += 2;
    while(strlen(cptr) > (MAX_AST_PACKET_ITEM_VALUE_SIZE - 1)){
      strncpy(chunkBuff,cptr,(MAX_AST_PACKET_ITEM_VALUE_SIZE - 1));
      chunkBuff[(MAX_AST_PACKET_ITEM_VALUE_SIZE - 1)] = '\0';
      snprintf(chunkNameBuff,sizeof(chunkNameBuff),"Chunk%i",chunk);
      ami_add_new_packet_item(response,chunkNameBuff,chunkBuff);
      chunk++;
      cptr += (MAX_AST_PACKET_ITEM_VALUE_SIZE - 1);
    }
    strncpy(chunkBuff,cptr,(MAX_AST_PACKET_ITEM_VALUE_SIZE - 1));
    chunkBuff[(MAX_AST_PACKET_ITEM_VALUE_SIZE - 1)] = '\0';
    snprintf(chunkNameBuff,sizeof(chunkNameBuff),"Chunk%i",chunk);
    ami_add_new_packet_item(response,chunkNameBuff,chunkBuff);
    free(resp);
  }
  return response;
}
