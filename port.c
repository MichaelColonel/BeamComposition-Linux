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

#include <unistd.h>      /* UNIX standard function definitions */
#include <sys/ioctl.h>   /* File control definitions */
#include <fcntl.h>       /* File control definitions */
#include <errno.h>       /* Error number definitions */
#include <termios.h>     /* POSIX terminal control definitions */

#include <stdio.h>       /* Standard input/output definitions */
#include <stdlib.h>      /* Standard library definitions */
#include <string.h>      /* String function definitions */

#include "port.h"

static struct termios newio, oldio;
static const char* const CommandHandshake_string = "I000";
static const char* const CommandResetAltera_string = "R000";

static int port_check(int fd);

int
port_init( const char *device, int rdrw_flag)
{
    int fd;

    fd = open( device, rdrw_flag | O_NOCTTY | O_NDELAY);

    if (fd == -1) {
        fprintf( stderr, "unable to open device: %s\n", device);
        fprintf( stderr, "%s\n", strerror(errno));
        return -1;
    }
/*    else {
        printf("device %s has been opened successfully\n", device);
        if (fcntl( fd, F_SETFL, 0) == -1) {
            fprintf( stderr, "%s\n", strerror(errno));
            return -1;
        }
    }
*/
    // get the current options for the port
    if (tcgetattr( fd, &oldio) == -1) {
        fprintf( stderr, "error getting device attributes: %s\n",
                 strerror(errno));
        return -1;
    }
    memcpy( &newio, &oldio, sizeof(struct termios));

    // set the baud rates to B19200
    if (cfsetispeed( &newio, B19200) == -1) {
        fprintf( stderr, "error setting device input baud rate: %s\n",
                 strerror(errno));
        return -1;
    }
    if (cfsetospeed( &newio, B19200) == -1) {
        fprintf( stderr, "error setting device output baud rate: %s\n",
                 strerror(errno));
        return -1;
    }
    // enable the receiver and set local mode
    newio.c_cflag |= (CLOCAL | CREAD);

    // size
    // mask the character size bits
    newio.c_cflag &= ~CSIZE;
    // select 8 data bits
    newio.c_cflag |= CS8;

    // raw input
    newio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    // raw output
    newio.c_oflag &= ~OPOST;
    // disable hardware flow control (Also called CRTSCTS)
    newio.c_cflag &= ~CRTSCTS;

    // skip parity check
    newio.c_cflag &= ~PARENB;
    newio.c_cflag &= ~CSTOPB;
    // disable software flow control
    newio.c_cflag &= ~(IXON | IXOFF | IXANY);
    // ignore input parity
    newio.c_iflag |= IGNPAR;
    // 1 character output, timeout 1 sec
    newio.c_cc[VMIN] = 1;
    newio.c_cc[VTIME] = 1;

    // set the new options for the port
    if (tcsetattr( fd, TCSANOW, &newio) == -1) {
        fprintf( stderr, "error setting new device attributes: %s\n",
                 strerror(errno));
        return -1;
    }

    return fd;
}

int
port_close(int fd)
{
    if (tcflush( fd, TCIOFLUSH) == -1) {
        fprintf( stderr, "error flushing device: %s\n", strerror(errno));
        return -1;
    }
    if (tcsetattr( fd, TCSANOW, &oldio) == -1) {
        fprintf( stderr, "error restoring device attributes: %s\n",
                 strerror(errno));
        return -1;
    }

    if (close(fd) == -1) {
        fprintf( stderr, "error closing device: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

int
port_flush(int fd)
{
    if (tcflush( fd, TCOFLUSH) == -1) {
        fprintf( stderr, "error flushing device: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

size_t
port_readn( int fd, char* buf, size_t count, int* err)
{
    size_t nleft = count;
    size_t bytes_read = 0;
    char* ptr = buf;

    ssize_t nread;

    *err = 0;
    while (nleft > 0) {
        if (!port_check(fd)) {
            if ((nread = read( fd, ptr, nleft)) > 0) {
                bytes_read += nread;
                nleft -= nread;
                ptr += nread;
            }
            else if (nread == -1) {
                *err = errno;
                fprintf( stderr, "error reading from device: %s.\n", strerror(errno));
                break;
            }
        }
        else {
            *err = 1;
            break;
        }
    }

    return bytes_read;
}

int
port_check(int fd)
{
    fd_set rfds;
    struct timeval tv;
    int retval;

    /* Watch fd to see when it has input. */
    FD_ZERO(&rfds);
    FD_SET( fd, &rfds);

    /* Wait up to one second. */
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    retval = select( fd + 1, &rfds, NULL, NULL, &tv);
    /* Don't rely on the value of tv now! */

    if (retval == -1) {
        perror("select()");
        return -1;
    }
    else if (retval) {
        /* printf("Data is available now.\n"); */
        /* FD_ISSET(0, &rfds) will be true. */
        return 0;
    }
    else {
        fprintf( stderr, "Timeout warning. No data within one second waiting interval!\n");
        return 1;
    }
    return 0;
}

int
port_write_command( int fd,  const char* command)
{
    size_t cmdlen = strlen(command);
    ssize_t res = write( fd, command, cmdlen);
    if (res != -1 && res == (ssize_t)cmdlen)
        return 0;
    else
        return -1;
}

int
port_handshake(int fd)
{
    return port_write_command( fd, CommandHandshake_string);
}

int
port_reset_altera(int fd)
{
    return port_write_command( fd, CommandResetAltera_string);
}
