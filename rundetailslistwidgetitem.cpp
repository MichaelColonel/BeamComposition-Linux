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

#include "rundetailslistwidgetitem.h"

/*
RunDetailsListWidgetItem::RunDetailsListWidgetItem( size_t batch_bytes,
    size_t batch_events, size_t batch_offset,
    QListWidget* parent, int type)
    :
    QListWidgetItem( parent, type),
    offset(batch_offset),
    bytes(batch_bytes),
    events(batch_events)
{
    date_time = QDateTime::currentDateTime();
    batch_number = 1;

    QString date_text = date_time.toString("dd.MM.yyyy");
    QString time_text = date_time.toString("hh:mm:ss");

    QString text = QString("Batch %1\nDate: %2, Time: %3\nBytes: %4\nEvents: %5")
            .arg(batch_number).arg(date_text).arg(time_text).arg(bytes).arg(events);

    setText(text);
}
*/

RunDetailsListWidgetItem::RunDetailsListWidgetItem( const QDateTime& datetime,
    int number, size_t batch_bytes, size_t batch_events, size_t batch_offset,
    QListWidget* parent, int type)
    :
    QListWidgetItem( parent, type),
    date_time(datetime),
    batch_number(number),
    offset(batch_offset),
    bytes(batch_bytes),
    events(batch_events)
{
    QString date_text = date_time.toString("dd.MM.yyyy");
    QString time_text = date_time.toString("hh:mm:ss");

    QString text = QObject::tr("Batch %1\nDate: %2, Time: %3\nBytes: %4\nEvents: %5")
            .arg(batch_number).arg(date_text).arg(time_text).arg(bytes).arg(events);

    setText(text);
}

RunDetailsListWidgetItem::~RunDetailsListWidgetItem()
{

}

QString
RunDetailsListWidgetItem::file_string() const
{
    QString date_text = date_time.toString("dd.MM.yyyy");
    QString time_text = date_time.toString("hh:mm:ss");

    QString text = QString("%1 %2 %3 %4 %5").arg(date_text).arg(time_text).arg( offset, 16).arg( bytes, 10).arg( events, 10);

    return text;
}
