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

#include <QFileInfo>
#include <QWaitCondition>
#include <QMutex>
#include <QDebug>

#include <TH1.h>
#include <TH2.h>

#include <fstream>

#include "runinfo.h"
#include "channelscountsfit.h"
#include "diagramtreewidgetaction.h"
#include "rundetailslistwidgetitem.h"

#include "acquisitionthread.h"

#define MASK_BITS 0x03 // mask
#define BUFFER_SIZE 0x100000 // 1 Megabyte

namespace {

/*
// lambda to check mask within the sequence of raw buffer data
std::function<bool( quint8, quint8)>
check_mask = [] ( quint8 v, quint8 m) {
    return !((v & MASK_BITS) ^ m);
};
*/

// function to check mask within the sequence of raw buffer data
bool
check_mask( quint8 v, quint8 m)
{
    return !((v & MASK_BITS) ^ m);
}

// mask buffer of the batch (high byte, low byte)
const DataVector mask{ // first two low bits
    0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x03
};

QWaitCondition cond_acquire;
QMutex* mutex = new QMutex;
quint8 buffer[BUFFER_SIZE];
DataQueue queue;

} // namespace

AcquireThread::AcquireThread(QObject* parent)
    :
    QThread(parent),
    stopped(false),
    device(nullptr)
{
}

AcquireThread::~AcquireThread()
{
}

void
AcquireThread::stop()
{
    QMutexLocker locker(mutex);
    stopped = true;
}

