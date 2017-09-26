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

namespace {

QMutex* mutex = new QMutex;
const char* const Carbon = "Carbon";
//const char* const Signal = "Signal";
const char* const Finish = "Finish";
const char* const Spill0 = "Spill0";
const char* const Spill1 = "Spill1";

} // namespace

CommandThread::CommandThread(QObject *parent)
    :
    AcquireThread(parent)
{
    std::vector<char> handshake({ 'I', '0', '0', '0' });
    commands.push(handshake);
}

CommandThread::~CommandThread()
{

}

void
CommandThread::writeCommand( const char* cmd, size_t cmd_size)
{
    QMutexLocker locker(mutex);

    std::vector<char> command( cmd, cmd + cmd_size);
    commands.push(command);
}

void
CommandThread::run()
{
    while (true) {
        {
            QMutexLocker locker(mutex);
            if (stopped)
                break;

            if (commands.size()) {
                DWORD towrite, written;
                std::vector<char> buffer = commands.front();
                commands.pop();
                towrite = buffer.size();
                LPVOID buf = static_cast<LPVOID>(buffer.data());
                status = FT_Write( device, buf, towrite, &written);
                if (FT_SUCCESS(status) && written == towrite) {
                    qDebug() << "Data written to the FT2232H Channel-A";
//                    msleep(100);
                }
            }
        }

        DWORD rx_bytes, nread, toread;
        status = FT_GetQueueStatus( device, &rx_bytes);

        if (FT_SUCCESS(status) && rx_bytes >= COMMAND_RESPONSE_SIZE) {
            std::vector<char> buffer = std::vector<char>(rx_bytes);
            LPVOID buf = static_cast<LPVOID>(buffer.data());
            toread = rx_bytes;
            status = FT_Read( device, buf, toread, &nread);
            if (FT_SUCCESS(status) && nread == toread) {
                std::string Message( buffer.data(), buffer.size());

                if (!Message.compare(std::string(Carbon))) { // Carbon test word
                    qDebug() << "Carbon message: " << QString::fromStdString(Message);
                }
//                else if (!Message.compare(std::string(Signal))) { // if new batch signal -- send signal
//                    emit signalNewBatch();
//                    qDebug() << "Batch signal message: " << QString::fromStdString(Message);
//                }
                else if (!Message.compare(std::string(Finish))) { // if movement finished -- send signal
                    emit signalMovementFinished();
                    qDebug() << "Finish message: " << QString::fromStdString(Message);
                }
                else if (!Message.compare(std::string(Spill0))) { // if new batch state 0 signal -- send signal
                    emit signalNewBatchState(false);
                    qDebug() << "Batch signal state message: " << QString::fromStdString(Message);
                }
                else if (!Message.compare(std::string(Spill1))) { // if new batch state 1 signal -- send signal
                    emit signalNewBatchState(true);
                    qDebug() << "Batch signal state message: " << QString::fromStdString(Message);
                }
                else { // if command answer -- send msg to status bar
                    qDebug() << "Command response: " << QString::fromStdString(Message);
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

    stopped = false;
}
