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
 *    admin.h
 *
/*/

#ifndef _AMI_ADMIN_H
#define _AMI_ADMIN_H

#include <ami/core.h>

/*/
 * ast_packet = ami_dbshow(int sock, actionID)
 * Returns an ast_packet containing a response and every key->value pair in the 
 *  asterisk database
/*/
ast_packet *ami_dbshow(int sock, const char *actionID);

/*/
 * ast_packet = ami_dbget(int sock, char *family, char *key, actionID)
 * Returns an ast_packet containing the value of the specified key within the specified family
 * Note: Do not put the initial '/' before the family key. This is how asterisk wants it given:
 * i.e.  /Agents/501/whatever would be specified as ami_dbget(sock,"Agents/501","whatever")
 * You are not stopped from putting a '/' in the beginning as some existing keys (namely Agents) begin with a 
 *  double'/' (i.e. //Agents/501), simply beware, asterisk assumes an initial '/'
/*/
ast_packet *ami_dbget(int sock, const char *family, const char *key, const char *actionID);

/*/
 * ast_packet = ami_dbput(int sock, char *family, char *key, char *value, actionID)
 * Creates or updates the value of the specified key within the specified family
 * Note: Do not put the initial '/' before the family key.
/*/
ast_packet *ami_dbput(int sock, const char *family, const char *key, const char *value, const char *actionID);

/*/
 * ast_packet = ami_dbdel(int sock, char *family, char *key, actionID)
 * Deletes a key->value pair from the asterisk database
 * Note: Do not put the initial '/' before the family key.
/*/
ast_packet *ami_dbdel(int sock, const char *family, const char *key, const char *actionID);

/*/
 * ast_packet = ami_dbdeltree(int sock, char *family, char *key, actionID)
 * Deletes a family/key tree from the database. Key can optionally be NULL.
 * Note: Do not put the initial '/' before the family key.
/*/
ast_packet *ami_dbdeltree(int sock, const char *family, const char *key, const char *actionID);

/*/
 * ast_packet = ami_queue_add(int sock, char interface, char queue, actionID)
 * Adds an agent interface to a queue
 * Note: interface is the full interface (i.e. "Agent/500")
/*/
ast_packet *ami_queue_add(int sock, const char *interface, const char *queue, const char *actionID);

/*/
 * ast_packet = ami_queue_remove(int sock, char interface, char queue, actionID)
 * Removes an agent interface from a queue
 * Note: interface is the full interface (i.e. "Agent/500")
/*/
ast_packet *ami_queue_remove(int sock, const char *interface, const char *queue, const char *actionID);

/*/
 * ast_packet = ami_queue_pause(int sock, char interface, char queue, short int paused, actionID)
 * Pauses/Unpauses an interface. If Pause is omitted, TRUE is assumed. If queue is omitted, the pause/unpause
 *   effects the interface globally (across all queues)
 * Note: interface is the full interface (i.e. "Agent/500")
/*/
ast_packet *ami_queue_pause(int sock, const char *interface, const char *queue, short int paused, const char *actionID);

/*/
 * ast_packet = ami_agent_callback_login(int sock, int agentID, char exten, char context, int ackCall, int wrapUpTime, actionID)
 * Login an agent in for callback. Only sock, agentID and exten are required.
 * Note: interface is only the ID of the agent
/*/
ast_packet *ami_agent_callback_login(int sock, int agentID, const char *exten, const char *context, short int ackCall, int wrapUpTime, const char *actionID);

/*/
 * ast_packet = ami_agent_logoff(int sock, int agentID, short int softhangup, actionID)
 * Logs off an agent. If softhangup is true, existing calls will not be hung up on.
 * Note: interface is only the ID of the agent
/*/
ast_packet *ami_agent_logoff(int sock, int agentID, short int softhangup, const char *actionID);

/*/
 * ast_packet = ami_command(int sock, char command, actionID)
 * Sends a command to the asterisk manager interface exactly as supplied and returns the response verbatim
 * Some caveots exist to this function: the packet will always return successful, even if an error exists
 *  within the content. Also, the content is returned in "chunks", each in it's own ast_packet_item. It's the 
 *  programmer's responsability to peice the chunks together to resemble the original output.
/*/
ast_packet *ami_command(int sock, const char *command, const char *actionID);

/*/
 * ast_packet = ami_custom(int sock, ast_packet *cmd)
 * Sends the custom command exactly as supplied in the args ast_packet and expects the custom module to respond
 * using asterisk native format.
/*/
ast_packet *ami_custom(int sock, ast_packet *cmd);

/*/
 * ast_packet = ami_custom_free_form(int sock, ast_packet *cmd)
 * Sends the custom command exactly as supplied in the args ast_packet and expects a free-form string as a response.
 * The result is given in the chunk# method exactly like the output of ami_command().
/*/
ast_packet *ami_custom_free_form(int sock, ast_packet *cmd);

#endif /* _AMI_ADMIN_H */
