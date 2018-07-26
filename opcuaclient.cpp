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

#include "opcuanodes.h"
#include "opcuaclient.h"

namespace {

const UA_NodeId NODE_ID_SPECTRUM_SYSTEM = UA_NODEID_STRING( 0, "RBS.BeamPrectrum.01");
const UA_NodeId NODE_ID_EXTERNAL_COMMAND = UA_NODEID_STRING( 0, "Command");
const UA_NodeId NODE_ID_CHARGE_VALUE = UA_NODEID_STRING( 0, "Value");
const UA_NodeId NODE_ID_CHARGE_VALUE_INTEGRAL = UA_NODEID_STRING( 0, "ValueIntegral");
const UA_NodeId NODE_ID_HEART_BEAT = UA_NODEID_STRING( 0, "HeartBeat");
const UA_NodeId NODE_ID_STATE = UA_NODEID_STRING( 0, "State");

OpcUaClient* local_client_ptr = 0;

#ifdef UA_ENABLE_SUBSCRIPTIONS
const UA_CreateSubscriptionRequest request = UA_CreateSubscriptionRequest_default();
const UA_MonitoredItemCreateRequest monRequest = UA_MonitoredItemCreateRequest_default(NODE_ID_EXTERNAL_COMMAND);
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
    std::string serv_string = server_string.toStdString();

    client = UA_Client_new(config);

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

UA_StatusCode
OpcUaClient::connect_async( const QString& path, const UA_ClientConfig& config)
{
    std::string serv_string = path.toStdString();

    client = UA_Client_new(config);

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
OpcUaClient::writeBeamComposition( const RunInfo& batch, const RunInfo& mean, const QDateTime& datetime)
{
    if (!isConnected())
        return false;

    bool result = false;
    uint t = datetime.toTime_t();
    UA_DateTime dt = UA_DateTime_fromUnixTime(static_cast<UA_Int64>(t));

    RunInfo::BeamSpectrumArray batch_array = batch.averageComposition();
    const float* batch_data = batch_array.data();

    RunInfo::BeamSpectrumArray mean_array = mean.averageComposition();
    const float* mean_data = mean_array.data();

    void* ptr = UA_Array_new( 2, &UA_TYPES[UA_TYPES_WRITEVALUE]);
    UA_WriteValue* values = reinterpret_cast<UA_WriteValue*>(ptr);

    UA_WriteRequest wReq;
    UA_WriteRequest_init(&wReq);
    wReq.nodesToWrite = values;
    wReq.nodesToWriteSize = 2;
    wReq.nodesToWrite[0].nodeId = UA_NODEID_STRING_ALLOC( 0, "Value");
    wReq.nodesToWrite[0].attributeId = UA_ATTRIBUTEID_VALUE;
    wReq.nodesToWrite[0].value.hasValue = true;
    wReq.nodesToWrite[0].value.sourceTimestamp = dt;
    wReq.nodesToWrite[0].value.value.type = &UA_TYPES[UA_TYPES_FLOAT];
    wReq.nodesToWrite[0].value.value.storageType = UA_VARIANT_DATA_NODELETE;
    UA_Variant_setArrayCopy( &wReq.nodesToWrite[0].value.value, batch_data, CARBON_Z, &UA_TYPES[UA_TYPES_FLOAT]);

    wReq.nodesToWrite[1].nodeId = UA_NODEID_STRING_ALLOC( 0, "ValueIntegral");
    wReq.nodesToWrite[1].attributeId = UA_ATTRIBUTEID_VALUE;
    wReq.nodesToWrite[1].value.hasValue = true;
    wReq.nodesToWrite[1].value.sourceTimestamp = dt;
    wReq.nodesToWrite[1].value.value.type = &UA_TYPES[UA_TYPES_FLOAT];
    wReq.nodesToWrite[1].value.value.storageType = UA_VARIANT_DATA_NODELETE;
    UA_Variant_setArrayCopy( &wReq.nodesToWrite[1].value.value, mean_data, CARBON_Z, &UA_TYPES[UA_TYPES_FLOAT]);


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
OpcUaClient::writeHeartBeatValue(int heart_beat)
{
    if (!isConnected())
        return false;

    bool result = false;

    UA_UInt32 value = heart_beat;

    UA_WriteRequest wReq;
    UA_WriteRequest_init(&wReq);
    wReq.nodesToWrite = UA_WriteValue_new();
    wReq.nodesToWriteSize = 1;
    wReq.nodesToWrite[0].nodeId = UA_NODEID_STRING_ALLOC( 0, "HeartBeat");
    wReq.nodesToWrite[0].attributeId = UA_ATTRIBUTEID_VALUE;
    wReq.nodesToWrite[0].value.hasValue = true;
    wReq.nodesToWrite[0].value.value.type = &UA_TYPES[UA_TYPES_UINT32];
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
OpcUaClient::writeCurrentStateValue(int current_state)
{
    if (!isConnected())
        return false;

    bool result = false;

    UA_Int16 value = current_state;

    UA_WriteRequest wReq;
    UA_WriteRequest_init(&wReq);
    wReq.nodesToWrite = UA_WriteValue_new();
    wReq.nodesToWriteSize = 1;
    wReq.nodesToWrite[0].nodeId = UA_NODEID_STRING_ALLOC( 0, "State");
    wReq.nodesToWrite[0].attributeId = UA_ATTRIBUTEID_VALUE;
    wReq.nodesToWrite[0].value.hasValue = true;
    wReq.nodesToWrite[0].value.value.type = &UA_TYPES[UA_TYPES_INT16];
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
        UA_Int16 ext_command = *(UA_Int16*) value->value.data;
        if (local_client_ptr) {
            local_client_ptr->signalExternalCommandChanged(ext_command);
        }
    }
}
#endif

void
OpcUaClient::onConnectCallback( UA_Client* /* client */, void* userdata,
    UA_UInt32 /* requestId */, void* status)
{
    UA_StatusCode status_code = *(UA_StatusCode*)status;

    local_client_ptr = reinterpret_cast<OpcUaClient*>(userdata);
    if (local_client_ptr && (status_code == UA_STATUSCODE_GOOD)) {
        if (local_client_ptr->initExternalCommandSubscription())
            local_client_ptr->signalConnected();
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
OpcUaClient::signalExternalCommandChanged(int value)
{
    emit externalCommandChanged(value);
}

bool
OpcUaClient::initExternalCommandSubscription()
{
    bool result = false;
#ifdef UA_ENABLE_SUBSCRIPTIONS
    UA_CreateSubscriptionResponse response;
    response = UA_Client_Subscriptions_create( client, request, NULL, NULL, NULL);

//    UA_UInt32 subId = response.subscriptionId;
    if (response.responseHeader.serviceResult == UA_STATUSCODE_GOOD) {

        UA_MonitoredItemCreateResult monResponse;
        monResponse = UA_Client_MonitoredItems_createDataChange( client,
            response.subscriptionId, UA_TIMESTAMPSTORETURN_BOTH,
            monRequest, NULL, onSubscriptionExtCommandValueChanged, NULL);

        if (monResponse.statusCode == UA_STATUSCODE_GOOD)
            result = true;
    }
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
