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
 *    core.c
 *
/*/

#include "config.h"

#ifdef HAVE_WINSOCK2_H
  #include <winsock2.h>
#else
  #include <sys/socket.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <ami/core.h>

#define AMI_MAX_BUFFER 512

static int ami_sock_readwriteable(int sock, int milliseconds, int read);

/*/
 * ami_strip_internal_action_id_flag(ast_packet packet)
 *  To seperate regular events from events belonging to actions
 *  several libami functions inject a prefix to the ActionID field
 *  This function strips out this prefix so that the original
 *  actionID will be presented to the application
/*/
static void ami_strip_internal_action_id_flag(ast_packet *packet);

static ami_event_cb_func event_cb_func = NULL;
static void* event_cb_data = NULL;

int ami_sock_send(int sock, const char *cmd){
  int i;
  if(sock <= 0 || !cmd || !strlen(cmd)){ return -1; }
  for(i=0;i<strlen(cmd);i++){
    if(ami_sock_writable(sock,AMI_NET_TIMEOUT)){
      if(send(sock,(void *)&cmd[i],sizeof(char) * 1,0) <= 0){
        return -1;
      }
    }
  }
  return 0;
}

char *ami_sock_receive_internal(int sock, int wait_for_an_action_reply){
  char *resp = NULL;
  char *reall = NULL;
  char c;
  int buffLen = 0;
  int i = 0;
  int bytes;
  int total = 0;
  if(sock <= 0){ return NULL; }
  resp = (char *)malloc(sizeof(char) * AMI_MAX_BUFFER);
  buffLen = AMI_MAX_BUFFER;
  if(!resp){ return NULL; }
  resp[0] = '\0';
  while(1){
    switch (ami_sock_readable(sock,250)) {
      case -1:
        free(resp);
        return NULL;
      case 0:
        continue;

      default:
        break;
    }
    bytes = recv(sock,&c,1,0);
    if(bytes <= 0 && total <= 0){
      free(resp);
      return NULL;
    }else if(bytes <= 0){
      break;
    }else{
      total += bytes;
    }
    if(i >= (buffLen - 1)){
      reall = (char *)realloc((void *)resp,(buffLen + AMI_MAX_BUFFER));
      if(!reall){
        free(resp);
        return NULL;
      }
      resp = reall;
      reall = NULL;
      buffLen += AMI_MAX_BUFFER;
    }
    resp[i] = c;
    resp[i+1] = '\0';
    if(c == '\n' && i >= 3){
      if(resp[i-3] == '\r' && resp[i-2] == '\n' && resp[i-1] == '\r' && resp[i] == '\n'){
        resp[i-1] = '\0';
        /* Is this packet an Event packet which isn't part of an action ? */
        if (!strncmp(resp, "Event:", 6)) {
          if (!strstr(resp, "ActionID: ami_")) {
            if (event_cb_func) {
              ast_packet *packet = ami_parse_packet(resp);
              if (!packet) {
#ifdef _AMI_DEBUG
                printf("Unable to parse event: %s\n", resp);
#endif
              } else {
                event_cb_func(packet, event_cb_data);
                ami_destroy_packet(packet);
              }
            }
            free(resp);
            if (!wait_for_an_action_reply) {
              return NULL;
            }

            i = 0;
            resp = (char *)malloc(sizeof(char) * AMI_MAX_BUFFER);
            buffLen = AMI_MAX_BUFFER;
            if(!resp){ return NULL; }
            resp[0] = '\0';
            continue;
          }
        }
        break;
      }
    } else if (i == 1) {
      if (resp[i-1] == '\r' && resp[i] == '\n') {
        resp[0] = '\0';
        break;
      }
    }
    i++;
  }
  if(!strlen(resp)){
    free(resp);
    return NULL;
  }
  return resp;
}

char *ami_sock_receive(int sock){
  return ami_sock_receive_internal(sock, 1);
}

void ami_sock_receive_event(int sock){
  ami_sock_receive_internal(sock, 0);
}

int ami_sock_readable(int sock, int milliseconds){
 return ami_sock_readwriteable(sock,milliseconds,1);
}