void
AcquireThread::run()
{
    DWORD rx_bytes, nread, toread;

    status = FT_Purge( device, FT_PURGE_TX | FT_PURGE_RX);
    if (!FT_SUCCESS(status)) {
        qDebug() << "Data Acquisition Thread Error: unable to purge device buffers";
        emit signalDeviceError();
        return;
    }

    while (true) {
        status = FT_GetQueueStatus( device, &rx_bytes);
        {
            QMutexLocker locker(mutex);
            if (stopped)
                break;
        }

        if (FT_SUCCESS(status) && rx_bytes > mask.size()) {

            toread = (rx_bytes > BUFFER_SIZE) ? BUFFER_SIZE : rx_bytes;

            status = FT_Read( device, buffer, toread, &nread);
            if (FT_SUCCESS(status) && nread > mask.size()) {
                // put available data in the queue
                // acquire condition signal for processing thread
                DataVector localdata( buffer, buffer + nread);
                {
//                    qDebug() << "Data Acquisition Thread: signal data is ready";
                    QMutexLocker locker(mutex);
                    queue.push(std::move(localdata));
                    cond_acquire.wakeOne();
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
    stopped = false;
}

ProcessThread::ProcessThread(QObject* parent)
    :
    QThread(parent),
    stopped(false),
    flag_background(false)
{
    counts.reserve(BUFFER_SIZE);
    bufferdata.reserve(BUFFER_SIZE * mask.size());
}

ProcessThread::~ProcessThread()
{
}

void
ProcessThread::stop()
{
    QMutexLocker locker(mutex);
    stopped = true;
    cond_acquire.wakeOne();
}

void
ProcessThread::run()
{
    // background data
    SignalArray back;
    size_t n = 0;

    // fitting parameters
    SharedFitParameters params = FitParameters::instance();

    while (true) {
        // local raw data
        DataVector localdata;

        // wait until there are some data in the buffer
        {
            QMutexLocker locker(mutex);
            while(!stopped && queue.empty()) {
                cond_acquire.wait(mutex);
            }

            if (stopped) {
                break;
            }

            localdata = std::move(queue.front());
            queue.pop();
            for ( quint8 v : localdata)
                bufferdata.append(v);
        }

        // process the raw data to obtain ADC counts
        size_t proc = 0; // number of processed buffer data
        CountsList localcounts; // processed ADC counts list
        size_t res = process_data( localcounts, localdata, proc);
        if (res) {
            QMutexLocker locker(mutex);
            counts << localcounts;
        }

        if (res && flag_background)
            fill_background( back, localcounts, n);
    }

    // final background data (if it was any data obtained)
    if (n && flag_background) {
        for ( auto& p : back) {
            p.second = sqrt(p.second);
        }
        params->background() = back;
    }

    // clear queue
    while (!queue.empty()) queue.pop();
    // clear data
    bufferdata.clear();
    counts.clear();

    stopped = false;
}

size_t
ProcessThread::process_data( CountsList& list, DataVector& data, size_t& proc) const
{
    size_t res = 0;
/*
    quint8 mb = MASK_BITS;

    // lambda to check mask within the sequence of raw buffer data
    auto check_mask = [mb] ( quint8 v, quint8 m) -> bool {
        return !((v & mb) ^ m);
    };
*/
    auto it = data.begin(); // DataVector::iterator
    while (it != data.end()) {
        it = std::search( it, data.end(), mask.begin(), mask.end(), check_mask);
        if (it != data.end()) {
            DataVector batch( it, it + mask.size());
            batch_to_counts( list, batch);

            it += mask.size();
            res = it - data.begin();
            proc += mask.size();
        }
    }

    return res;
}

void
ProcessThread::fill_background( SignalArray& back, const CountsList& lcounts, size_t& n)
{
    for ( const CountsArray& ch : lcounts) {
        n++;
        for ( int i = 0; i < CHANNELS; ++i) {
            SignalPair& c = back[i];
            double& mean = c.first;
            double& sigma = c.second;
            mean += (ch[i] - mean) / n;
            sigma += ((ch[i] - mean) * (ch[i] - mean) - sigma) / n;
        }
    }
}

void
ProcessThread::getProcessedData( DataList &datalist, CountsList &countslist)
{
    QMutexLocker locker(mutex);
    countslist = std::move(counts);
    datalist = std::move(bufferdata);
}

ProcessFileThread::ProcessFileThread(QObject *parent)
    :
    ProcessThread(parent),
    process_type(PROCESS_FILE)
{
}

ProcessFileThread::~ProcessFileThread()
{
//    for (RunDetailsListWidgetItem* item : batches)
//        delete item;
}

void
ProcessFileThread::run()
{
    switch (process_type) {
    case PROCESS_FILE:
        processFileData();
        break;
    case PROCESS_BATCHES:
        processFileBatches();
        break;
    default:
        break;
    }
}

void
ProcessFileThread::processFileBatches()
{
    std::ifstream file( filename.toAscii(), std::ifstream::binary);
    runinfo.clear();

    if (file.is_open()) {

        // fitting parameters
        SharedFitParameters params = FitParameters::instance();

        // background data
        SignalArray back;
        size_t n = 0;

        // buffer data
        char* buf = reinterpret_cast<char*>(buffer);
        int items_proc = 0;
        for ( QListWidgetItem* item : batches) {

            if (file.eof())
                break;

            {
                QMutexLocker locker(mutex);
                if (stopped)
                    break;
            }
            RunDetailsListWidgetItem* ritem = dynamic_cast<RunDetailsListWidgetItem*>(item);
            if (ritem) {
                file.seekg( ritem->batch_offset(), file.beg);

                // read data as a block:
                size_t read_size = (ritem->batch_bytes() > BUFFER_SIZE) ? BUFFER_SIZE : ritem->batch_bytes();
                file.read( buf, read_size);

                size_t nread = (file) ? read_size : file.gcount();

                CountsList list;
                list.reserve(BUFFER_SIZE / mask.size());
                DataVector data( buffer, buffer + nread);

                size_t proc = 0;
                this->process_data( list, data, proc);

                // process ADC counts here
                if (flag_background) {
                    fill_background( back, list, n);
                }
                RunInfo info = params->fit( list, diagrams, flag_background);
                runinfo += info;

                items_proc++;
                emit progress(items_proc);
            }
        }

        // final background data
        if (n && flag_background) {
            for ( auto& p : back) {
                p.second = sqrt(p.second);
            }
            params->background() = back;
        }

    }
    file.close();

    stopped = false;
}

void
ProcessFileThread::processFileData()
{
    std::ifstream file( filename.toAscii(), std::ifstream::binary);
    runinfo.clear();

    if (file.is_open()) {

        // fitting parameters
        SharedFitParameters params = FitParameters::instance();

        // background data
        SignalArray back;
        size_t n = 0;

        // buffer data
        char* buf = reinterpret_cast<char*>(buffer);
        size_t total = 0;
        size_t proc = 0;
        do {
            {
                QMutexLocker locker(mutex);
                if (stopped)
                    break;
            }

            // read data as a block:
            file.read( buf, BUFFER_SIZE);

            size_t nread = (file) ? BUFFER_SIZE : file.gcount();

            CountsList list;
            list.reserve(BUFFER_SIZE / mask.size());
            DataVector data( buffer, buffer + nread);

            size_t pos = this->process_data( list, data, proc);
            total += nread;

            if (!file.eof() && nread != pos) {
                qDebug() << "File Processing Thread: current seekg changed " << int(pos - nread);
                file.seekg( int(pos - nread), file.cur);
                total += int(pos - nread);
            }

            // process ADC counts here
            if (flag_background) {
                fill_background( back, list, n);
            }
            RunInfo info = params->fit( list, diagrams, flag_background);
            runinfo += info;

            emit progress(total);
        } while (!file.eof());

        // final background data
        if (n && flag_background) {
            for ( auto& p : back) {
                p.second = sqrt(p.second);
            }
            params->background() = back;
        }
    }
    file.close();

    stopped = false;
}

void
ProcessFileThread::stop()
{
    QMutexLocker locker(mutex);
    stopped = true;
}
