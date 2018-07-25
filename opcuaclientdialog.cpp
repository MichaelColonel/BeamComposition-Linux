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
        tr("Cancel Connection"), 0, 0, this))
{
    ui->setupUi(this);

    connect( progress_dialog, SIGNAL(canceled()), this, SLOT(onCancelConnectionClicked()));
    connect( ui->connectPushButton, SIGNAL(clicked()), this, SLOT(onConnectClicked()));
    connect( ui->disconnectPushButton, SIGNAL(clicked()), this, SLOT(onDisconnectClicked()));

    ui->treeWidget->expandAll();

    if (opcua_client) {
        connect( opcua_client, SIGNAL(connected()), this, SLOT(onClientConnected()));
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

    if (connect_at_start) {
        QTimer::singleShot( 1000, this, SLOT(onConnectClicked()));
    }
}

OpcUaClientDialog::~OpcUaClientDialog()
{
    delete ui;
    delete progress_dialog;
}

void
OpcUaClientDialog::setExtCommandValue(int)
{
}

void
OpcUaClientDialog::setStateValue(int)
{
}

void
OpcUaClientDialog::setHeatBeatValue(int)
{
}

void
OpcUaClientDialog::setBreamSpectrumValue(const RunInfo::BeamSpectrumArray&)
{
}

void
OpcUaClientDialog::setIntergalBreamSpectrumValue(const RunInfo::BeamSpectrumArray&)
{
}

void
OpcUaClientDialog::onClientConnected()
{
    progress_dialog->hide();
    ui->connectPushButton->setEnabled(false);
    ui->disconnectPushButton->setEnabled(true);
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
        opcua_client->disconnect();
        ui->connectPushButton->setEnabled(true);
        ui->disconnectPushButton->setEnabled(false);
    }
}

void
OpcUaClientDialog::onCancelConnectionClicked()
{
    opcua_client->disconnect();
    ui->connectPushButton->setEnabled(true);
    ui->disconnectPushButton->setEnabled(false);
    progress_dialog->hide();
}