int ami_sock_writable(int sock, int milliseconds){
  return ami_sock_readwriteable(sock,milliseconds,0);
}

static int ami_sock_readwriteable(int sock, int milliseconds, int read){
  struct timeval timeout;
  int seconds = 0;
  int mseconds = 0;
  fd_set set;
  int ret;
  if(!sock){ return -1; }
  if(milliseconds >= 1000){
    seconds = (milliseconds / 1000);
    mseconds = (milliseconds - (seconds * 1000));
  }else if(milliseconds >= 0){
    mseconds = milliseconds;
  }
  timeout.tv_sec = seconds;
  timeout.tv_usec = (mseconds * 1000);
  FD_ZERO(&set);
  FD_SET(sock,&set);
  if(read){
    if(milliseconds >= 0){
      ret = select(FD_SETSIZE,&set,NULL,NULL,&timeout);
    }else{
      ret = select(FD_SETSIZE,&set,NULL,NULL,NULL);
    }
  }else{
    if(milliseconds >= 0){
      ret = select(FD_SETSIZE,NULL,&set,NULL,&timeout);
    }else{
      ret = select(FD_SETSIZE,NULL,&set,NULL,NULL);
    }
  }
  return ret;
}

void ami_set_event_cb(ami_event_cb_func func, void* cb_data){
  event_cb_func = func;
  event_cb_data = cb_data;
}

ast_packet *ami_parse_packet(const char *packet_str){
  int i = 0;
  int a = 0;
  int isname = 1;
  char name[MAX_AST_PACKET_ITEM_NAME_SIZE];
  char val[MAX_AST_PACKET_ITEM_VALUE_SIZE];
  ast_packet *p = NULL;
  ast_packet *check = NULL;
  if(!packet_str || !strlen(packet_str)){ return NULL; }
  for(i=0;i<strlen(packet_str);i++){
    if(packet_str[i] == ':' && isname == 1){
      isname = 0;
      name[a] = '\0';
      a = 0;
      i++;
      continue;
    }
    if(packet_str[i] == '\r' && packet_str[i+1] == '\n'){
      isname = 1;
      val[a] = '\0';
      a = 0;
      i++;
      if(strlen(name)){
        check = ami_add_new_packet_item(p,name,val);
      }
      name[0] = '\0';
      val[0] = '\0';
      if(!check){
        ami_destroy_packet(p);
        return NULL;
      }
      if(!p){
        p = check;
      }
      continue;
    }
    if(isname){
      if(a >= MAX_AST_PACKET_ITEM_NAME_SIZE){
        if(p){ ami_destroy_packet(p); }
        return NULL;
      }
      name[a] = packet_str[i];
      name[a+1] = '\0';
    }else{
      if(a >= MAX_AST_PACKET_ITEM_VALUE_SIZE){
        if(p){ ami_destroy_packet(p); }
        return NULL;
      }
      val[a] = packet_str[i];
      val[a+1] = '\0';
    }
    a++;
  }

  ami_strip_internal_action_id_flag(p);

  return p;
}

ast_packet *ami_create_packet(){
  ast_packet *p;
  p = (ast_packet *)malloc(sizeof(ast_packet));
  if(!p){ return NULL; }
  p->first_item = NULL;
  p->next = NULL;
  return p;
}

ast_packet_item *ami_create_packet_item(const char *name, const char *val){
  ast_packet_item *pi;
  if(!name || !strlen(name) || strlen(name) >= MAX_AST_PACKET_ITEM_NAME_SIZE){ return NULL; }
  if(val && strlen(val) >= MAX_AST_PACKET_ITEM_VALUE_SIZE){ return NULL; }
#ifdef _AMI_DEBUG
  printf("Creating packet name %s - val %s\n",name,val);
#endif
  pi = (ast_packet_item *)malloc(sizeof(ast_packet_item));
  if(!pi){ return NULL; }
  strncpy(pi->name,name,MAX_AST_PACKET_ITEM_NAME_SIZE);
  if(!val || !strlen(val)){
    strcpy(pi->value,"");
  }else{
    strncpy(pi->value,val,MAX_AST_PACKET_ITEM_VALUE_SIZE);
  }
  pi->next = NULL;
  return pi;
}

