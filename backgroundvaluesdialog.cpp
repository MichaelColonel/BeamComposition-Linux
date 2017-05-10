/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */

#include "backgroundvaluedelegate.h"
#include "channelscountsfit.h"

#include "backgroundvaluesdialog.h"
#include "ui_backgroundvaluesdialog.h"

BackgroundValuesDialog::BackgroundValuesDialog(QWidget* parent)
    :
    QDialog(parent),
    ui(new Ui::BackgroundValuesDialog)
{
    ui->setupUi(this);

    ui->tableWidget->setItemDelegate(new ValueDelegate());

    for ( int i = 0; i < CHANNELS; ++i) {
        const BackgroundPair& p = ChannelsCountsFit::background[i];
        QTableWidgetItem *item_mean = new QTableWidgetItem(tr("%1").arg( p.first, 4, 'f', 1));
        ui->tableWidget->setItem( i, 0, item_mean);

        QTableWidgetItem *item_sigma = new QTableWidgetItem(tr("%1").arg(p.second, 4, 'f', 1));
        ui->tableWidget->setItem( i, 1, item_sigma);
    }
}

BackgroundValuesDialog::~BackgroundValuesDialog()
{
    delete ui;
}
