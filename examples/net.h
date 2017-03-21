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
 *    net.h
 *    Network Functions
/*/

#ifndef _AMP_NET_H
#define _AMP_NET_H

#include <sys/socket.h>

#define NET_WAIT_TIMEOUT_MIL 500             /* 1/2 a second in mili (thousand) seconds */
#define NET_WAIT_TIMEOUT_BIL 750000000       /* 3/4 a second in nano (billion) seconds */

struct client_conn {
  int sock;
  struct sockaddr addr;
};

int sock_connect(char *, int);
int sock_bind(char *, int);
void sock_close(int);

#endif /* _AMP_NET_H */