ast_packet_item *ami_add_packet_item(ast_packet_item *p, ast_packet_item *pi){
  ast_packet_item *pptr;
  if(!p || !pi){ return NULL; }
  pptr = p;
  while(pptr->next){ pptr = pptr->next; }
  pptr->next = pi;
  return pi;
}

ast_packet *ami_add_packet_item_to_packet(ast_packet *p, ast_packet_item *pi){
  ast_packet_item *pptr;
  if(!p || !pi){ return NULL; }
  if(p->first_item){
    pptr = p->first_item;
    while(pptr->next){ pptr = pptr->next; }
    pptr->next = pi;
  }else{
    p->first_item = pi;
  }
  return p;
}

ast_packet *ami_add_new_packet_item(ast_packet *p, const char *key, const char *val){
  ast_packet_item *pi;
  ast_packet_item *check;
  short int new_p = 0;
  if(!key || !strlen(key)){ return NULL; }
#ifdef _AMI_DEBUG
  printf("Adding new packet item: %s -> %s\n",key,val);
#endif
  if(!p){
    p = (ast_packet *)malloc(sizeof(ast_packet));
    p->first_item = NULL;
    p->next = NULL;
    new_p = 1;
    if(!p){
      return NULL;
    }
  }
  if(val){
    pi = ami_create_packet_item(key,val);
  }else{
    pi = ami_create_packet_item(key,NULL);
  }
  if(!pi){
    if(new_p){ free(p); }
    return NULL;
  }
  if(!p->first_item){
    p->first_item = pi;
  }else{
    check = ami_add_packet_item(p->first_item,pi);
  }
  return p;
}

ast_packet *ami_add_packet(ast_packet *pg, ast_packet *p){
  ast_packet *pptr;
  if(!pg || !p){ return NULL; }
  pptr = pg;
  while(pptr->next){ pptr = pptr->next; }
  pptr->next = p;
  return pg;
}

void ami_destroy_packet(ast_packet *p){
  ast_packet_item *pi;
  ast_packet_item *tmp_pi;
  if(!p){ return; }
  pi = p->first_item;
  while(pi){
    tmp_pi = pi->next;
    free(pi);
    pi = tmp_pi;
  }
  free(p);
  p = NULL;
}

void ami_destroy_packet_item(ast_packet *p, ast_packet_item *pi){
  if(!p || !p->first_item || !pi){ return; }
  ami_unlink_packet_item(p,pi);
  free(pi);
  pi = NULL;
}

void ami_destroy_packet_item_byname(ast_packet *p, const char *name){
  ast_packet_item *pi;
  if(!p || !p->first_item || !name || !strlen(name)){ return; }
  pi = p->first_item;
  while(pi){
    if(!strcasecmp(pi->name,name)){
      ami_destroy_packet_item(p,pi);
      break;
    }
    pi = pi->next;
  }
}

void ami_unlink_packet_item(ast_packet *p, ast_packet_item *pi){
  ast_packet_item *iptr;
  if(!p || !p->first_item || !pi){ return; }
  if(p->first_item == pi){
#ifdef _AMI_DEBUG
    printf("Unlink: Making first packet item link to pi->next\n");
    if(pi->next){ printf("Unlink: Which is %s\n",pi->next->name); }else{ printf("Unlink: Which is NULL\n"); }
#endif
    p->first_item = pi->next;
    pi->next = NULL;
  }else{
    iptr = p->first_item;
    while(iptr->next){
      if(iptr->next == pi){
#ifdef _AMI_DEBUG
        printf("Unlink: Making %s->next == %s->next\n",iptr->name,pi->name);
        if(pi->next){ printf("Unlink: Which is %s\n",pi->next->name); }else{ printf("Unlink: Which is NULL\n"); }
#endif
        iptr->next = pi->next;
        pi->next = NULL;
        break;
      }
      iptr = iptr->next;
    }
  }
}

