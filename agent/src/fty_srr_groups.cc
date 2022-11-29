/*  =========================================================================
    fty_srr_worker - Fty srr worker

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

#include "fty_srr_groups.h"
#include <fty_log.h>
#include <algorithm>

namespace srr {

std::string getGroupFromFeature(const std::string& featureName)
{
    for (const auto& group : g_srrGroupMap) {
        const auto& fp = group.second.m_fp;

        auto found = std::find_if(fp.begin(), fp.end(), [&](SrrFeaturePriorityStruct x) {
            return (featureName == x.m_feature);
        });

        if (found != fp.end()) {
            return group.first; // groupId (m_id)
        }
    }

    return std::string{};
}

unsigned int getPriority(const std::string& featureName)
{
    const std::string groupId = getGroupFromFeature(featureName);

    if (groupId.empty()) {
        return 0;
    }

    const auto& featurePriority = g_srrGroupMap.at(groupId).m_fp;
    const auto  found =
        std::find_if(featurePriority.begin(), featurePriority.end(), [&](const SrrFeaturePriorityStruct& fp) {
            return featureName == fp.m_feature;
        });

    if (found != featurePriority.end()) {
        return found->m_priority;
    } else {
        return 0;
    }
}

static auto initSrrFeatures = []() {
    std::map<std::string, SrrFeatureStruct> tmp;

    tmp[F_ALERT_AGENT];
    tmp[F_ALERT_AGENT].m_id          = F_ALERT_AGENT;
    tmp[F_ALERT_AGENT].m_name        = F_ALERT_AGENT;
    tmp[F_ALERT_AGENT].m_description = TRANSLATE_ME("srr_alert-agent");
    tmp[F_ALERT_AGENT].m_agent       = ALERT_AGENT_NAME;
    tmp[F_ALERT_AGENT].m_requiredIn  = {"1.0", "2.0", "2.1", "2.2", "2.3"};
    tmp[F_ALERT_AGENT].m_restart     = true;
    tmp[F_ALERT_AGENT].m_reset       = true;

    tmp[F_ASSET_AGENT];
    tmp[F_ASSET_AGENT].m_id          = F_ASSET_AGENT;
    tmp[F_ASSET_AGENT].m_name        = F_ASSET_AGENT;
    tmp[F_ASSET_AGENT].m_description = TRANSLATE_ME("srr_asset-agent");
    tmp[F_ASSET_AGENT].m_agent       = ASSET_AGENT_NAME;
    tmp[F_ASSET_AGENT].m_requiredIn  = {"1.0", "2.0", "2.1", "2.2", "2.3"};
    tmp[F_ASSET_AGENT].m_restart     = true;
    tmp[F_ASSET_AGENT].m_reset       = true;

    tmp[F_AUTOMATIC_GROUPS];
    tmp[F_AUTOMATIC_GROUPS].m_id          = F_AUTOMATIC_GROUPS;
    tmp[F_AUTOMATIC_GROUPS].m_name        = F_AUTOMATIC_GROUPS;
    tmp[F_AUTOMATIC_GROUPS].m_description = TRANSLATE_ME("srr_automatic-groups");
    tmp[F_AUTOMATIC_GROUPS].m_agent       = AUTOMATIC_GROUPS_NAME;
    tmp[F_AUTOMATIC_GROUPS].m_requiredIn  = {"2.1", "2.2", "2.3"};
    tmp[F_AUTOMATIC_GROUPS].m_restart     = true;
    tmp[F_AUTOMATIC_GROUPS].m_reset       = true;

    tmp[F_AUTOMATION_SETTINGS];
    tmp[F_AUTOMATION_SETTINGS].m_id          = F_AUTOMATION_SETTINGS;
    tmp[F_AUTOMATION_SETTINGS].m_name        = F_AUTOMATION_SETTINGS;
    tmp[F_AUTOMATION_SETTINGS].m_description = TRANSLATE_ME("srr_automation-settings");
    tmp[F_AUTOMATION_SETTINGS].m_agent       = CONFIG_AGENT_NAME;
    tmp[F_AUTOMATION_SETTINGS].m_requiredIn  = {"1.0", "2.0", "2.1", "2.2", "2.3"};
    tmp[F_AUTOMATION_SETTINGS].m_restart     = true;
    tmp[F_AUTOMATION_SETTINGS].m_reset       = false;

    tmp[F_AUTOMATIONS];
    tmp[F_AUTOMATIONS].m_id          = F_AUTOMATIONS;
    tmp[F_AUTOMATIONS].m_name        = F_AUTOMATIONS;
    tmp[F_AUTOMATIONS].m_description = TRANSLATE_ME("srr_automations");
    tmp[F_AUTOMATIONS].m_agent       = EMC4J_AGENT_NAME;
    tmp[F_AUTOMATIONS].m_requiredIn  = {"1.0", "2.0", "2.1", "2.2", "2.3"};
    tmp[F_AUTOMATIONS].m_restart     = true;
    tmp[F_AUTOMATIONS].m_reset       = true;

    tmp[F_DISCOVERY_SETTINGS];
    tmp[F_DISCOVERY_SETTINGS].m_id          = F_DISCOVERY_SETTINGS;
    tmp[F_DISCOVERY_SETTINGS].m_name        = F_DISCOVERY_SETTINGS;
    tmp[F_DISCOVERY_SETTINGS].m_description = TRANSLATE_ME("srr_discovery-settings");
    tmp[F_DISCOVERY_SETTINGS].m_agent       = CONFIG_AGENT_NAME;
    tmp[F_DISCOVERY_SETTINGS].m_requiredIn  = {"2.2", "2.3"};
    tmp[F_DISCOVERY_SETTINGS].m_restart     = false;
    tmp[F_DISCOVERY_SETTINGS].m_reset       = false;

    tmp[F_DISCOVERY_AGENT_SETTINGS];
    tmp[F_DISCOVERY_AGENT_SETTINGS].m_id          = F_DISCOVERY_AGENT_SETTINGS;
    tmp[F_DISCOVERY_AGENT_SETTINGS].m_name        = F_DISCOVERY_AGENT_SETTINGS;
    tmp[F_DISCOVERY_AGENT_SETTINGS].m_description = TRANSLATE_ME("srr_discovery-agent-settings");
    tmp[F_DISCOVERY_AGENT_SETTINGS].m_agent       = CONFIG_AGENT_NAME;
    tmp[F_DISCOVERY_AGENT_SETTINGS].m_requiredIn  = {"2.2", "2.3"};
    tmp[F_DISCOVERY_AGENT_SETTINGS].m_restart     = true;
    tmp[F_DISCOVERY_AGENT_SETTINGS].m_reset       = false;

    tmp[F_MASS_MANAGEMENT];
    tmp[F_MASS_MANAGEMENT].m_id          = F_MASS_MANAGEMENT;
    tmp[F_MASS_MANAGEMENT].m_name        = F_MASS_MANAGEMENT;
    tmp[F_MASS_MANAGEMENT].m_description = TRANSLATE_ME("srr_etn-mass-management");
    tmp[F_MASS_MANAGEMENT].m_agent       = CONFIG_AGENT_NAME;
    tmp[F_MASS_MANAGEMENT].m_requiredIn  = {"1.0", "2.0", "2.1", "2.2", "2.3"};
    tmp[F_MASS_MANAGEMENT].m_restart     = true;
    tmp[F_MASS_MANAGEMENT].m_reset       = false;

    tmp[F_MONITORING_FEATURE_NAME];
    tmp[F_MONITORING_FEATURE_NAME].m_id          = F_MONITORING_FEATURE_NAME;
    tmp[F_MONITORING_FEATURE_NAME].m_name        = F_MONITORING_FEATURE_NAME;
    tmp[F_MONITORING_FEATURE_NAME].m_description = TRANSLATE_ME("srr_monitoring");
    tmp[F_MONITORING_FEATURE_NAME].m_agent       = CONFIG_AGENT_NAME;
    tmp[F_MONITORING_FEATURE_NAME].m_requiredIn  = {"1.0", "2.0", "2.1", "2.2", "2.3"};
    tmp[F_MONITORING_FEATURE_NAME].m_restart     = true;
    tmp[F_MONITORING_FEATURE_NAME].m_reset       = false;

    tmp[F_NETWORK];
    tmp[F_NETWORK].m_id          = F_NETWORK;
    tmp[F_NETWORK].m_name        = F_NETWORK;
    tmp[F_NETWORK].m_description = TRANSLATE_ME("srr_network");
    tmp[F_NETWORK].m_agent       = CONFIG_AGENT_NAME;
    tmp[F_NETWORK].m_requiredIn  = {"1.0", "2.0", "2.1", "2.2", "2.3"};
    tmp[F_NETWORK].m_restart     = true;
    tmp[F_NETWORK].m_reset       = false;

    tmp[F_NETWORK_HOST_NAME];
    tmp[F_NETWORK_HOST_NAME].m_id          = F_NETWORK_HOST_NAME;
    tmp[F_NETWORK_HOST_NAME].m_name        = F_NETWORK_HOST_NAME;
    tmp[F_NETWORK_HOST_NAME].m_description = TRANSLATE_ME("srr_network-host-name");
    tmp[F_NETWORK_HOST_NAME].m_agent       = CONFIG_AGENT_NAME;
    tmp[F_NETWORK_HOST_NAME].m_requiredIn  = {"2.2", "2.3"};
    tmp[F_NETWORK_HOST_NAME].m_restart     = true;
    tmp[F_NETWORK_HOST_NAME].m_reset       = false;

    tmp[F_NETWORK_AGENT_SETTINGS];
    tmp[F_NETWORK_AGENT_SETTINGS].m_id          = F_NETWORK_AGENT_SETTINGS;
    tmp[F_NETWORK_AGENT_SETTINGS].m_name        = F_NETWORK_AGENT_SETTINGS;
    tmp[F_NETWORK_AGENT_SETTINGS].m_description = TRANSLATE_ME("srr_network-agent-settings");
    tmp[F_NETWORK_AGENT_SETTINGS].m_agent       = CONFIG_AGENT_NAME;
    tmp[F_NETWORK_AGENT_SETTINGS].m_requiredIn  = {"2.2", "2.3"};
    tmp[F_NETWORK_AGENT_SETTINGS].m_restart     = true;
    tmp[F_NETWORK_AGENT_SETTINGS].m_reset       = false;

    tmp[F_NETWORK_PROXY];
    tmp[F_NETWORK_PROXY].m_id          = F_NETWORK_PROXY;
    tmp[F_NETWORK_PROXY].m_name        = F_NETWORK_PROXY;
    tmp[F_NETWORK_PROXY].m_description = TRANSLATE_ME("srr_network-proxy");
    tmp[F_NETWORK_PROXY].m_agent       = CONFIG_AGENT_NAME;
    tmp[F_NETWORK_PROXY].m_requiredIn  = {"2.3"};
    tmp[F_NETWORK_PROXY].m_restart     = true;
    tmp[F_NETWORK_PROXY].m_reset       = false;

    tmp[F_NOTIFICATION_FEATURE_NAME];
    tmp[F_NOTIFICATION_FEATURE_NAME].m_id          = F_NOTIFICATION_FEATURE_NAME;
    tmp[F_NOTIFICATION_FEATURE_NAME].m_name        = F_NOTIFICATION_FEATURE_NAME;
    tmp[F_NOTIFICATION_FEATURE_NAME].m_description = TRANSLATE_ME("srr_notification");
    tmp[F_NOTIFICATION_FEATURE_NAME].m_agent       = CONFIG_AGENT_NAME;
    tmp[F_NOTIFICATION_FEATURE_NAME].m_requiredIn  = {"1.0", "2.0", "2.1", "2.2", "2.3"};
    tmp[F_NOTIFICATION_FEATURE_NAME].m_restart     = true;
    tmp[F_NOTIFICATION_FEATURE_NAME].m_reset       = false;

    tmp[F_RSYSLOG_FEATURE_NAME];
    tmp[F_RSYSLOG_FEATURE_NAME].m_id          = F_RSYSLOG_FEATURE_NAME;
    tmp[F_RSYSLOG_FEATURE_NAME].m_name        = F_RSYSLOG_FEATURE_NAME;
    tmp[F_RSYSLOG_FEATURE_NAME].m_description = TRANSLATE_ME("srr_rsyslog");
    tmp[F_RSYSLOG_FEATURE_NAME].m_agent       = RSYSLOG_AGENT_NAME;
    tmp[F_RSYSLOG_FEATURE_NAME].m_requiredIn  = {"2.1", "2.2", "2.3"};
    tmp[F_RSYSLOG_FEATURE_NAME].m_restart     = true;
    tmp[F_RSYSLOG_FEATURE_NAME].m_reset       = true;

    tmp[F_SECURITY_WALLET];
    tmp[F_SECURITY_WALLET].m_id          = F_SECURITY_WALLET;
    tmp[F_SECURITY_WALLET].m_name        = F_SECURITY_WALLET;
    tmp[F_SECURITY_WALLET].m_description = TRANSLATE_ME("srr_security-wallet");
    tmp[F_SECURITY_WALLET].m_agent       = SECU_WALLET_AGENT_NAME;
    tmp[F_SECURITY_WALLET].m_requiredIn  = {"1.0", "2.0", "2.1", "2.2", "2.3"};
    tmp[F_SECURITY_WALLET].m_restart     = true;
    tmp[F_SECURITY_WALLET].m_reset       = false;

    tmp[F_SECURITY_WALLET_CAM];
    tmp[F_SECURITY_WALLET_CAM].m_id          = F_SECURITY_WALLET_CAM;
    tmp[F_SECURITY_WALLET_CAM].m_name        = F_SECURITY_WALLET_CAM;
    tmp[F_SECURITY_WALLET_CAM].m_description = TRANSLATE_ME("srr_credential-asset-mapping");
    tmp[F_SECURITY_WALLET_CAM].m_agent       = SECU_WALLET_CAM_AGENT_NAME;
    tmp[F_SECURITY_WALLET_CAM].m_requiredIn  = {"2.3"};
    tmp[F_SECURITY_WALLET_CAM].m_restart     = true;
    tmp[F_SECURITY_WALLET_CAM].m_reset       = false;

    tmp[F_USER_SESSION_MANAGEMENT_FEATURE_NAME];
    tmp[F_USER_SESSION_MANAGEMENT_FEATURE_NAME].m_id          = F_USER_SESSION_MANAGEMENT_FEATURE_NAME;
    tmp[F_USER_SESSION_MANAGEMENT_FEATURE_NAME].m_name        = F_USER_SESSION_MANAGEMENT_FEATURE_NAME;
    tmp[F_USER_SESSION_MANAGEMENT_FEATURE_NAME].m_description = TRANSLATE_ME("srr_user-session-management");
    tmp[F_USER_SESSION_MANAGEMENT_FEATURE_NAME].m_agent       = USM_AGENT_NAME;
    tmp[F_USER_SESSION_MANAGEMENT_FEATURE_NAME].m_requiredIn  = {"2.1", "2.2", "2.3"};
    tmp[F_USER_SESSION_MANAGEMENT_FEATURE_NAME].m_restart     = true;
    tmp[F_USER_SESSION_MANAGEMENT_FEATURE_NAME].m_reset       = false;

    tmp[F_VIRTUAL_ASSETS];
    tmp[F_VIRTUAL_ASSETS].m_id          = F_VIRTUAL_ASSETS;
    tmp[F_VIRTUAL_ASSETS].m_name        = F_VIRTUAL_ASSETS;
    tmp[F_VIRTUAL_ASSETS].m_description = TRANSLATE_ME("srr_virtual-assets");
    tmp[F_VIRTUAL_ASSETS].m_agent       = EMC4J_AGENT_NAME;
    tmp[F_VIRTUAL_ASSETS].m_requiredIn  = {"1.0", "2.0", "2.1", "2.2", "2.3"};
    tmp[F_VIRTUAL_ASSETS].m_restart     = true;
    tmp[F_VIRTUAL_ASSETS].m_reset       = true;

    tmp[F_VIRTUALIZATION_SETTINGS];
    tmp[F_VIRTUALIZATION_SETTINGS].m_id          = F_VIRTUALIZATION_SETTINGS;
    tmp[F_VIRTUALIZATION_SETTINGS].m_name        = F_VIRTUALIZATION_SETTINGS;
    tmp[F_VIRTUALIZATION_SETTINGS].m_description = TRANSLATE_ME("srr_virtualization-settings");
    tmp[F_VIRTUALIZATION_SETTINGS].m_agent       = EMC4J_AGENT_NAME;
    tmp[F_VIRTUALIZATION_SETTINGS].m_requiredIn  = {"2.1", "2.2", "2.3"};
    tmp[F_VIRTUALIZATION_SETTINGS].m_restart     = true;
    tmp[F_VIRTUALIZATION_SETTINGS].m_reset       = true;

    tmp[F_AI_SETTINGS];
    tmp[F_AI_SETTINGS].m_id          = F_AI_SETTINGS;
    tmp[F_AI_SETTINGS].m_name        = F_AI_SETTINGS;
    tmp[F_AI_SETTINGS].m_description = TRANSLATE_ME("srr_ai-settings");
    tmp[F_AI_SETTINGS].m_agent       = EMC4J_AGENT_NAME;
    tmp[F_AI_SETTINGS].m_requiredIn  = {"2.1", "2.2", "2.3"};
    tmp[F_AI_SETTINGS].m_restart     = true;
    tmp[F_AI_SETTINGS].m_reset       = true;

    logDebug("initSrrFeatures (size: {})", tmp.size());
    return tmp;
};

static auto initSrrGroups = []() {
    std::map<std::string, SrrGroupStruct> tmp;
    unsigned int restoreOrder = 0;

    // assets group, create and add features
    tmp[G_ASSETS];
    tmp[G_ASSETS].m_id           = G_ASSETS;
    tmp[G_ASSETS].m_name         = G_ASSETS;
    tmp[G_ASSETS].m_description  = TRANSLATE_ME("srr_group-assets");
    tmp[G_ASSETS].m_restoreOrder = restoreOrder++;

    tmp[G_ASSETS].m_fp.push_back(SrrFeaturePriorityStruct(F_SECURITY_WALLET, 1));
    tmp[G_ASSETS].m_fp.push_back(SrrFeaturePriorityStruct(F_SECURITY_WALLET_CAM, 2));
    tmp[G_ASSETS].m_fp.push_back(SrrFeaturePriorityStruct(F_ASSET_AGENT, 3));
    tmp[G_ASSETS].m_fp.push_back(SrrFeaturePriorityStruct(F_AUTOMATIC_GROUPS, 4));
    tmp[G_ASSETS].m_fp.push_back(SrrFeaturePriorityStruct(F_VIRTUAL_ASSETS, 5));
    tmp[G_ASSETS].m_fp.push_back(SrrFeaturePriorityStruct(F_ALERT_AGENT, 6));
    tmp[G_ASSETS].m_fp.push_back(SrrFeaturePriorityStruct(F_AUTOMATION_SETTINGS, 7));
    tmp[G_ASSETS].m_fp.push_back(SrrFeaturePriorityStruct(F_AUTOMATIONS, 8));

    // discovery group, create &nd add features
    tmp[G_DISCOVERY];
    tmp[G_DISCOVERY].m_id           = G_DISCOVERY;
    tmp[G_DISCOVERY].m_name         = G_DISCOVERY;
    tmp[G_DISCOVERY].m_description  = TRANSLATE_ME("srr_group-discovery");
    tmp[G_DISCOVERY].m_restoreOrder = restoreOrder++;

    tmp[G_DISCOVERY].m_fp.push_back(SrrFeaturePriorityStruct(F_DISCOVERY_SETTINGS, 1));
    tmp[G_DISCOVERY].m_fp.push_back(SrrFeaturePriorityStruct(F_DISCOVERY_AGENT_SETTINGS, 2));

    // mass management group, create and add features
    tmp[G_MASS_MANAGEMENT];
    tmp[G_MASS_MANAGEMENT].m_id           = G_MASS_MANAGEMENT;
    tmp[G_MASS_MANAGEMENT].m_name         = G_MASS_MANAGEMENT;
    tmp[G_MASS_MANAGEMENT].m_description  = TRANSLATE_ME("srr_group-mass-management");
    tmp[G_MASS_MANAGEMENT].m_restoreOrder = restoreOrder++;

    tmp[G_MASS_MANAGEMENT].m_fp.push_back(SrrFeaturePriorityStruct(F_MASS_MANAGEMENT, 1));

    // monitoring group, create and add features
    tmp[G_MONITORING];
    tmp[G_MONITORING].m_id           = G_MONITORING;
    tmp[G_MONITORING].m_name         = G_MONITORING;
    tmp[G_MONITORING].m_description  = TRANSLATE_ME("srr_group-monitoring-feature-name");
    tmp[G_MONITORING].m_restoreOrder = restoreOrder++;

    tmp[G_MONITORING].m_fp.push_back(SrrFeaturePriorityStruct(F_MONITORING_FEATURE_NAME, 1));

    // rsyslog group, create and add features
    tmp[G_RSYSLOG];
    tmp[G_RSYSLOG].m_id           = G_RSYSLOG;
    tmp[G_RSYSLOG].m_name         = G_RSYSLOG;
    tmp[G_RSYSLOG].m_description  = TRANSLATE_ME("srr_group-remote-syslog");
    tmp[G_RSYSLOG].m_restoreOrder = restoreOrder++;

    tmp[G_RSYSLOG].m_fp.push_back(SrrFeaturePriorityStruct(F_RSYSLOG_FEATURE_NAME, 1));

    // network group, create and add features
    tmp[G_NETWORK];
    tmp[G_NETWORK].m_id           = G_NETWORK;
    tmp[G_NETWORK].m_name         = G_NETWORK;
    tmp[G_NETWORK].m_description  = TRANSLATE_ME("srr_group-network");
    tmp[G_NETWORK].m_restoreOrder = restoreOrder++;

    tmp[G_NETWORK].m_fp.push_back(SrrFeaturePriorityStruct(F_NETWORK, 1));
    tmp[G_NETWORK].m_fp.push_back(SrrFeaturePriorityStruct(F_NETWORK_HOST_NAME, 2));
    tmp[G_NETWORK].m_fp.push_back(SrrFeaturePriorityStruct(F_NETWORK_AGENT_SETTINGS, 3));
    tmp[G_NETWORK].m_fp.push_back(SrrFeaturePriorityStruct(F_NETWORK_PROXY, 4));

    // notification group, create and add features
    tmp[G_NOTIFICATION];
    tmp[G_NOTIFICATION].m_id           = G_NOTIFICATION;
    tmp[G_NOTIFICATION].m_name         = G_NOTIFICATION;
    tmp[G_NOTIFICATION].m_description  = TRANSLATE_ME("srr_group-notification-feature-name");
    tmp[G_NOTIFICATION].m_restoreOrder = restoreOrder++;

    tmp[G_NOTIFICATION].m_fp.push_back(SrrFeaturePriorityStruct(F_NOTIFICATION_FEATURE_NAME, 1));

    // virtualization settings group, create and add features
    tmp[G_VIRTUALIZATION_SETTINGS];
    tmp[G_VIRTUALIZATION_SETTINGS].m_id           = G_VIRTUALIZATION_SETTINGS;
    tmp[G_VIRTUALIZATION_SETTINGS].m_name         = G_VIRTUALIZATION_SETTINGS;
    tmp[G_VIRTUALIZATION_SETTINGS].m_description  = TRANSLATE_ME("srr_group-virtualization-settings");
    tmp[G_VIRTUALIZATION_SETTINGS].m_restoreOrder = restoreOrder++;

    tmp[G_VIRTUALIZATION_SETTINGS].m_fp.push_back(SrrFeaturePriorityStruct(F_VIRTUALIZATION_SETTINGS, 1));

    // ai settings group, create and add features
    tmp[G_AI_SETTINGS];
    tmp[G_AI_SETTINGS].m_id           = G_AI_SETTINGS;
    tmp[G_AI_SETTINGS].m_name         = G_AI_SETTINGS;
    tmp[G_AI_SETTINGS].m_description  = TRANSLATE_ME("srr_group-ai-settings");
    tmp[G_AI_SETTINGS].m_restoreOrder = restoreOrder++;

    tmp[G_AI_SETTINGS].m_fp.push_back(SrrFeaturePriorityStruct(F_AI_SETTINGS, 1));

    // user session management group, create and add features
    tmp[G_USER_SESSION_MANAGEMENT];
    tmp[G_USER_SESSION_MANAGEMENT].m_id           = G_USER_SESSION_MANAGEMENT;
    tmp[G_USER_SESSION_MANAGEMENT].m_name         = G_USER_SESSION_MANAGEMENT;
    tmp[G_USER_SESSION_MANAGEMENT].m_description  = TRANSLATE_ME("srr_group-user-session-management");
    tmp[G_USER_SESSION_MANAGEMENT].m_restoreOrder = restoreOrder++;

    tmp[G_USER_SESSION_MANAGEMENT].m_fp.push_back(SrrFeaturePriorityStruct(F_USER_SESSION_MANAGEMENT_FEATURE_NAME, 1));

    logDebug("initSrrGroups (size: {})", tmp.size());
    return tmp;
};

const std::map<std::string, SrrFeatureStruct> g_srrFeatureMap = initSrrFeatures();

const std::map<std::string, SrrGroupStruct> g_srrGroupMap = initSrrGroups();

// agents that implements SRR queue messaging
// see SrrFeatureStruct.m_agent
const std::map<const std::string, const std::string> g_agentToQueue = {
    { ALERT_AGENT_NAME,           ALERT_AGENT_MSG_QUEUE_NAME },
    { ASSET_AGENT_NAME,           ASSET_AGENT_MSG_QUEUE_NAME },
    { AUTOMATIC_GROUPS_NAME,      AUTOMATIC_GROUPS_QUEUE_NAME },
    { CONFIG_AGENT_NAME,          CONFIG_MSG_QUEUE_NAME },
    { EMC4J_AGENT_NAME,           EMC4J_MSG_QUEUE_NAME },
    { RSYSLOG_AGENT_NAME,         RSYSLOG_AGENT_MSG_QUEUE_NAME },
    { SECU_WALLET_AGENT_NAME,     SECU_WALLET_MSG_QUEUE_NAME },
    { SECU_WALLET_CAM_AGENT_NAME, SECU_WALLET_CAM_MSG_QUEUE_NAME },
    { USM_AGENT_NAME,             USM_AGENT_MSG_QUEUE_NAME },
};

} // namespace srr