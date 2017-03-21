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
 *    manager.h
 *
/*/

#ifndef _AMI_MANAGER_H
#define _AMI_MANAGER_H

#include <ami/core.h>

/*/
 * ast_packet = ami_login(int sock, char username, char secret, char events, char actionID)
 * Logs a connection in to the asterisk manager interface
 * If events are NULL events will be sent to "Off"
/*/
ast_packet *ami_login(int sock, const char *username, const char *secret, const char *events, const char *actionID);

/*/
 * ami_logoff(int sock, char actionID)
 * Logs user off the asterisk manager interface. Returns nothing. The socket will be closed by the server.
 * Note: This function is basically useless due to asterisk automatically logging you out when the connection
 *       to the server is terminated. It is included for completeness.
/*/
void ami_logoff(int sock, const char *actionID);

/*/
 * short int ami_set_events(int sock, int system, int call, int log, int verbose, int command, int agent, int user, char actionID)
 * Set what events you wish to receive from asterisk
 * If all are false, all events are turned off
/*/
short int ami_set_events(int sock, short int system, short int call, short int log, short int verbose, short int command, short int agent, short int user, const char *actionID);

/*/
 * short int = ami_ping(int sock, char actionID)
 * Pings the connection to keep the connection alive
 * If the ping is successful, 1 is returned, otherwise 0 is returned
/*/
short int ami_ping(int sock, const char *actionID);

/*/
 * ast_packet = ami_list_commands(int sock, char actionID)
 * List the commands available natively from the asterisk manager interface
/*/
ast_packet *ami_list_commands(int sock, const char *actionID);

#endif /* _AMI_MANAGER_H */
