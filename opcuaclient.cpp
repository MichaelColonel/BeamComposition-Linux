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

#include <iostream>

#include <QDebug>
#include <QDateTime>

#include "opcuaclient.h"

namespace {

const UA_String parent_node_str = UA_STRING_STATIC("RBS.BeamSpectrum.01");
const UA_String heart_beat_str = UA_STRING_STATIC("HeartBeat");
const UA_String state_str = UA_STRING_STATIC("State");
const UA_String value_str = UA_STRING_STATIC("Value");
const UA_String value_integral_str = UA_STRING_STATIC("ValueIntegral");
const UA_String command_str = UA_STRING_STATIC("Command");

OpcUaClient* local_client_ptr = 0;

#ifdef UA_ENABLE_SUBSCRIPTIONS
const UA_CreateSubscriptionRequest request = UA_CreateSubscriptionRequest_default();
UA_MonitoredItemCreateRequest monRequest;
#endif

} // namespace

OpcUaClient::OpcUaClient(QObject* parent)
    :
    QObject(parent),
    client(nullptr),
    server_path("opc.tcp://localhost"),
    server_port(4840)
{
}

void
OpcUaClient::disconnect()
{
    if (client) {
        UA_Client_disconnect(client);
        emit disconnected();
        UA_Client_delete(client); /* Disconnects the client internally */
        client = nullptr;
    }
}

OpcUaClient::~OpcUaClient()
{
    disconnect();
}

UA_StatusCode
OpcUaClient::connect_async( const QString& path, int port, const UA_ClientConfig& config)
{
    server_path = path;
    server_port = port;

    QString server_string = QString("%1:%2").arg(server_path).arg(server_port);

    UA_StatusCode retval = connect_async( server_string, config);
/*
    if (retval != UA_STATUSCODE_GOOD) {
        UA_Client_delete(opcua_client);
        opcua_client = 0;
        ui->statusBar->showMessage( tr("Connection failed"), 1000);
    }
    else {
        progress_dialog->show();
        ui->connectPushButton->setEnabled(false);
        ui->statusBar->showMessage( tr("Async connection initiated..."), static_cast<int>(config.timeout));
        opcua_timer->start();
    }
*/
    return retval;
}

UA_StatusCode
OpcUaClient::connect_async( const QString& path, const UA_ClientConfig& config)
{
    std::string serv_string = path.toStdString();

    client = UA_Client_new(config);
/*
    UA_StatusCode retval = UA_Client_connect( client, serv_string.c_str());
    if (retval != UA_STATUSCODE_GOOD) {
        UA_Client_delete(client);
        client = nullptr;
    }
    else {
        const char* str = reinterpret_cast<const char*>(parent_node_str.data);
        UA_NodeId parent = UA_NODEID_STRING_ALLOC( 0, str);
        retval = UA_Client_forEachChildNodeCall( client, parent, nodeIterCallback, this);
//        UA_NodeId_deleteMembers(&parent);

        if ((retval == UA_STATUSCODE_GOOD) && initExternalCommandSubscription()) {
            emit connected();
        }
    }
    return retval;
*/

    UA_StatusCode retval = UA_Client_connect_async( client, serv_string.c_str(),
        onConnectCallback, this);
/*
    if (retval != UA_STATUSCODE_GOOD) {
        UA_Client_delete(opcua_client);
        opcua_client = 0;
        ui->statusBar->showMessage( tr("Connection failed"), 1000);
    }
    else {
        progress_dialog->show();
        ui->connectPushButton->setEnabled(false);
        ui->statusBar->showMessage( tr("Async connection initiated..."), static_cast<int>(config.timeout));
        opcua_timer->start();
    }
*/
    return retval;
}

