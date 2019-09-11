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

#include <QWaitCondition>
#include <QMutex>

#include <TH1.h>
#include <TH2.h>

#include <iostream>
#include <functional>
#include <fstream>

#include "runinfo.h"
#include "channelscountsfit.h"
#include "diagramtreewidgetaction.h"
#include "rundetailslistwidgetitem.h"
#include "port.h"
#include "acquisitionthread.h"

#define MASK_SIZE 8
#define MASK_BITS 0x03 // mask
#define BUFFER_SIZE 0x1000000 // 16 Megabyte

namespace {

#if defined(__GNUG__) && (__cplusplus >= 201103L)
// lambda to check mask within the sequence of raw buffer data
std::function< bool( unsigned char, unsigned char) >
check_mask = [] ( unsigned char value, unsigned char mask) -> bool {
    return !((value & MASK_BITS) ^ mask);
};
#else
bool
check_mask( unsigned char value, unsigned char mask)
{
    return !((value & MASK_BITS) ^ mask);
}
#endif

#if defined(__GNUG__) && (__cplusplus >= 201103L)
const std::array< unsigned char, MASK_SIZE > mask_vector{
    0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x03
};
#else
// mask buffer of the batch (high byte, low byte)
const unsigned char mask_array[MASK_SIZE] = {
    0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x03
};
const DataVector mask_vector( mask_array, mask_array + MASK_SIZE);
#endif

QWaitCondition cond_acquire;
QMutex* mutex = new QMutex;
char* buffer = new char[BUFFER_SIZE];
DataQueue queue;

} // namespace

AcquireThread::AcquireThread(QObject* parent)
    :
    QThread(parent),
    stopped(false),
    fd(-1)
{
}

AcquireThread::~AcquireThread()
{
    fd = -1;
}

void
AcquireThread::stop()
{
    QMutexLocker locker(mutex);
    stopped = true;
}

void
AcquireThread::setFileDescriptor(int fd_)
{
    fd = fd_;
}

void
AcquireThread::run()
{
    while (true) {
        int error_res = 0;
        size_t nread;
        nread = port_readn( fd, buffer, MASK_SIZE, &error_res);
        {
            QMutexLocker locker(mutex);
            if (stopped)
                break;
        }
        if (nread >= mask_vector.size() && !error_res) {
            // put available data in the queue
            // acquire condition signal for processing thread

            DataVector localdata( buffer, buffer + nread);
            {
//                qDebug() << "Data Acquisition Thread: signal data is ready";
                QMutexLocker locker(mutex);
                queue.push(std::move(localdata));
                cond_acquire.wakeOne();
            }
        }
        else if (error_res < 0) {
            emit signalDeviceError(error_res);
            break;
        }
        else if (error_res == 1) {
            std::cerr << "No data within timeout interval" << std::endl;
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
//    counts.reserve(BUFFER_SIZE);
//    bufferdata.reserve(BUFFER_SIZE * mask.size());
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
ProcessThread::appendData(const DataVector& rawdata)
{
	QMutexLocker locker(mutex);
	queue.push(rawdata);
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
#if defined(__GNUG__) && (__cplusplus >= 201103L)
            for ( unsigned char v : localdata)
                 bufferdata.push_back(v);
#else
            for ( DataVector::const_iterator it = localdata.begin(); it != localdata.end(); ++it)
                 bufferdata.push_back(*it);
#endif
        }

        // process the raw data to obtain ADC counts
        size_t proc = 0; // number of processed buffer data
        CountsList localcounts; // processed ADC counts list
        size_t res = process_data( localcounts, localdata, proc);
        if (res) {
            QMutexLocker locker(mutex);
#if defined(__GNUG__) && (__cplusplus >= 201103L)
            for ( const CountsArray& event_counts: localcounts)
                counts.push_back(event_counts);
#else
            for ( CountsList::const_iterator it = localcounts.begin(); it != localcounts.end(); ++it)
                counts.push_back(*it);
#endif
        }

        if (res && flag_background)
            fill_background( back, localcounts, n);
    }

    // final background data (if it was any data obtained)
    if (n && flag_background) {
#if defined(__GNUG__) && (__cplusplus >= 201103L)
        for ( auto& p : back) {
#else
        for ( SignalArray::iterator it = back.begin(); it != back.end(); ++it) {
            SignalPair& p = *it;
#endif
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

/**
 * @brief Transform batch of raw data into ADC counts array
 * @param list - list of ADC arrays
 * @param data - batch of raw data
 * @param proc - number of process events
 * @return
 */
size_t
ProcessThread::process_data( CountsList& list, DataVector& data, size_t& proc) const
{
    size_t res = 0;
/*
    quint8 mb = MASK_BITS;

    // lambda to check mask within the sequence of raw buffer data
    auto check_mask = [mb] ( unsigned char v, unsigned char m) -> bool {
        return !((v & mb) ^ m);
    };
*/
    auto it = data.begin(); // DataVector::iterator
    while (it != data.end()) {
        it = std::search( it, data.end(), mask_vector.begin(), mask_vector.end(), check_mask);

        if (it != data.end()) {
//            DataVector batch( it, it + mask_vector.size());
//            batch_to_counts( list, batch);
            CountsArray counts = BufferData::array(DataVector( it, it + mask_vector.size()));
            list.push_back(counts);

            it += mask_vector.size();
            proc += mask_vector.size();
            res = it - data.begin();
        }
    }

    return res;
}

void
ProcessThread::fill_background( SignalArray& back, const CountsList& lcounts, size_t& n)
{
#if defined(__GNUG__) && (__cplusplus >= 201103L)
    for ( const CountsArray& ch : lcounts) {
#else
    for ( CountsList::const_iterator it = lcounts.begin(); it != lcounts.end(); ++it) {
        const CountsArray& ch = *it;
#endif
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
    std::string std_filename = filename.toStdString();
    std::ifstream file( std_filename.c_str(), std::ifstream::binary);

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
#if defined(__GNUG__) && (__cplusplus >= 201103L)
        for ( QListWidgetItem* item : batches) {
#else
        for ( QList<QListWidgetItem*>::iterator it = batches.begin(); it != batches.end(); ++it) {
            QListWidgetItem* item = *it;
#endif
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
//                list.reserve(BUFFER_SIZE / mask.size());
                DataVector data( buffer, buffer + nread);

                size_t proc = 0;
                this->process_data( list, data, proc);

                // process ADC counts here
                if (flag_background) {
                    fill_background( back, list, n);
                }
                RunInfo info = params->fit( list, diagrams, flag_background);
                runinfo += info;
                ritem->set_number_of_events( info.counted(), info.processed());

                items_proc++;
                emit progress(items_proc);
            }
        }

        // final background data
        if (n && flag_background) {
#if defined(__GNUG__) && (__cplusplus >= 201103L)
            for ( auto& p : back) {
#else
            for ( SignalArray::iterator it = back.begin(); it != back.end(); ++it) {
                SignalPair& p = *it;
#endif
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
    std::string std_filename = filename.toStdString();
    std::ifstream file( std_filename.c_str(), std::ifstream::binary);
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
//            list.reserve(BUFFER_SIZE / mask.size());
            DataVector data( buffer, buffer + nread);

            size_t pos = this->process_data( list, data, proc);
            total += nread;

            if (!file.eof() && nread != pos) {
//                qDebug() << "File Processing Thread: current seekg changed " << int(pos - nread);
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
#if defined(__GNUG__) && (__cplusplus >= 201103L)
            for ( auto& p : back) {
#else
            for ( SignalArray::iterator it = back.begin(); it != back.end(); ++it) {
                SignalPair& p = *it;
#endif
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
