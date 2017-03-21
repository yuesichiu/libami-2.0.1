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
 *    info.h
 *
/*/

#ifndef _AMI_INFO_H
#define _AMI_INFO_H

/*/
 * ast_packet = ami_server_version(int sock, char actionID)
 * Retrieve the version of the asterisk server 
/*/
ast_packet *ami_server_version(int sock, const char *command, const char *actionID);

/*/
 * ast_packet = ami_sip_peers(int sock, char actionID)
 * Retrieve a list of all sip peers
/*/
ast_packet *ami_sip_peers(int sock, const char *actionID);

/*/
 * ast_packet = ami_sip_show_peer(int sock, char peer, char actionID)
 * Retrieve information about a specific sip peer
/*/
ast_packet *ami_sip_show_peer(int sock, const char *peer, const char *actionID);

/*/
 * ast_packet = ami_agents(int sock, char actionID)
 * Retrieve a list of all agents
/*/
ast_packet *ami_agents(int sock, const char *actionID);

/*/
 * ast_packet = ami_parked_calls(int sock, char actionID)
 * Retrieve a list of all parked calls
/*/
ast_packet *ami_parked_calls(int sock, const char *actionID);

/*/
 * ast_packet = ami_channels(int sock, char actionID)
 * Retrieve a list of all channels
/*/
ast_packet *ami_channels(int sock, const char *actionID);

/*/
 * ast_packet = ami_extension_state(int sock, char exten, char context, char actionID)
 * Retrieve the status of a device on a given extension within the specified context
/*/
ast_packet *ami_extension_state(int sock, const char *exten, const char *context, const char *actionID);

/*/
 * ast_packet = ami_mailbox_count(int sock, int mailbox, char context, char actionID)
 * Retrieve a count of new and old messages for a given mailbox in the specified context
/*/
ast_packet *ami_mailbox_count(int sock, int mailbox, const char *context, const char *actionID);

/*/
 * ast_packet = ami_mailbox_status(int sock, int mailbox, char context, char actionID)
 * Retrieve a status (how many messages not listened to) for a mailbox in a specified context
/*/
ast_packet *ami_mailbox_status(int sock, int mailbox, const char *context, const char *actionID);

/*/
 * ast_packet = ami_queue_status(int sock, char queue, char actionID)
 * Retrieve status information for queues. If queue is NULL information for all queues will be returned, otherwise
 *   information only for the specified queue will be returned.
/*/
ast_packet *ami_queue_status(int sock, const char *queue, const char *actionID);

#endif /* _AMI_INFO_H */