bool
OpcUaClient::writeBeamSpectrumValue( const RunInfo::BeamSpectrumArray& batch_array,
    const RunInfo::BeamSpectrumArray& mean_array, const QDateTime& datetime)
{
    if (!isConnected())
        return false;

    bool result = false;
    uint t = datetime.toTime_t();
    UA_DateTime dt = UA_DateTime_fromUnixTime(static_cast<UA_Int64>(t));    
    UA_Double batch_data[batch_array.size()], mean_data[batch_array.size()];

    for ( size_t i = 0; i < batch_array.size(); ++i) {
        batch_data[i] = batch_array[i];
        mean_data[i] = mean_array[i];
    }

    void* ptr = UA_Array_new( 2, &UA_TYPES[UA_TYPES_WRITEVALUE]);
    UA_WriteValue* values = reinterpret_cast<UA_WriteValue*>(ptr);

    UA_WriteRequest wReq;
    UA_WriteRequest_init(&wReq);
    wReq.nodesToWrite = values;
    wReq.nodesToWriteSize = 2;
    UA_NodeId_copy( &children_nodes[VALUE_NODE], &wReq.nodesToWrite[0].nodeId);
//    wReq.nodesToWrite[0].nodeId = UA_NODEID_STRING_ALLOC( 0, "Value");
    wReq.nodesToWrite[0].attributeId = UA_ATTRIBUTEID_VALUE;
    wReq.nodesToWrite[0].value.hasValue = true;
    wReq.nodesToWrite[0].value.sourceTimestamp = dt;
    wReq.nodesToWrite[0].value.value.type = &UA_TYPES[UA_TYPES_DOUBLE];
    wReq.nodesToWrite[0].value.value.storageType = UA_VARIANT_DATA_NODELETE;
    UA_Variant_setArrayCopy( &wReq.nodesToWrite[0].value.value, batch_data, CARBON_Z, &UA_TYPES[UA_TYPES_DOUBLE]);

    UA_NodeId_copy( &children_nodes[VALUE_INTEGRAL_NODE], &wReq.nodesToWrite[1].nodeId);
//    wReq.nodesToWrite[1].nodeId = UA_NODEID_STRING_ALLOC( 0, "ValueIntegral");
    wReq.nodesToWrite[1].attributeId = UA_ATTRIBUTEID_VALUE;
    wReq.nodesToWrite[1].value.hasValue = true;
    wReq.nodesToWrite[1].value.sourceTimestamp = dt;
    wReq.nodesToWrite[1].value.value.type = &UA_TYPES[UA_TYPES_DOUBLE];
    wReq.nodesToWrite[1].value.value.storageType = UA_VARIANT_DATA_NODELETE;
    UA_Variant_setArrayCopy( &wReq.nodesToWrite[1].value.value, mean_data, CARBON_Z, &UA_TYPES[UA_TYPES_DOUBLE]);

    UA_WriteResponse wResp = UA_Client_Service_write( client, wReq);
    if(wResp.responseHeader.serviceResult == UA_STATUSCODE_GOOD) {
        result = true;
    }
    UA_WriteRequest_deleteMembers(&wReq);
    UA_WriteResponse_deleteMembers(&wResp);
    return result;

/*
    UA_Variant batch_var;
    UA_Variant_init(&batch_var);
    UA_Variant_setArrayCopy( &batch_var, batch_data, CARBON_Z, &UA_TYPES[UA_TYPES_FLOAT]);

    UA_WriteValue batch_value;
    UA_WriteValue_init(&batch_value);
    batch_value.nodeId = NODE_ID_CHARGE_VALUE;
    batch_value.attributeId = UA_ATTRIBUTEID_VALUE;
    batch_value.value.status = UA_STATUSCODE_GOOD;
    batch_value.value.sourceTimestamp = ua_dt;
    batch_value.value.hasStatus = true;
    batch_value.value.value = batch_var;
    batch_value.value.hasValue = true;

    UA_Variant mean_var;
    UA_Variant_init(&mean_var);
    UA_Variant_setArrayCopy( &mean_var, mean_data, CARBON_Z, &UA_TYPES[UA_TYPES_FLOAT]);

    UA_WriteValue mean_value;
    UA_WriteValue_init(&mean_value);
    mean_value.nodeId = NODE_ID_CHARGE_VALUE;
    mean_value.attributeId = UA_ATTRIBUTEID_VALUE;
    mean_value.value.status = UA_STATUSCODE_GOOD;
    mean_value.value.sourceTimestamp = ua_dt;
    mean_value.value.hasStatus = true;
    mean_value.value.value = mean_var;
    mean_value.value.hasValue = true;

    UA_WriteValue values[2] = { batch_value, mean_value };
    UA_WriteRequest write_request;
    UA_WriteRequest_init(&write_request);
    write_request.nodesToWrite = values;
    write_request.nodesToWriteSize = 2;
    UA_WriteResponse write_response = UA_Client_Service_write( client, write_request);
    bool result = false;
    if (write_response.responseHeader.serviceResult == UA_STATUSCODE_GOOD) {
        result = true;
    }

    UA_WriteRequest_deleteMembers(&write_request);
    UA_WriteResponse_deleteMembers(&write_response);
    return result;
*/
}

