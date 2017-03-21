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
 *    channel.c
 *
/*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ami/core.h>
#include <ami/channel.h>

ast_packet *ami_absolute_timeout(int sock, const char *chan, long int timeout, const char *actionID){
  ast_packet *response = NULL;
  ast_packet *ast_resp = NULL;
  char *resp = NULL;
  char ibuff[32];
  int send_check;

  if(!chan || !strlen(chan)) {
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid channel");
    return response;
  }
  if (timeout <= 0){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid timeout");
    return response;
  }
  
  if(sock <= 0){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid socket");
    return response;
  }

  response = ami_add_new_packet_item(response,"Action","AbsoluteTimeout");
  if(!response){
    return NULL;
  }
  
  snprintf(ibuff,sizeof(ibuff),"%li",timeout);
  
  if(ami_sock_writable(sock,AMI_NET_TIMEOUT)){
    ami_sock_send(sock,"Action: AbsoluteTimeout\r\nChannel: ");
    ami_sock_send(sock,chan);
    ami_sock_send(sock,"\r\nTimeout: ");
    ami_sock_send(sock,ibuff);
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

ast_packet *ami_getvar(int sock, const char *varname, const char *chan, const char *actionID){
  ast_packet *response = NULL;
  ast_packet *ast_resp = NULL;
  char *resp = NULL;
  int send_check;

  if(!varname || !strlen(varname)) {
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid varname");
    return response;
  }
  
  if(sock <= 0){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid socket");
    return response;
  }

  response = ami_add_new_packet_item(response,"Action","Getvar");
  if(!response){
    return NULL;
  }
  
  if(ami_sock_writable(sock,AMI_NET_TIMEOUT)){
    ami_sock_send(sock,"Action: Getvar\r\nVariable: ");
    ami_sock_send(sock,varname);
    if(chan && strlen(chan)){
      ami_sock_send(sock,"\r\nChannel: ");
      ami_sock_send(sock,chan);
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

ast_packet *ami_setvar(int sock, const char *varname, const char *value, const char *chan, const char *actionID){
  ast_packet *response = NULL;
  ast_packet *ast_resp = NULL;
  char *resp = NULL;
  int send_check;

  if(!varname || !strlen(varname)) {
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid varname");
    return response;
  }
  if (!value || !strlen(value)){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid val");
    return response;
  }
  
  if(sock <= 0){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid socket");
    return response;
  }

  response = ami_add_new_packet_item(response,"Action","Setvar");
  if(!response){
    return NULL;
  }
  
  if(ami_sock_writable(sock,AMI_NET_TIMEOUT)){
    ami_sock_send(sock,"Action: Setvar\r\nVariable: ");
    ami_sock_send(sock,varname);
    ami_sock_send(sock,"\r\nValue: ");
    ami_sock_send(sock,value);
    if(chan && strlen(chan)){
      ami_sock_send(sock,"\r\nChannel: ");
      ami_sock_send(sock,chan);
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

ast_packet *ami_hangup(int sock, const char *chan, const char *actionID){
  ast_packet *response = NULL;
  ast_packet *ast_resp = NULL;
  char *resp = NULL;
  int send_check;

  if(!chan || !strlen(chan)) {
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid channel");
    return response;
  }
  
  if(sock <= 0){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid socket");
    return response;
  }

  response = ami_add_new_packet_item(response,"Action","Hangup");
  if(!response){
    return NULL;
  }
  
  if(ami_sock_writable(sock,AMI_NET_TIMEOUT)){
    ami_sock_send(sock,"Action: Hangup\r\nChannel: ");
    ami_sock_send(sock,chan);
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

ast_packet *ami_redirect(int sock, const char *chan, const char *context, const char *exten, int priority, const char *extrachan, const char *actionID){
  ast_packet *response = NULL;
  ast_packet *ast_resp = NULL;
  char ibuff[16];
  char *resp = NULL;
  int send_check;

  if(!chan || !strlen(chan)) {
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid channel");
    return response;
  }
  if (!context || !strlen(context)){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid context");
    return response;
  }
  if (!exten || !strlen(exten)){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid extension");
    return response;
  }
  if (priority < 0){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid priority");
    return response;
  }
  
  if(sock <= 0){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid socket");
    return response;
  }

  response = ami_add_new_packet_item(response,"Action","Redirect");
  if(!response){
    return NULL;
  }
  
  if(ami_sock_writable(sock,AMI_NET_TIMEOUT)){
    ami_sock_send(sock,"Action: Redirect\r\nChannel: ");
    ami_sock_send(sock,chan);
    ami_sock_send(sock,"\r\nExten: ");
    ami_sock_send(sock,exten);
    ami_sock_send(sock,"\r\nContext: ");
    ami_sock_send(sock,context);
    snprintf(ibuff,sizeof(ibuff),"%i",priority);
    ami_sock_send(sock,"\r\nPriority: ");
    ami_sock_send(sock,ibuff);
    if(extrachan && strlen(extrachan)){
      ami_sock_send(sock,"\r\nExtraChannel: ");
      ami_sock_send(sock,extrachan);
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

ast_packet *ami_originate(int sock,
                          const char *chan,
                          const char *context,
                          const char *exten,
                          int priority, 
                          const char *app,
                          const char *data,
                          long int timeout,
                          const char *caller_id,
                          const char **variables,
                          const char *accountCode,
                          short int async,
                          const char *actionID){
  ast_packet *response = NULL;
  ast_packet *ast_resp = NULL;
  char ibuff[32];
  char *resp = NULL;
  int send_check;
  int var_i = 0;

  if(!chan || !strlen(chan)) {
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid channel");
    return response;
  }
  
  if(sock <= 0){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid socket");
    return response;
  }
  
  response = ami_add_new_packet_item(response,"Action","Originate");
  if(!response){
    return NULL;
  }
  /* Context Priority and Exten must all be present if any one of them are present */
  if(context && strlen(context)){
    if(!exten || !strlen(exten) || priority <= 0){
      ami_add_new_packet_item(response,"Response","Error");
      ami_add_new_packet_item(response,"Message","If one of Exten, Priority, or Context is used, "
                                                 "all three must be used");
      return response;
    }
  }
  if(exten && strlen(exten)){
    if(priority <= 0 || (!context || !strlen(context))){
      ami_add_new_packet_item(response,"Response","Error");
      ami_add_new_packet_item(response,"Message","If one of Exten, Priority, or Context is used, "
                                                 "all three must be used");
      return response;
    }
  }
  if(priority > 0){
  if(!exten || !strlen(exten) || (!context || !strlen(context))){
      ami_add_new_packet_item(response,"Response","Error");
      ami_add_new_packet_item(response,"Message","If one of Exten, Priority, or Context is used, "
                                                 "all three must be used");
      return response;
    }
  }

  if(data && strlen(data)){
    if(!app || !strlen(app)){
      ami_add_new_packet_item(response,"Response","Error");
      ami_add_new_packet_item(response,"Message","Data argument requires the Application argument");
      return response;
    }
  }

  if(ami_sock_writable(sock,AMI_NET_TIMEOUT)){
    ami_sock_send(sock,"Action: Originate\r\nChannel: ");
    ami_sock_send(sock,chan);
    if(context && strlen(context)){
      ami_sock_send(sock,"\r\nContext: ");
      ami_sock_send(sock,context);
    }
    if(exten && strlen(exten)){
      ami_sock_send(sock,"\r\nExten: ");
      ami_sock_send(sock,exten);
    }
    if(priority > 0){
      snprintf(ibuff,sizeof(ibuff),"%i",priority);
      ami_sock_send(sock,"\r\nPriority: ");
      ami_sock_send(sock,ibuff);
    }
    if(app && strlen(app)){
      ami_sock_send(sock,"\r\nApplication: ");
      ami_sock_send(sock,app);
    }
    if(data && strlen(data)){
      ami_sock_send(sock,"\r\nData: ");
      ami_sock_send(sock,data);
    }
    if(timeout > 0){
      snprintf(ibuff,sizeof(ibuff),"%li",timeout);
      ami_sock_send(sock,"\r\nTimeout: ");
      ami_sock_send(sock,ibuff);
    }
    if(caller_id && strlen(caller_id)){
      ami_sock_send(sock,"\r\nCallerID: ");
      ami_sock_send(sock,caller_id);
    }
    if(variables){
      while(variables[var_i]){
        ami_sock_send(sock,"\r\nVariable: ");
        ami_sock_send(sock,variables[var_i]);
        var_i++;
      }
    }
    if(accountCode && strlen(accountCode)){
      ami_sock_send(sock,"\r\nAccount: ");
      ami_sock_send(sock,accountCode);
    }
    if(async){
      ami_sock_send(sock,"\r\nAsync: 1");
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

ast_packet *ami_monitor(int sock, const char *chan, const char *file, const char *format, short int combine, const char *actionID){
  ast_packet *response = NULL;
  ast_packet *ast_resp = NULL;
  char *resp = NULL;
  int send_check;

  if(!chan || !strlen(chan)) {
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid channel");
    return response;
  }
  
  if(sock <= 0){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid socket");
    return response;
  }

  response = ami_add_new_packet_item(response,"Action","Monitor");
  if(!response){
    return NULL;
  }
  
  if(ami_sock_writable(sock,AMI_NET_TIMEOUT)){
    ami_sock_send(sock,"Action: Monitor\r\nChannel: ");
    ami_sock_send(sock,chan);
    if(file && strlen(file)){
      ami_sock_send(sock,"\r\nFile: ");
      ami_sock_send(sock,file);
    }
    if(format && strlen(format)){
      ami_sock_send(sock,"\r\nFormat: ");
      ami_sock_send(sock,format);
    }
    if(combine){
      ami_sock_send(sock,"\r\nMix: 1");
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

/*/
 * ast_packet = ami_monitor_change(int sock, char channel, char file)
 * Change the monitoring session to a new file
/*/
ast_packet *ami_monitor_change(int sock, const char *chan, const char *file, const char *actionID){
  ast_packet *response = NULL;
  ast_packet *ast_resp = NULL;
  char *resp = NULL;
  int send_check;

  if(!chan || !strlen(chan)) {
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid channel");
    return response;
  }
  if(!file || !strlen(file)) {
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid filename");
    return response;
  }
  
  if(sock <= 0){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid socket");
    return response;
  }

  response = ami_add_new_packet_item(response,"Action","ChangeMonitor");
  if(!response){
    return NULL;
  }
  
  if(ami_sock_writable(sock,AMI_NET_TIMEOUT)){
    ami_sock_send(sock,"Action: ChangeMonitor\r\nChannel: ");
    ami_sock_send(sock,chan);
    ami_sock_send(sock,"\r\nFile: ");
    ami_sock_send(sock,file);
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

ast_packet *ami_monitor_stop(int sock, const char *chan, const char *actionID){
  ast_packet *response = NULL;
  ast_packet *ast_resp = NULL;
  char *resp = NULL;
  int send_check;

  if(!chan || !strlen(chan)) {
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid filename");
    return response;
  }
  
  if(sock <= 0){
    response = ami_add_new_packet_item(response,"Response","Error");
    ami_add_new_packet_item(response,"Message","Invalid socket");
    return response;
  }

  response = ami_add_new_packet_item(response,"Action","StopMonitor");
  if(!response){
    return NULL;
  }
  
  if(ami_sock_writable(sock,AMI_NET_TIMEOUT)){
    ami_sock_send(sock,"Action: StopMonitor\r\nChannel: ");
    ami_sock_send(sock,chan);
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
