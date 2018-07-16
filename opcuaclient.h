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

#include <open62541.h>

void onConnectCallback( UA_Client* client, void* userdata,
    UA_UInt32 requestId, void* status);
void onReadExtCommandAttributeCallback( UA_Client* client, void* userdata,
    UA_UInt32 requestId, UA_Variant* var);

#ifdef UA_ENABLE_SUBSCRIPTIONS
void
onSubscriptionExtCommandValueChanged( UA_Client* client, UA_UInt32 subId, void* subContext,
    UA_UInt32 monId, void* monContext, UA_DataValue* value);
#endif
