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

#include <QTextStream>

#include "serialdevice.h"

namespace {

const char* const OK_string = "OK";
const char* const NO_string = "NO";
const char* const Signal_string = "Signal";
const char* const Accept_string = "Accept";
const char* const Reject_string = "Reject";
const char* const Finish_string = "Finish";
const char* const Carbon_string = "Carbon";

} // namespace

SerialDevice::SerialDevice(QObject *parent)
    :
    QTcpSocket(parent)
{
    connect( this, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect( this, SIGNAL(bytesWritten(qint64)), this, SLOT(onBytesWritten(qint64)));
    connect( this, SIGNAL(readChannelFinished()), this, SLOT(onReadFinished()));
}

SerialDevice::~SerialDevice()
{
}

void
SerialDevice::onReadyRead()
{
    QTextStream output(stdout);
    QByteArray bytes_array = this->readAll();
    output << bytes_array;
    response.append(bytes_array);

    if (response.size() == command.size() + 2) {
        if (response.endsWith(OK_string) && input_buffer.contains(command)) {
            response_ok = true;// response ok
            response.clear();
        }
        else if (response.endsWith(NO_string) && input_buffer.contains(command)) {
            response_ok = false;
            response.clear();
        }
        else if (response == Signal_string) {
            response_ok = true;
            response.clear();
        }
        else if (response == Accept_string) {
            response_ok = true;
            response.clear();
        }
        else if (response == Reject_string) {
            response_ok = true;
            response.clear();
        }
        else if (response == Finish_string) {
            response_ok = true;
            response.clear();
        }
        else if (response == Carbon_string) {
            response_ok = true;
            response.clear();
        }
        else {
            response_ok = false;
            // unknown response
            response.clear();
        }
    }
}

void
SerialDevice::onBytesWritten(qint64 com_size)
{
    if (com_size == command.size()) {
        // command if fully written
    }
}

void
SerialDevice::onReadFinished()
{

}

void
SerialDevice::write_command(const QString &com)
{
    response_ok = false;
    QByteArray current_command;
    current_command.append(com);
    command = current_command;
    this->write(command);
}
