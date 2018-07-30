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

#include <QDialog>
#include <QProgressDialog>

#include "runinfo.h"

namespace Ui {
class OpcUaClientDialog;
}

class QTreeWidgetItem;
class QSettings;
class OpcUaClient;

class OpcUaClientDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit OpcUaClientDialog( const QString& path, OpcUaClient* client,
        bool connect_on_start = false, QWidget *parent = 0);
    ~OpcUaClientDialog();

public slots:
    void onClientConnected();
    void onCancelConnectionClicked();
    void setExtCommandValue(int);
    void setStateValue(int);
    void setHeatBeatValue(int);
    void setBreamSpectrumValue(const RunInfo::BeamSpectrumArray&);
    void setIntergalBreamSpectrumValue(const RunInfo::BeamSpectrumArray&);

private slots:
    void onConnectClicked();
    void onDisconnectClicked();

private:
    Ui::OpcUaClientDialog *ui;
    OpcUaClient* opcua_client;
    QProgressDialog* progress_dialog;
    QTreeWidgetItem* item_state;
    QTreeWidgetItem* item_value;
    QTreeWidgetItem* item_value_int;
    QTreeWidgetItem* item_ext_command;
    QTreeWidgetItem* item_heartbeat;
};