void ami_merge_packets(ast_packet *p1, ast_packet *p2){
  ast_packet_item *iptr;
  ast_packet_item *tmp_pi;
  if(!p1 || !p2){ return; }
  iptr = p2->first_item;
  while(iptr){
#ifdef _AMI_DEBUG
    printf("Merge: Working with %s\n",iptr->name);
#endif
    tmp_pi = iptr->next;
#ifdef _AMI_DEBUG
    printf("Merge: Set tmp ptr to %p %s\n",tmp_pi, tmp_pi->name);
#endif
    ami_add_packet_item(p1->first_item,iptr);
    ami_unlink_packet_item(p2,iptr);
    iptr = tmp_pi;
#ifdef _AMI_DEBUG
    printf("Merge: Set iteration to %s\n",iptr->name);
#endif
  }
  ami_destroy_packet(p2);
  p2 = NULL;
}

void ami_destroy_packet_group(ast_packet *first_packet){
  ast_packet *pi;
  ast_packet *tmp_pi;
  pi = first_packet;
  while(pi){
    tmp_pi = pi->next;
    ami_destroy_packet(pi);
    pi = NULL;
    pi = tmp_pi;
  }
}

ast_packet *ami_duplicate_packet(ast_packet *p){
  ast_packet *firstp = NULL;
  ast_packet *np = NULL;
  ast_packet *npptr = NULL;
  ast_packet *pptr;
  ast_packet_item *iptr;
  if(!p){ return NULL; }
  if(!p->first_item){ return NULL; }
  pptr = p;
  while(pptr){
    iptr = pptr->first_item;
    while(iptr){
      np = ami_add_new_packet_item(np,iptr->name,iptr->value);
      iptr = iptr->next;
    }
    if(!firstp){
      firstp = np;
      npptr = firstp;
    }else{
      npptr->next = np;
      npptr = npptr->next;
    }
    pptr = pptr->next;
    np = NULL;
  }
  return firstp;
}

char *ami_get_packet_item_value(ast_packet *p, const char *key){
  ast_packet_item *pi;
  if(!p || !p->first_item || !key || !strlen(key)){ return NULL; }
  pi = p->first_item;
  while(pi){
    if(pi->name && strlen(pi->name) && !strcasecmp(pi->name,key)){
      return pi->value;
    }
    pi = pi->next;
  }
  return NULL;
}

ast_packet_item *ami_get_packet_item(ast_packet *p, const char *key){
  ast_packet_item *pi;
  if(!p || !p->first_item || !key || !strlen(key)){ return NULL; }
  pi = p->first_item;
  while(pi){
    if(pi->name && strlen(pi->name) && !strcasecmp(pi->name,key)){
      return pi;
    }
    pi = pi->next;
  }
  return NULL;
}

static void ami_strip_internal_action_id_flag(ast_packet *packet){
  char *val = ami_get_packet_item_value(packet, "ActionID");

  if (!val) {
    return;
  }

  if (!strncmp(val, "ami_", 4)){
    memmove(val, val + 4, strlen(val) - 4 + 1);   /* The +1 is for the terminating \0 */
    if (strlen(val) == 0){
      ami_destroy_packet_item_byname(packet, "ActionID");
    }
  }
}

void ami_ltrim(char *str){
  int i = 0;
  int wEnd = 0;
  while(str[i] == ' ' || str[i] == '\t'){ i++; }
  if(i){
    wEnd = i;
    for(i=wEnd;i<=strlen(str);i++){
      str[i-wEnd] = str[i];
    }
  }
}


void ami_rtrim(char *str){
  int i;
  i = (strlen(str) - 1);
  while(str[i] == ' ' || str[i] == '\t'){ i--;}
  str[i+1] = '\0';
}


void ami_trim(char *str){
  ami_ltrim(str);
  ami_rtrim(str);
}

void ami_chomp(char *str){
  int i;
  if(!str || !strlen(str)){ return; }
  for(i=(strlen(str)-1);i>=0;i--){
    if(str[i] == '\n' || str[i] == '\r'){
      str[i] = '\0';
    }else{
      break;
    }
  }
}
