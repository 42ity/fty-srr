/*  =========================================================================
    fty-srr - Save and restore and reset agent for 42ITy ecosystem

    Copyright (C) 2014 - 2020 Eaton

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
    =========================================================================
*/

#ifndef FTY_SRR_H_H_INCLUDED
#define FTY_SRR_H_H_INCLUDED

// clang-format off
//  Include the project library file
#include "fty_srr_library.h"

//  SRR agent configuration
constexpr auto REQUEST_TIMEOUT_KEY          = "requestTimeOut";
constexpr auto AGENT_NAME_KEY               = "agentName";
constexpr auto AGENT_NAME                   = "fty-srr";
constexpr auto ENDPOINT_KEY                 = "endPoint";
constexpr auto DEFAULT_ENDPOINT             = "ipc://@/malamute";
constexpr auto DEFAULT_LOG_CONFIG           = "/etc/fty/ftylog.cfg";
constexpr auto SRR_QUEUE_NAME_KEY           = "queueName";
constexpr auto SRR_MSG_QUEUE_NAME           = "ETN.Q.IPMCORE.SRR";
// Config agent definition  
constexpr auto CONFIG_AGENT_NAME            = "fty-config";
constexpr auto CONFIG_MSG_QUEUE_NAME        = "ETN.Q.IPMCORE.CONFIG";
// EMC4J agent definition   
constexpr auto EMC4J_AGENT_NAME             = "etn-malamute-translator";
constexpr auto EMC4J_MSG_QUEUE_NAME         = "ETN.Q.EMC4J.REQ";
// Security wallet agent definition 
constexpr auto SECU_WALLET_AGENT_NAME       = "security-wallet";
constexpr auto SECU_WALLET_MSG_QUEUE_NAME   = "ETN.Q.IPMCORE.SECUWALLET";
// Asset agent definition
constexpr auto ASSET_AGENT_NAME             = "asset-agent-srr";
constexpr auto ASSET_AGENT_MSG_QUEUE_NAME   = "FTY.Q.ASSET.SRR";
// Features definition
constexpr auto MONITORING_FEATURE_NAME      = "monitoring";
constexpr auto NOTIFICATION_FEATURE_NAME    = "notification";
constexpr auto AUTOMATION_SETTINGS          = "automation-settings";
constexpr auto USER_SESSION_FEATURE_NAME    = "user-session";
constexpr auto DISCOVERY                    = "discovery";
constexpr auto MASS_MANAGEMENT              = "etn-mass-management";
constexpr auto NETWORK                      = "network";
constexpr auto AUTOMATIONS                  = "automations";
constexpr auto VIRTUAL_ASSETS               = "virtual-assets";
constexpr auto SECURITY_WALLET              = "security-wallet";
constexpr auto ASSET_AGENT                  = "asset-agent";
// Common definition                    
constexpr auto SRR_VERSION_KEY              = "version";
constexpr auto ACTIVE_VERSION               = "1.0";
constexpr auto SRR_PREFIX_TRANSLATE_KEY     = "srr_";

#endif
