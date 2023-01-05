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

#pragma once

#include <set>
#include <string>

//  SRR agent configuration
constexpr auto REQUEST_TIMEOUT_KEY   = "requestTimeOut";
constexpr auto AGENT_NAME_KEY        = "agentName";
constexpr auto ENDPOINT_KEY          = "endPoint";
constexpr auto SRR_QUEUE_NAME_KEY    = "queueName";
constexpr auto SRR_VERSION_KEY       = "version";
constexpr auto SRR_ENABLE_REBOOT_KEY = "enableReboot";

// SRR agent conf. defaults
constexpr auto REQUEST_TIMEOUT_DEFAULT   = "600000"; //ms
constexpr auto AGENT_NAME                = "fty-srr";
constexpr auto DEFAULT_ENDPOINT          = "ipc://@/malamute";
constexpr auto SRR_MSG_QUEUE_NAME        = "ETN.Q.IPMCORE.SRR";
constexpr auto SRR_ACTIVE_VERSION        = "2.3";
constexpr auto SRR_ENABLE_REBOOT_DEFAULT = "true";
constexpr auto SRR_PREFIX_TRANSLATE_KEY  = "srr_";

//set of all released versions
//see SRR_ACTIVE_VERSION IS_VERSION_1 IS_VERSION_2
const auto SRR_VERSION_ALL{std::set<std::string>({"1.0", "2.0", "2.1", "2.2", "2.3"})};

// AGENTS AND QUEUES
// Config agent definition
constexpr auto CONFIG_AGENT_NAME     = "fty-config";
constexpr auto CONFIG_MSG_QUEUE_NAME = "ETN.Q.IPMCORE.CONFIG";
// EMC4J agent definition
constexpr auto EMC4J_AGENT_NAME     = "etn-malamute-translator";
constexpr auto EMC4J_MSG_QUEUE_NAME = "ETN.Q.EMC4J.REQ";
// Security wallet agent definition
constexpr auto SECU_WALLET_AGENT_NAME     = "security-wallet";
constexpr auto SECU_WALLET_MSG_QUEUE_NAME = "ETN.Q.IPMCORE.SECUWALLET";
// Security wallet CAM agent definition
constexpr auto SECU_WALLET_CAM_AGENT_NAME     = "credential-asset-mapping-srr";
constexpr auto SECU_WALLET_CAM_MSG_QUEUE_NAME = "ETN.Q.IPMCORE.CREDASSETMAPPING";
// Asset agent definition
constexpr auto ASSET_AGENT_NAME           = "asset-agent-srr";
constexpr auto ASSET_AGENT_MSG_QUEUE_NAME = "FTY.Q.ASSET.SRR";
// Automatic groups
constexpr auto AUTOMATIC_GROUPS_NAME       = "automatic-group";
constexpr auto AUTOMATIC_GROUPS_QUEUE_NAME = "FTY.Q.GROUP.SRR";
// Alert agent definition
constexpr auto ALERT_AGENT_NAME           = "alert-agent-srr";
constexpr auto ALERT_AGENT_MSG_QUEUE_NAME = "FTY.Q.ALERT.SRR";
// User Session Management agent definition
constexpr auto USM_AGENT_NAME           = "usm-agent-srr";
constexpr auto USM_AGENT_MSG_QUEUE_NAME = "ETN.Q.IPMCORE.USMSRR";
// Remote syslog agent definition
constexpr auto RSYSLOG_AGENT_NAME           = "etn-syslog";
constexpr auto RSYSLOG_AGENT_MSG_QUEUE_NAME = "FTY.Q.SYSLOG.SRR";

// GROUPS
constexpr auto G_ASSETS                  = "group-assets";
constexpr auto G_DISCOVERY               = "group-discovery-ng";
constexpr auto G_MASS_MANAGEMENT         = "group-mass-management";
constexpr auto G_MONITORING              = "group-monitoring-feature-name";
constexpr auto G_NETWORK                 = "group-network";
constexpr auto G_NOTIFICATION            = "group-notification-feature-name";
constexpr auto G_RSYSLOG                 = "group-remote-syslog";
constexpr auto G_USER_SESSION_MANAGEMENT = "group-user-session-management";
constexpr auto G_VIRTUALIZATION_SETTINGS = "group-virtualization-settings";
constexpr auto G_AI_SETTINGS             = "group-ai-settings";
constexpr auto G_DATETIME_SETTINGS       = "group-datetime-settings";

// FEATURES
constexpr auto F_ALERT_AGENT                          = "alert-agent";
constexpr auto F_ASSET_AGENT                          = "asset-agent";
constexpr auto F_AUTOMATIC_GROUPS                     = "automatic-groups";
constexpr auto F_AUTOMATION_SETTINGS                  = "automation-settings";
constexpr auto F_AUTOMATIONS                          = "automations";
constexpr auto F_DISCOVERY_SETTINGS                   = "discovery-ng-settings";
constexpr auto F_DISCOVERY_AGENT_SETTINGS             = "discovery-ng-agent-settings";
constexpr auto F_MASS_MANAGEMENT                      = "etn-mass-management";
constexpr auto F_MONITORING_FEATURE_NAME              = "monitoring";
constexpr auto F_NETWORK                              = "network";
constexpr auto F_NETWORK_HOST_NAME                    = "network-host-name";
constexpr auto F_NETWORK_AGENT_SETTINGS               = "network-agent-settings";
constexpr auto F_NETWORK_PROXY                        = "network-proxy";
constexpr auto F_NOTIFICATION_FEATURE_NAME            = "notification";
constexpr auto F_RSYSLOG_FEATURE_NAME                 = "rsyslog";
constexpr auto F_SECURITY_WALLET                      = "security-wallet";
constexpr auto F_SECURITY_WALLET_CAM                  = "credential-asset-mapping";
constexpr auto F_USER_SESSION_MANAGEMENT_FEATURE_NAME = "user-session-management";
constexpr auto F_VIRTUAL_ASSETS                       = "virtual-assets";
constexpr auto F_VIRTUALIZATION_SETTINGS              = "virtualization-settings";
constexpr auto F_AI_SETTINGS                          = "ai-settings";
constexpr auto F_TIMEZONE_SETTINGS                    = "timezone-settings";
constexpr auto F_NTP_SETTINGS                         = "ntp-settings";
