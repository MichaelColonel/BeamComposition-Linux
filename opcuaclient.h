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
 */

#pragma once

#include <QObject>
#include <QString>

#include <open62541.h>
#include "runinfo.h"

class OpcUaClient : public QObject {
    Q_OBJECT
public:
    OpcUaClient(QObject* parent = 0);
    virtual ~OpcUaClient();
    UA_StatusCode connect_async( const QString& server, int port,
        const UA_ClientConfig& config = UA_ClientConfig_default);
    UA_StatusCode connect_async( const QString& path,
        const UA_ClientConfig& config = UA_ClientConfig_default);
    void disconnect();

    static void onConnectCallback( UA_Client* client, void* userdata,
        UA_UInt32 requestId, void* status);
    static void onReadExtCommandAttributeCallback( UA_Client* client, void* userdata,
        UA_UInt32 requestId, UA_Variant* var);

    #ifdef UA_ENABLE_SUBSCRIPTIONS
    static void
    onSubscriptionExtCommandValueChanged( UA_Client* client, UA_UInt32 subId, void* subContext,
        UA_UInt32 monId, void* monContext, UA_DataValue* value);
    #endif
    void signalConnected();
    bool isConnected() const { return bool(client != nullptr); }
public slots:
    void iterate();
    UA_StatusCode writeBeamSpectrumValue(const RunInfo::BeamSpectrumArray&);
    UA_StatusCode writeBeamIntegralSpectrumValue(const RunInfo::BeamSpectrumArray&);
    UA_StatusCode writeHeartBeatValue(int);
    UA_StatusCode writeHeartBeatValue();
    UA_StatusCode writeCurrentStatusValue(int);

signals:
    void disconnected();
    void connected();
    void externalCommandChanged(int);

private:
    UA_Client* client;
    QString server_path;
    int server_port;
};
