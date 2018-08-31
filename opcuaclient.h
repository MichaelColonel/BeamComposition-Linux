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

class QDateTime;

enum ChildrenNodesType {
    STATE_NODE,
    HEART_BEAT_NODE,
    VALUE_NODE,
    VALUE_INTEGRAL_NODE,
    COMMAND_NODE
};

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

    void signalConnected();
    void signalExternalCommandChanged( int, const QDateTime&);

    bool isConnected() const;
    bool initExternalCommandSubscription();
    void setChildNode( const UA_NodeId& child, ChildrenNodesType type);

    static void onConnectCallback( UA_Client* client, void* userdata,
        UA_UInt32 requestId, void* status);
    static void onReadExtCommandAttributeCallback( UA_Client* client, void* userdata,
        UA_UInt32 requestId, UA_Variant* var);

#ifdef UA_ENABLE_SUBSCRIPTIONS
    static void
    onSubscriptionExtCommandValueChanged( UA_Client* client, UA_UInt32 subId, void* subContext,
        UA_UInt32 monId, void* monContext, UA_DataValue* value);
#endif

    static UA_StatusCode nodeIterCallback( UA_NodeId childId,
        UA_Boolean isInverse, UA_NodeId referenceTypeId,
        void* handle);

public slots:
    void iterate();
    bool writeBeamSpectrumValue( const RunInfo::BeamSpectrumArray& batch,
        const RunInfo::BeamSpectrumArray& mean, const QDateTime&);
    bool writeHeartBeatValue( int, const QDateTime& datetime);
    bool writeCurrentStateValue( int, const QDateTime& datetime);

signals:
    void disconnected();
    void connected();
    void externalCommandChanged( int, const QDateTime&);

private:
    UA_Client* client;
    QString server_path;
    int server_port;
    std::map< ChildrenNodesType, UA_NodeId > children_nodes;
};
