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
 *    channel.h
 *
/*/

#ifndef _AMI_CHANNEL_H
#define _AMI_CHANNEL_H

/*/
 * ast_packet = ami_absolute_timeout(int sock, char channel, long int timeout, actionID)
 * Sets an absolute timeout for a call in seconds. The call will be rudely hung up on when the timeout occurs.
/*/
ast_packet *ami_absolute_timeout(int sock, const char *channel, long int timeout, const char *actionID);

/*/
 * ast_packet = ami_getvar(int sock, char varname, char channel, actionID)
 * Retrieve a variable. If channel is NULL a global variable will be returned, otherwise it will be 
 *  channel specific.
/*/
ast_packet *ami_getvar(int sock, const char *varname, const char *channel, const char *actionID);

/*/
 * ast_packet = ami_setvar(int sock, char varname, char value, char channel, actionID)
 * Set a variable. If channel is NULL a global variable will be set, otherwise it will be channel specific.
/*/
ast_packet *ami_setvar(int sock, const char *varname, const char *value, const char *channel, const char *actionID);

/*/
 *  ast_packet = ami_hangup(int sock, char channel, actionID)
 *  Rudely hang up on a given channel
/*/
ast_packet *ami_hangup(int sock, const char *channel, const char *actionID);

/*/
 *  ast_packet = ami_redirect(int sock, char channel, char context, char exten, int priority, char extraChannel, actionID)
 *  Rudely redirect a call to somewhere else in the dialplan
 *  extraChannel is optional
/*/
ast_packet *ami_redirect(int sock, const char *channel, const char *context, const char *exten, int priority, const char *extraChannel, const char *actionID);

/*/
 * ast_packet = ami_originate(int sock, char channel, char context, char exten, int priority,
                              char app, char data, long int timeout, char callerID, char list variables,
                              char accountCode, short int async, actionID)
 * This function is a bit of a monster. Refer to the asterisk documentation to understand how it works.
 * An online reference is at the asterisk CLI> 'show manager command Originate'
 * It works exactly as described.
 * The only major programming caveot is the variable list. This should be a pointer to a list of pointers of character
 *  strings. It MUST be terminated by a pointer to a NULL string.
/*/
ast_packet *ami_originate(int sock, const char *channel, const char *context, const char *exten, int priority, const char *app, const char *data, long int timeout, const char *callerID, const char **variables, const char *accountCode, short int async, const char *actionID);

/*/
 * ast_packet = ami_monitor(int sock, char channel, char filename, char format, short int combine, actionID)
 * Start monitoring a channel. Only sock and channel are required. If combine is set, the in and out channel
 *  recordings are combined.
/*/
ast_packet *ami_monitor(int sock, const char *channel, const char *filename, const char *format, short int combine, const char *actionID);

/*/
 * ast_packet = ami_monitor_change(int sock, char channel, char file, actionID)
 * Change the monitoring session to a new file
/*/
ast_packet *ami_monitor_change(int sock, const char *channel, const char *file, const char *actionID);

/*/
 * ast_packet = ami_monitor_stop(int sock, char channel, actionID)
 * Stop recording on the given channel
/*/
ast_packet *ami_monitor_stop(int sock, const char *channel, const char *actionID);

#endif /* _AMI_CHANNEL_H */
