/*
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define COMMAND_SIZE 4
#define RESPONSE_SUFFIX_SIZE 2
#define RESPONSE_SIZE (COMMAND_SIZE + RESPONSE_SUFFIX_SIZE)

int port_init( const char *device, int rdrw_flag);
int port_close(int fd);
size_t port_readn( int fd, char* buf, size_t count, int* err);
int port_flush(int fd);
int port_write_command( int fd, const char* command);
int port_handshake(int fd);
int port_reset_altera(int fd);
int port_movement( int fd, int move, int distance);
int port_acquisition_timing( int fd, char delay_time, char acquisition_time);
int port_acquisition_delay( int fd, int delay_time);
int port_pedestal_triggers( int fd, int trigger_code);

#ifdef __cplusplus
}
#endif /* __cplusplus */
