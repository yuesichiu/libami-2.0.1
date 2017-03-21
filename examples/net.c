/*/
 *  Copyright © Justin Camp 2006
 *
 *  This file is part of Asterisk Manager Proxy (AMP).
 *
 *  Asterisk Manager Proxy is free software; you can redistribute it 
 *  and/or modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2 of 
 *  the License, or (at your option) any later version.
 *
 *  Asterisk Manager Proxy is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Asterisk Manager Proxy; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *  Send bugs, patches, comments, flames, the phone number of a very lonely
 *  and very ritch supermodel or any other ideas or information worth
 *  communicating to j@intuitivecreations.com
/*/

/*/
 *    net.c
 *    Network Functions
/*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
//#include <libxml/parser.h>

#include "ami/ami.h"
#include "net.h"

/*
#include "utils.h"
#include "log.h"
#include "thread.h"
#include "configuration.h"
#include "client.h"
#include "event.h"
#include "xml.h"
#include "status.h"
*/


#define AMP_NET_MAX_BUFFER 256

/*
static int check_agent_event(proxy_thread *pt, ast_event *e);
static int compare_agent(const char *needle, const char *agent);
*/

int sock_connect(char *host, int port){
  int sock;
  struct hostent *h;
  struct sockaddr_in saddr;
  int flags = 0;
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(port);
  if(!inet_aton(host,&saddr.sin_addr)){
    h = gethostbyname(host);
    if(!h){
      //amp_log(LOG_ERROR,"Unable to look up hostname %s: %s",host,strerror(errno));
      return -1;
    }
    //amp_log(LOG_DEBUG,"Found host %s",host);
    memcpy(&saddr.sin_addr,h->h_addr,sizeof(saddr.sin_addr));
  }
  sock = socket(PF_INET,SOCK_STREAM,0);
  if(sock <= 0){
    //amp_log(LOG_ERROR,"Unable to open a socket to host %s: %s",host,strerror(errno));
    return -1;
  }
  if(connect(sock,(struct sockaddr *)&saddr,sizeof(saddr)) < 0){
    //amp_log(LOG_ERROR,"Unable to connect to host %s: %s",host,strerror(errno));
    shutdown(sock,2);
    return -1;
  }
  fcntl(sock,F_SETFL,flags|O_NONBLOCK);
  //amp_log(LOG_DEBUG,"Connected to host %s",host);
  return sock;
}

int sock_bind(char *host, int port){
  int sock;
  struct hostent *h;
  struct sockaddr_in saddr;
  int flags;
  int so_reuse_opt = 1;
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(port);
  if(!strcmp(host,"*")){
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
  }else{
    if(!inet_aton(host,&saddr.sin_addr)){
      h = gethostbyname(host);
      if(!h){
        //amp_log(LOG_ERROR,"Unable to look up hostname %s: %s",host,strerror(errno));
        return -1;
      }
      //amp_log(LOG_DEBUG,"Found host %s",host);
      memcpy(&saddr.sin_addr,h->h_addr,sizeof(saddr.sin_addr));
    }
  }
  sock = socket(PF_INET,SOCK_STREAM,0);
  if(sock <= 0){
    //amp_log(LOG_ERROR,"Unable to open a socket to host %s: %s",host,strerror(errno));
    return -1;
  }
  flags = fcntl(sock,F_GETFL,0);
  if(fcntl(sock,F_SETFL,flags|O_NONBLOCK) != 0){
    //amp_log(LOG_ERROR,"Error setting sock to non-block: %s",strerror(errno));
  }
  setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&so_reuse_opt,sizeof(so_reuse_opt));
  if(bind(sock,(struct sockaddr *)&saddr,sizeof(saddr)) < 0){
    //amp_log(LOG_ERROR,"Unable to connect to host %s: %s",host,strerror(errno));
    shutdown(sock,2);
    return -1;
  }
  //amp_log(LOG_INFO,"Listening on host %s:%i",host,port);
  return sock;
}

void sock_close(int sock){
  struct sockaddr_in saddr;
  socklen_t size;
  char *addr;
  if(sock <= 0){ return; }
  size = sizeof(saddr);
  if(getpeername(sock,(struct sockaddr *)&saddr,&size) > -1){
    addr = inet_ntoa(saddr.sin_addr);
    //amp_log(LOG_DEBUG,"Closing connection %s (sock #%i)",inet_ntoa(saddr.sin_addr),sock);
  }else{
    //amp_log(LOG_DEBUG,"Closing connection [unknown]");
  }
  shutdown(sock,2);
}