bool
OpcUaClient::writeHeartBeatValue( int heart_beat, const QDateTime& datetime)
{
    if (!isConnected())
        return false;

    bool result = false;

    uint t = datetime.toTime_t();
    UA_DateTime dt = UA_DateTime_fromUnixTime(static_cast<UA_Int64>(t));

    UA_Int32 value = static_cast<UA_Int32>(heart_beat);

    UA_WriteRequest wReq;
    UA_WriteRequest_init(&wReq);
    wReq.nodesToWrite = UA_WriteValue_new();
    wReq.nodesToWriteSize = 1;
    UA_NodeId_copy( &children_nodes[HEART_BEAT_NODE], &wReq.nodesToWrite[0].nodeId);
//    wReq.nodesToWrite[0].nodeId = UA_NODEID_STRING_ALLOC( 0, "HeartBeat");
    wReq.nodesToWrite[0].attributeId = UA_ATTRIBUTEID_VALUE;
    wReq.nodesToWrite[0].value.hasValue = true;
    wReq.nodesToWrite[0].value.value.type = &UA_TYPES[UA_TYPES_INT32];
    wReq.nodesToWrite[0].value.sourceTimestamp = dt;
    wReq.nodesToWrite[0].value.value.storageType = UA_VARIANT_DATA_NODELETE; /* do not free the integer on deletion */
    wReq.nodesToWrite[0].value.value.data = &value;
    UA_WriteResponse wResp = UA_Client_Service_write( client, wReq);
    if (wResp.responseHeader.serviceResult == UA_STATUSCODE_GOOD) {
        result = true;
    }
    UA_WriteRequest_deleteMembers(&wReq);
    UA_WriteResponse_deleteMembers(&wResp);
    return result;
}

bool
OpcUaClient::writeStateValue(StateType state_type)
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
    return writeCurrentStateValue( state, QDateTime::currentDateTime());
}

bool
OpcUaClient::writeCurrentStateValue( int current_state, const QDateTime& datetime)
{
    if (!isConnected())
        return false;

    bool result = false;

    uint t = datetime.toTime_t();
    UA_DateTime dt = UA_DateTime_fromUnixTime(static_cast<UA_Int64>(t));

    UA_Int16 value = static_cast<UA_Int16>(current_state);

    UA_WriteRequest wReq;
    UA_WriteRequest_init(&wReq);
    wReq.nodesToWrite = UA_WriteValue_new();
    wReq.nodesToWriteSize = 1;
    UA_NodeId_copy( &children_nodes[STATE_NODE], &wReq.nodesToWrite[0].nodeId);
//    wReq.nodesToWrite[0].nodeId = UA_NODEID_STRING_ALLOC( 0, "State");
    wReq.nodesToWrite[0].attributeId = UA_ATTRIBUTEID_VALUE;
    wReq.nodesToWrite[0].value.hasValue = true;
    wReq.nodesToWrite[0].value.value.type = &UA_TYPES[UA_TYPES_INT16];
    wReq.nodesToWrite[0].value.sourceTimestamp = dt;
    wReq.nodesToWrite[0].value.value.storageType = UA_VARIANT_DATA_NODELETE; /* do not free the integer on deletion */
    wReq.nodesToWrite[0].value.value.data = &value;
    UA_WriteResponse wResp = UA_Client_Service_write( client, wReq);
    if (wResp.responseHeader.serviceResult == UA_STATUSCODE_GOOD) {
        result = true;
    }
    UA_WriteRequest_deleteMembers(&wReq);
    UA_WriteResponse_deleteMembers(&wResp);
    return result;
}

void
OpcUaClient::iterate()
{
    if (client)
        UA_Client_run_iterate( client, 0);
}

#ifdef UA_ENABLE_SUBSCRIPTIONS
void
OpcUaClient::onSubscriptionExtCommandValueChanged( UA_Client* /* client */,
    UA_UInt32 /* subId */, void* /* subContext */, UA_UInt32 /* monId */,
    void* /* monContext */, UA_DataValue* value)
{
    if(UA_Variant_hasScalarType( &value->value, &UA_TYPES[UA_TYPES_INT16])) {
        UA_Int16 ext_command = *(UA_Int16*)value->value.data;
        UA_Int64 dtut = UA_DateTime_toUnixTime(value->sourceTimestamp);
        QDateTime dt = QDateTime::fromMSecsSinceEpoch(dtut * 1000);
        if (local_client_ptr) {
            local_client_ptr->signalExternalCommandChanged( int(ext_command), dt);
        }
    }
}
#endif

