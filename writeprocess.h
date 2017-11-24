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

#include <QRunnable>

#include "typedefs.h"

class QFile;

class WriteDataProcess : public QRunnable {
public:
    explicit WriteDataProcess( QFile* file, const DataList& d = DataList());
    virtual ~WriteDataProcess();
    virtual void run();

protected:
    QFile* filerun;
    DataList data;
};

/*
class WriteCountsProcess : public WriteDataProcess {
public:
    explicit WriteCountsProcess( QFile* file, const CountsList& c = CountsList());
    virtual ~WriteCountsProcess();
    virtual void run();

protected:
    CountsList counts;
};
*/

class WriteDataTimeProcess : public WriteDataProcess {
public:
    explicit WriteDataTimeProcess( QFile* file, time_t datetime, const DataList& d = DataList());
    virtual ~WriteDataTimeProcess();
    virtual void run();

protected:
    time_t datetime;
};
