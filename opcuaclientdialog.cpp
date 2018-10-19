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

#include <QTimer>
#include <QDateTime>
#include <iostream>

#include "opcuaclient.h"

#include "opcuaclientdialog.h"
#include "ui_opcuaclientdialog.h"

OpcUaClientDialog::OpcUaClientDialog( const QString& path, OpcUaClient* client,
    bool connect_at_start, QWidget *parent)
    :
    QDialog(parent),
    ui(new Ui::OpcUaClientDialog),
    opcua_client(client),
    progress_dialog(new QProgressDialog( tr("Connection..."), \
        tr("Cancel Connection"), 0, 0, this)),
    item_state(nullptr),
    item_value(nullptr),
    item_value_int(nullptr),
    item_ext_command(nullptr),
    item_heartbeat(nullptr)
{
    ui->setupUi(this);

    connect( progress_dialog, SIGNAL(canceled()), this, SLOT(onCancelConnectionClicked()));

    connect( ui->connectPushButton, SIGNAL(clicked()), this, SLOT(onConnectClicked()));
    connect( ui->disconnectPushButton, SIGNAL(clicked()), this, SLOT(onDisconnectClicked()));

    ui->opcUaNodesTreeWidget->expandAll();

    if (opcua_client) {
        connect( opcua_client, SIGNAL(connected()), this, SLOT(onClientConnected()));
        connect( opcua_client, SIGNAL(disconnected()), progress_dialog, SLOT(cancel()));
    }
    if (opcua_client && opcua_client->isConnected()) {
        ui->connectPushButton->setEnabled(false);
        ui->disconnectPushButton->setEnabled(true);
    }
    else {
        ui->connectPushButton->setEnabled(true);
        ui->disconnectPushButton->setEnabled(false);
    }

    ui->opcUaServerNameLabel->setText(path);
    progress_dialog->setWindowTitle(tr("Connection progress"));

    QTreeWidgetItem* item = ui->opcUaNodesTreeWidget->topLevelItem(0);
    if (item) {
        item->setTextAlignment( 1, Qt::AlignHCenter | Qt::AlignVCenter);
        item->setTextAlignment( 2, Qt::AlignHCenter | Qt::AlignVCenter);

        if (opcua_client && opcua_client->isConnected())
            item->setText( 1, tr("Connected"));
        else
            item->setText( 1, tr("Disconnected"));

        QTreeWidgetItem* child = item->child(0);
        if (child) {
            child->setTextAlignment( 1, Qt::AlignHCenter | Qt::AlignVCenter);
            child->setTextAlignment( 2, Qt::AlignHCenter | Qt::AlignVCenter);
            item_state = child;
        }
        child = item->child(1);
        if (child) {
            child->setTextAlignment( 1, Qt::AlignHCenter | Qt::AlignVCenter);
            child->setTextAlignment( 2, Qt::AlignHCenter | Qt::AlignVCenter);
            item_ext_command = child;
        }
        child = item->child(2);
        if (child) {
            child->setTextAlignment( 1, Qt::AlignHCenter | Qt::AlignVCenter);
            child->setTextAlignment( 2, Qt::AlignHCenter | Qt::AlignVCenter);
            item_value = child;
        }
        child = item->child(3);
        if (child) {
            child->setTextAlignment( 1, Qt::AlignHCenter | Qt::AlignVCenter);
            child->setTextAlignment( 2, Qt::AlignHCenter | Qt::AlignVCenter);
            item_value_int = child;
        }
        child = item->child(4);
        if (child) {
            child->setTextAlignment( 1, Qt::AlignHCenter | Qt::AlignVCenter);
            child->setTextAlignment( 2, Qt::AlignHCenter | Qt::AlignVCenter);
            item_heartbeat = child;
        }
    }

    if (connect_at_start) {
        QTimer::singleShot( 1000, this, SLOT(onConnectClicked()));
    }
}

OpcUaClientDialog::~OpcUaClientDialog()
{
    delete ui;
}

void
OpcUaClientDialog::setExtCommandValue( int value, const QDateTime& dt)
{
    QString dt_str = dt.toString(Qt::ISODate);
    if (item_ext_command) {
        item_ext_command->setText( 1, QString("%1").arg(value));
        item_ext_command->setText( 2, dt_str);
    }
}