void
OpcUaClient::onConnectCallback( UA_Client* client, void* userdata,
    UA_UInt32 /* requestId */, void* status)
{
    UA_StatusCode status_code = *(UA_StatusCode*)status;
//    std::cout << "Client connected: " << UA_StatusCode_name(status_code) << std::endl;

    local_client_ptr = reinterpret_cast<OpcUaClient*>(userdata);
    if (local_client_ptr && (status_code == UA_STATUSCODE_GOOD)) {

        char* str = reinterpret_cast<char*>(parent_node_str.data);
        UA_NodeId parent = UA_NODEID_STRING( 0, str);

        UA_NodeId* res = UA_NodeId_new();
        status_code = UA_Client_readNodeIdAttribute( client, parent, res);
        UA_NodeId_delete(res);
        if (status_code != UA_STATUSCODE_GOOD) {
            local_client_ptr->signalDisconnected();
        }
        else {
            status_code = UA_Client_forEachChildNodeCall( client, parent, nodeIterCallback, userdata);

            if ((status_code == UA_STATUSCODE_GOOD) && local_client_ptr->initExternalCommandSubscription()) {
//              bool res = local_client_ptr->initExternalCommandSubscription();
//              std::cout << "Monitor initiation result: " << res << std::endl;
                local_client_ptr->signalConnected();
            }
        }
    }
    else if (local_client_ptr) {
    }
}

void
OpcUaClient::onReadExtCommandAttributeCallback( UA_Client* /* client */, void* /* userdata */,
    UA_UInt32 /* requestId */, UA_Variant* /*var */)
{

}

void
OpcUaClient::signalConnected()
{
    emit connected();
}

void
OpcUaClient::signalDisconnected()
{
    emit disconnected();
}

void
OpcUaClient::signalExternalCommandChanged( int value, const QDateTime& timestamp)
{
    emit externalCommandChanged( value, timestamp);
}

bool
OpcUaClient::initExternalCommandSubscription()
{
    bool result = false;
#ifdef UA_ENABLE_SUBSCRIPTIONS
    UA_CreateSubscriptionResponse response;
    response = UA_Client_Subscriptions_create( client, request, NULL, NULL, NULL);

    UA_NodeId monitored_node;
    UA_NodeId_copy( &children_nodes[COMMAND_NODE], &monitored_node);
    monRequest = UA_MonitoredItemCreateRequest_default(monitored_node);
//    UA_UInt32 subId = response.subscriptionId;
    if (response.responseHeader.serviceResult == UA_STATUSCODE_GOOD) {

        UA_MonitoredItemCreateResult monResponse;
        monResponse = UA_Client_MonitoredItems_createDataChange( client,
            response.subscriptionId, UA_TIMESTAMPSTORETURN_BOTH,
            monRequest, NULL, onSubscriptionExtCommandValueChanged, NULL);

        if (monResponse.statusCode == UA_STATUSCODE_GOOD)
            result = true;
    }
    UA_NodeId_deleteMembers(&monitored_node);

#endif
    return result;
}

bool
OpcUaClient::isConnected() const
{
    if (client) {
        UA_ClientState res = UA_Client_getState(client);
        return (res == UA_CLIENTSTATE_CONNECTED ||
                res == UA_CLIENTSTATE_SECURECHANNEL ||
                res == UA_CLIENTSTATE_SESSION);
    }
    else
        return false;
}

UA_StatusCode
OpcUaClient::nodeIterCallback( UA_NodeId childId, UA_Boolean isInverse, UA_NodeId /* referenceTypeId */,
    void* handle)
{
    if(isInverse)
        return UA_STATUSCODE_GOOD;

    OpcUaClient* ptr = reinterpret_cast<OpcUaClient*>(handle);
/*
    printf( "%s ---> NodeId %d, %s\n",
        referenceTypeId.identifier.string.data, childId.namespaceIndex,
        childId.identifier.string.data);
*/
    if (ptr) {

        if (UA_String_equal( &childId.identifier.string, &heart_beat_str)) {
//            std::cout << "Heart Beat Node OK!" << std::endl;
            ptr->setChildNode( childId, HEART_BEAT_NODE);
        }
        else if (UA_String_equal( &childId.identifier.string, &state_str)) {
//            std::cout << "State Node OK!" << std::endl;
            ptr->setChildNode( childId, STATE_NODE);
        }
        else if (UA_String_equal( &childId.identifier.string, &value_str)) {
//            std::cout << "Value Node OK!" << std::endl;
            ptr->setChildNode( childId, VALUE_NODE);
        }
        else if (UA_String_equal( &childId.identifier.string, &value_integral_str)) {
//            std::cout << "Value Integral Node OK!" << std::endl;
            ptr->setChildNode( childId, VALUE_INTEGRAL_NODE);
        }
        else if (UA_String_equal( &childId.identifier.string, &command_str)) {
//            std::cout << "Command Node OK!" << std::endl;
            ptr->setChildNode( childId, COMMAND_NODE);
        }
    }
    return UA_STATUSCODE_GOOD;
}

void
OpcUaClient::setChildNode(const UA_NodeId &child, ChildrenNodesType type)
{
    UA_NodeId_copy( &child, &children_nodes[type]);
}
