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

#include <queue>
#include <QMutex>
#include <QDebug>

#include "commandthread.h"

#define COMMAND_RESPONSE_SIZE 6
#define BUFFER_SIZE 0x100000

namespace {

QMutex* mutex = new QMutex;
const char* const Signal = "Signal";
const char* const BeamOk = "BeamOk";
const char* const BeamNo = "BeamNo";
const char* const Finish = "Finish";

} // namespace

CommandThread::CommandThread(QObject *parent)
    :
    AcquireThread(parent),
    buffer(new char[BUFFER_SIZE])
{
    commands.push(std::string("I000"));
}

CommandThread::~CommandThread()
{
    delete [] buffer;
}

void
CommandThread::writeCommand( const char* cmd, size_t cmd_size)
{
    QMutexLocker locker(mutex);

    commands.push(std::string( cmd, cmd + cmd_size));
}

void
CommandThread::run()
{
    std::list<char> charlist;

    while (true) {
        {
            QMutexLocker locker(mutex);
            if (stopped)
                break;

            if (commands.size()) {
                DWORD towrite, written;
                std::string command_str = commands.front();
                commands.pop();
                towrite = command_str.size();
                char* buff = const_cast< char* >(command_str.data());
                status = FT_Write( device, buff, towrite, &written);
                if (FT_SUCCESS(status) && written == towrite) {
                    qDebug() << "Data written to the FT2232H Channel-A";
                }
            }
        }

        DWORD rx_bytes, nread, toread;
        status = FT_GetQueueStatus( device, &rx_bytes);
        if (FT_SUCCESS(status) && rx_bytes) {
            LPVOID buff = static_cast<LPVOID>(buffer);
            toread = rx_bytes;
            status = FT_Read( device, buff, toread, &nread);
            if (FT_SUCCESS(status) && nread) {
                for ( unsigned int i = 0; i < nread; ++i)
                    charlist.push_back(buffer[i]);

                size_t responses = charlist.size() / COMMAND_RESPONSE_SIZE;

                if (responses) {
                    std::vector<char> ldata;
                    ldata.reserve(COMMAND_RESPONSE_SIZE * responses);
                    for ( size_t i = COMMAND_RESPONSE_SIZE * responses; i != 0; --i) {
                        ldata.push_back(charlist.front());
                        charlist.pop_front();
                    }

                    for ( size_t i = 0; i < responses; ++i) {
                        size_t pbegin = i * COMMAND_RESPONSE_SIZE;
                        size_t pend = (i + 1) * COMMAND_RESPONSE_SIZE;

                        std::string Message( ldata.data() + pbegin, ldata.data() + pend);
                        if (!Message.compare(std::string(Signal))) { // External signal
                            emit signalExternalSignal();
                            qDebug() << "External signal message: " << QString::fromStdString(Message);
                        }
                        else if (!Message.compare(std::string(BeamOk))) { // Slow extraction start -- send signal
                            emit signalNewBatchState(false);
                            qDebug() << "Batch start message: " << QString::fromStdString(Message);
                        }
                        else if (!Message.compare(std::string(BeamNo))) { // Slow extraction finished -- send signal
                            emit signalNewBatchState(true);
                            qDebug() << "Batch finish message: " << QString::fromStdString(Message);
                        }
                        else if (!Message.compare(std::string(Finish))) { // Movement finished -- send signal
                            emit signalMovementFinished();
                            qDebug() << "Movement finish message: " << QString::fromStdString(Message);
                        }
                        else { // if command answer -- send msg to status bar
                            qDebug() << "Command response: " << QString::fromStdString(Message);
                        }
                    }
                }
            }
            else if (!FT_SUCCESS(status)) {
                qDebug() << "failed device status";
                emit signalDeviceError();
                break;
            }
        }
        else if (!FT_SUCCESS(status)) {
            qDebug() << "failed device status";
            emit signalDeviceError();
            break;
        }

    }

    // clear queue
    while (!commands.empty()) commands.pop();

    charlist.clear();

    stopped = false;
}
