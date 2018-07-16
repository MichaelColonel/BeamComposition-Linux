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

#include "opcuanodes.h"
#include "opcuaclient.h"

namespace {

const UA_NodeId NODE_ID_SPECTRUM_SYSTEM = UA_NODEID_STRING( 0, "RBS.Prectrum.01");
const UA_NodeId NODE_ID_EXTERNAL_COMMAND = UA_NODEID_STRING( 0, "Command");
const UA_NodeId NODE_ID_CHARGE_VALUE = UA_NODEID_STRING( 0, "Value");
const UA_NodeId NODE_ID_CHARGE_VALUE_INTEGRAL = UA_NODEID_STRING( 0, "ValueIntegral");
const UA_NodeId NODE_ID_HEART_BEAT = UA_NODEID_STRING( 0, "HeartBeat");
const UA_NodeId NODE_ID_STATE = UA_NODEID_STRING( 0, "State");

}

#ifdef UA_ENABLE_SUBSCRIPTIONS
void
onSubscriptionExtCommandValueChanged( UA_Client* /* client */,
    UA_UInt32 /* subId */, void* /* subContext */, UA_UInt32 /* monId */,
    void* /* monContext */, UA_DataValue* value)
{
    if(UA_Variant_hasScalarType( &value->value, &UA_TYPES[UA_TYPES_INT16])) {
        UA_Int16 ext_command = *(UA_Int16*) value->value.data;
    }
}
#endif

void
onConnectCallback( UA_Client* client, void* userdata,
    UA_UInt32 requestId, void* status)
{
    UA_StatusCode status_code = *(UA_StatusCode*)status;

    void* ptr = reinterpret_cast<void*>(userdata);
    if (ptr && (status_code == UA_STATUSCODE_GOOD)) {
    }
    else if (ptr) {
    }
}

void
onReadExtCommandAttributeCallback( UA_Client* client, void* userdata,
    UA_UInt32 requestId, UA_Variant* var)
{

}