void
OpcUaClientDialog::setState(StateType state_type)
{
    int state;
    switch (state_type) {
    case STATE_NONE:
        state = 0xFF;
        break;
    case STATE_DEVICE_DISCONNECTED:
        state = 0x00;
        break;
    case STATE_DEVICE_CONNECTED:
        state = 0x01;
        break;
    case STATE_ACQUISITION_BACKGROUND:
        state = 0x02;
        break;
    case STATE_ACQUISITION_FIXED_POSITION:
        state = 0x04;
        break;
    case STATE_ACQUISITION_SCANNING:
        state = 0x08;
        break;
    case STATE_ACQUISITION_EXTERNAL_COMMAND:
        state = 0x10;
        break;
    case STATE_POSITION_MOVE:
        state = 0x20;
        break;
    case STATE_POSITION_REMOVE:
        state = 0x40;
        break;
    case STATE_POSITION_FINISH:
        state = 0x80;
        break;
    default:
        state = -1;
        break;
    }

    setStateValue( state, QDateTime::currentDateTime());
}

void
OpcUaClientDialog::setStateValue( int value, const QDateTime& dt)
{
    QString dt_str = dt.toString(Qt::ISODate);
    if (item_state) {
        item_state->setText( 1, QString("%1").arg(value));
        item_state->setText( 2, dt_str);
    }
}

void
OpcUaClientDialog::setHeatBeatValue( int value, const QDateTime& dt)
{
    QString dt_str = dt.toString(Qt::ISODate);
    if (item_heartbeat) {
        item_heartbeat->setText( 1, QString("%1").arg(value));
        item_heartbeat->setText( 2, dt_str);
    }
}

void
OpcUaClientDialog::setBreamSpectrumValue( const RunInfo::BeamSpectrumArray& batch_array,
    const RunInfo::BeamSpectrumArray& mean_array, const QDateTime& datetime)
{
}

void
OpcUaClientDialog::serverPathChanged( const QString& opcua_path, int opcua_port)
{
    QString server_path;
    if (opcua_port == -1)
        server_path = opcua_path;
    else
        server_path = QString("%1:%2").arg(opcua_path).arg(opcua_port);

    ui->opcUaServerNameLabel->setText(server_path);
}

void
OpcUaClientDialog::onClientConnected()
{
    progress_dialog->hide();
    ui->connectPushButton->setEnabled(false);
    ui->disconnectPushButton->setEnabled(true);
    QTreeWidgetItem* item = ui->opcUaNodesTreeWidget->topLevelItem(0);
    if (item)
        item->setText( 1, tr("Connected"));
}

void
OpcUaClientDialog::onConnectClicked()
{
    if (opcua_client && !opcua_client->isConnected()) {
        QString path = ui->opcUaServerNameLabel->text();
        opcua_client->connect_async(path);
        progress_dialog->show();
        ui->connectPushButton->setEnabled(false);
        ui->disconnectPushButton->setEnabled(false);
    }
}

void
OpcUaClientDialog::onDisconnectClicked()
{
    if (opcua_client && opcua_client->isConnected()) {

        QDateTime now = QDateTime::currentDateTime();
        if (opcua_client->writeHeartBeatValue( 0, now)) {
            setHeatBeatValue( 0, now);
        }

        opcua_client->disconnect();
        ui->connectPushButton->setEnabled(true);
        ui->disconnectPushButton->setEnabled(false);
        QTreeWidgetItem* item = ui->opcUaNodesTreeWidget->topLevelItem(0);
        if (item)
            item->setText( 1, tr("Disconnected"));
    }
}

void
OpcUaClientDialog::onCancelConnectionClicked()
{
    if (opcua_client)
        opcua_client->disconnect();

    ui->connectPushButton->setEnabled(true);
    ui->disconnectPushButton->setEnabled(false);
    progress_dialog->hide();
    QTreeWidgetItem* item = ui->opcUaNodesTreeWidget->topLevelItem(0);
    if (item)
        item->setText( 1, tr("Disconnected"));
}

void
OpcUaClientDialog::setBeamSpectrumState( int state, const QDateTime&)
{
    QTreeWidgetItem* item = ui->opcUaNodesTreeWidget->topLevelItem(0);
    if (item && state)
        item->setText( 1, tr("Connected"));
    else if (item && !state)
        item->setText( 1, tr("Disconnected"));
}
