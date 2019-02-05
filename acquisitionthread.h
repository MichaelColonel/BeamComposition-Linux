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

#include <QThread>

#include "runinfo.h"
#include "typedefs.h"

class QListWidgetItem;

/**
 * @brief The AcquireThread class -- acquire raw data from FT2232H channel B
 */
class AcquireThread : public QThread {
    Q_OBJECT
public:
    explicit AcquireThread(QObject* parent = 0);
    virtual ~AcquireThread();
    void setFileDescriptor(int fd);

public slots:
    void stop();

signals:
    void signalDeviceError(int);

protected:
    virtual void run();

    bool stopped;
    int fd;
};

/**
 * @brief The ProcessThread class -- process the acquired raw data to detectors counts
 */
class ProcessThread : public QThread {
    Q_OBJECT
public:
    explicit ProcessThread(QObject* parent = 0);
    virtual ~ProcessThread();
    void getProcessedData( DataList&, CountsList&);
    void setBackground(bool background) { flag_background = background; }
    bool isBackground() const { return flag_background; }

public slots:
    virtual void stop();

protected:
    virtual void run();
    virtual size_t process_data( CountsList& list,
        DataVector& bufdata, size_t& proc) const;
    void fill_background( SignalArray& back, const CountsList& lcounts, size_t& n);
    bool stopped;
    bool flag_background; // background signal measurement

private:
/*
    void batch_to_counts( CountsList& list, const DataVector& batch) const {
        CountsArray counts = BufferData(batch).array();
        list.push_back(counts);
    }
*/
    // ADC count from high and low data bytes
    // MO=2 mask bits offset
    // BO=6 high byte bits offset
    // Count = (high_byte >> MO) << BO | low_byte >> MO
    class Count {
    public:
        explicit Count( unsigned char hb, unsigned char lb) : h(hb), l(lb) {}
        unsigned short value() const { return (h >> MO) << BO | l >> MO; }
        static unsigned short value( unsigned char h, unsigned char l) {
            return (h >> MO) << BO | l >> MO;
        }
    private:
        unsigned short h, l;
        static const int MO = 2, BO = 6;
    };

    // Batch of raw buffer data transforms into counts array
    class BufferData {
    public:
        explicit BufferData(const DataVector& v) : d(v) {}
        CountsArray array() const {
            CountsArray channel;
            channel[0] = Count( d[4], d[5]).value(); // channel-1 / index 4-5
            channel[1] = Count( d[2], d[3]).value(); // channel-2 / index 2-3
            channel[2] = Count( d[6], d[7]).value(); // channel-3 / index 6-7
            channel[3] = Count( d[0], d[1]).value(); // channel-4 / index 0-1
            return channel;
        }
        static CountsArray array(const DataVector& d) {
            CountsArray channel;
            channel[0] = Count::value( d[4], d[5]); // channel-1 / index 4-5
            channel[1] = Count::value( d[2], d[3]); // channel-2 / index 2-3
            channel[2] = Count::value( d[6], d[7]); // channel-3 / index 6-7
            channel[3] = Count::value( d[0], d[1]); // channel-4 / index 0-1
            return channel;
        }

    private:
        const DataVector& d;
    };

    CountsList counts;
    DataList bufferdata;
};

class ProcessFileThread : public ProcessThread {
    Q_OBJECT
public:
    explicit ProcessFileThread(QObject* parent = 0);
    virtual ~ProcessFileThread();
    void setDiagrams(const Diagrams& d) { diagrams = d; }
//    QList<QListWidgetItem*> processedBatches() const { return batches; }
    void processBatches( const QList<QListWidgetItem*>& items, bool background_data = false) {
        batches = items;
        flag_background = background_data;
        process_type = PROCESS_BATCHES;
    }

    void setBatches( const QString& fname, const QList<QListWidgetItem*>& items,
        bool background_data = false) {
        filename = fname,
        batches = items;
        flag_background = background_data;
        process_type = PROCESS_BATCHES;
    }

    void setFile( const QString& fname, bool background_data = false) {
        filename = fname; flag_background = background_data;
        process_type = PROCESS_FILE;
    }

    RunInfo runInfo() const { return runinfo; }

public slots:
    virtual void stop();

signals:
    void progress(int);

protected:
    virtual void run();

private:
    void processFileData();
    void processFileBatches();

    Diagrams diagrams;
    QString filename;
    RunInfo runinfo;
    QList<QListWidgetItem*> batches;
    enum ProcessType {
        PROCESS_BATCHES,
        PROCESS_FILE
    };
    ProcessType process_type;
};
