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

#include <QListWidgetItem>
#include <QDateTime>
#include <QString>

#include "typedefs.h"

class RunDetailsListWidgetItem : public QListWidgetItem {

public:
//    RunDetailsListWidgetItem( size_t data_bytes, size_t event_counts, size_t offset = 0,
//        QListWidget* parent = 0, int type = QListWidgetItem::UserType + 1);
    RunDetailsListWidgetItem( const QDateTime& datetime, int spillnumber,
        size_t data_bytes, size_t event_counts, size_t offset = 0,
        QListWidget* parent = 0, int type = QListWidgetItem::UserType + 1);
    virtual ~RunDetailsListWidgetItem();
    QString file_string() const;
    void set_number_of_events( size_t new_events, size_t proc_events = 0) {
        events = new_events;
        processed_events = proc_events;
    }
    size_t batch_bytes() const { return bytes; }
    size_t batch_events() const { return events; }
    size_t batch_offset() const { return offset; }
    void update_text();
private:
    QDateTime date_time;
    int batch_number;
    size_t offset;
    size_t bytes;
    size_t events;
    size_t processed_events;
};
