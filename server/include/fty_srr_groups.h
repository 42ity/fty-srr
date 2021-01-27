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

#pragma once

#include "fty-srr.h"

#include <fty_common_macros.h>
#include <map>
#include <string>
#include <vector>

namespace srr
{
std::string getGroupFromFeature(const std::string& featureName);
unsigned int getPriority(const std::string& featureName);

typedef struct SrrFeatureStruct
{
    std::string m_id;
    std::string m_name;
    std::string m_description;

    std::string m_agent;

    bool m_restart;
    bool m_reset;
} SrrFeatureStruct;

typedef struct SrrFeaturePriorityStruct
{
    SrrFeaturePriorityStruct(const std::string& f, unsigned int p) : m_feature(f), m_priority(p) {};
    std::string m_feature;
    unsigned int m_priority;
} SrrFeaturePriorityStruct;

typedef struct SrrGroupStruct
{
    std::string m_id;
    std::string m_name;
    std::string m_description;

    std::vector<SrrFeaturePriorityStruct> m_fp;
} SrrGroupStruct;

auto initSrrFeatures = [&]() {
    std::map<std::string, SrrFeatureStruct> tmp;

    tmp[F_ALERT_AGENT];
    tmp[F_ALERT_AGENT].m_id = F_ALERT_AGENT;
    tmp[F_ALERT_AGENT].m_name = F_ALERT_AGENT;
    tmp[F_ALERT_AGENT].m_description = TRANSLATE_ME("srr_alert-agent");
    tmp[F_ALERT_AGENT].m_agent = ALERT_AGENT_NAME;
    tmp[F_ALERT_AGENT].m_restart = true;
    tmp[F_ALERT_AGENT].m_reset = true;

    tmp[F_ASSET_AGENT];
    tmp[F_ASSET_AGENT].m_id = F_ASSET_AGENT;
    tmp[F_ASSET_AGENT].m_name = F_ASSET_AGENT;
    tmp[F_ASSET_AGENT].m_description = TRANSLATE_ME("srr_asset-agent");
    tmp[F_ASSET_AGENT].m_agent = ASSET_AGENT_NAME;
    tmp[F_ASSET_AGENT].m_restart = true;
    tmp[F_ASSET_AGENT].m_reset = true;

    tmp[F_AUTOMATION_SETTINGS];
    tmp[F_AUTOMATION_SETTINGS].m_id = F_AUTOMATION_SETTINGS;
    tmp[F_AUTOMATION_SETTINGS].m_name = F_AUTOMATION_SETTINGS;
    tmp[F_AUTOMATION_SETTINGS].m_description = TRANSLATE_ME("srr_automation-settings");
    tmp[F_AUTOMATION_SETTINGS].m_agent = CONFIG_AGENT_NAME;
    tmp[F_AUTOMATION_SETTINGS].m_restart = true;
    tmp[F_AUTOMATION_SETTINGS].m_reset = false;

    tmp[F_AUTOMATIONS];
    tmp[F_AUTOMATIONS].m_id = F_AUTOMATIONS;
    tmp[F_AUTOMATIONS].m_name = F_AUTOMATIONS;
    tmp[F_AUTOMATIONS].m_description = TRANSLATE_ME("srr_automations");
    tmp[F_AUTOMATIONS].m_agent = EMC4J_AGENT_NAME;
    tmp[F_AUTOMATIONS].m_restart = true;
    tmp[F_AUTOMATIONS].m_reset = true;

    tmp[F_DISCOVERY];
    tmp[F_DISCOVERY].m_id = F_DISCOVERY;
    tmp[F_DISCOVERY].m_name = F_DISCOVERY;
    tmp[F_DISCOVERY].m_description = TRANSLATE_ME("srr_discovery");
    tmp[F_DISCOVERY].m_agent = CONFIG_AGENT_NAME;
    tmp[F_DISCOVERY].m_restart = true;
    tmp[F_DISCOVERY].m_reset = false;

    tmp[F_MASS_MANAGEMENT];
    tmp[F_MASS_MANAGEMENT].m_id = F_MASS_MANAGEMENT;
    tmp[F_MASS_MANAGEMENT].m_name = F_MASS_MANAGEMENT;
    tmp[F_MASS_MANAGEMENT].m_description = TRANSLATE_ME("srr_etn-mass-management");
    tmp[F_MASS_MANAGEMENT].m_agent = CONFIG_AGENT_NAME;
    tmp[F_MASS_MANAGEMENT].m_restart = true;
    tmp[F_MASS_MANAGEMENT].m_reset = false;

    tmp[F_MONITORING_FEATURE_NAME];
    tmp[F_MONITORING_FEATURE_NAME].m_id = F_MONITORING_FEATURE_NAME;
    tmp[F_MONITORING_FEATURE_NAME].m_name = F_MONITORING_FEATURE_NAME;
    tmp[F_MONITORING_FEATURE_NAME].m_description = TRANSLATE_ME("srr_monitoring");
    tmp[F_MONITORING_FEATURE_NAME].m_agent = CONFIG_AGENT_NAME;
    tmp[F_MONITORING_FEATURE_NAME].m_restart = true;
    tmp[F_MONITORING_FEATURE_NAME].m_reset = false;

    tmp[F_NETWORK];
    tmp[F_NETWORK].m_id = F_NETWORK;
    tmp[F_NETWORK].m_name = F_NETWORK;
    tmp[F_NETWORK].m_description = TRANSLATE_ME("srr_network");
    tmp[F_NETWORK].m_agent = CONFIG_AGENT_NAME;
    tmp[F_NETWORK].m_restart = true;
    tmp[F_NETWORK].m_reset = false;

    tmp[F_NOTIFICATION_FEATURE_NAME];
    tmp[F_NOTIFICATION_FEATURE_NAME].m_id = F_NOTIFICATION_FEATURE_NAME;
    tmp[F_NOTIFICATION_FEATURE_NAME].m_name = F_NOTIFICATION_FEATURE_NAME;
    tmp[F_NOTIFICATION_FEATURE_NAME].m_description = TRANSLATE_ME("srr_notification");
    tmp[F_NOTIFICATION_FEATURE_NAME].m_agent = CONFIG_AGENT_NAME;
    tmp[F_NOTIFICATION_FEATURE_NAME].m_restart = true;
    tmp[F_NOTIFICATION_FEATURE_NAME].m_reset = false;


    tmp[F_SECURITY_WALLET];
    tmp[F_SECURITY_WALLET].m_id = F_SECURITY_WALLET;
    tmp[F_SECURITY_WALLET].m_name = F_SECURITY_WALLET;
    tmp[F_SECURITY_WALLET].m_description = TRANSLATE_ME("srr_security-wallet");
    tmp[F_SECURITY_WALLET].m_agent = SECU_WALLET_AGENT_NAME;
    tmp[F_SECURITY_WALLET].m_restart = true;
    tmp[F_SECURITY_WALLET].m_reset = false;

    tmp[F_USER_SESSION_FEATURE_NAME];
    tmp[F_USER_SESSION_FEATURE_NAME].m_id = F_USER_SESSION_FEATURE_NAME;
    tmp[F_USER_SESSION_FEATURE_NAME].m_name = F_USER_SESSION_FEATURE_NAME;
    tmp[F_USER_SESSION_FEATURE_NAME].m_description = TRANSLATE_ME("srr_user-session");
    tmp[F_USER_SESSION_FEATURE_NAME].m_agent = CONFIG_AGENT_NAME;
    tmp[F_USER_SESSION_FEATURE_NAME].m_restart = true;
    tmp[F_USER_SESSION_FEATURE_NAME].m_reset = false;

    tmp[F_VIRTUAL_ASSETS];
    tmp[F_VIRTUAL_ASSETS].m_id = F_VIRTUAL_ASSETS;
    tmp[F_VIRTUAL_ASSETS].m_name = F_VIRTUAL_ASSETS;
    tmp[F_VIRTUAL_ASSETS].m_description = TRANSLATE_ME("srr_virtual-assets");
    tmp[F_VIRTUAL_ASSETS].m_agent = EMC4J_AGENT_NAME;
    tmp[F_VIRTUAL_ASSETS].m_restart = true;
    tmp[F_VIRTUAL_ASSETS].m_reset = true;

    tmp[F_USER_SESSION_MANAGEMENT];
    tmp[F_USER_SESSION_MANAGEMENT].m_id = F_USER_SESSION_MANAGEMENT;
    tmp[F_USER_SESSION_MANAGEMENT].m_name = F_USER_SESSION_MANAGEMENT;
    tmp[F_USER_SESSION_MANAGEMENT].m_description = TRANSLATE_ME("srr_user-session-management");
    tmp[F_USER_SESSION_MANAGEMENT].m_agent = USM_AGENT_NAME;
    tmp[F_USER_SESSION_MANAGEMENT].m_restart = true;
    tmp[F_USER_SESSION_MANAGEMENT].m_reset = true;

    return tmp;
};

static const std::map<std::string, SrrFeatureStruct> SrrFeatureMap = initSrrFeatures();

auto initSrrGroups = [&]() {

    std::map<std::string, SrrGroupStruct> tmp;

    // create groups
    tmp[G_ASSETS];
    tmp[G_DISCOVERY];
    tmp[G_MASS_MANAGEMENT];
    tmp[G_MONITORING_FEATURE_NAME];
    tmp[G_NETWORK];
    tmp[G_NOTIFICATION_FEATURE_NAME];
    tmp[G_USER_SESSION_FEATURE_NAME];
    tmp[G_USER_SESSION_MANAGEMENT];

    // add features to asset group
    tmp[G_ASSETS].m_id = G_ASSETS,
    tmp[G_ASSETS].m_name = G_ASSETS,
    tmp[G_ASSETS].m_description = TRANSLATE_ME("srr_group-assets");

    tmp[G_ASSETS].m_fp.push_back(SrrFeaturePriorityStruct(F_SECURITY_WALLET, 1));
    tmp[G_ASSETS].m_fp.push_back(SrrFeaturePriorityStruct(F_ASSET_AGENT, 2));
    tmp[G_ASSETS].m_fp.push_back(SrrFeaturePriorityStruct(F_VIRTUAL_ASSETS, 3));
    tmp[G_ASSETS].m_fp.push_back(SrrFeaturePriorityStruct(F_ALERT_AGENT, 4));
    tmp[G_ASSETS].m_fp.push_back(SrrFeaturePriorityStruct(F_AUTOMATION_SETTINGS, 5));
    tmp[G_ASSETS].m_fp.push_back(SrrFeaturePriorityStruct(F_AUTOMATIONS, 6));

    // add features to discovery group
    tmp[G_DISCOVERY].m_id = G_DISCOVERY,
    tmp[G_DISCOVERY].m_name = G_DISCOVERY,
    tmp[G_DISCOVERY].m_description = TRANSLATE_ME("srr_group-discovery");

    tmp[G_DISCOVERY].m_fp.push_back(SrrFeaturePriorityStruct(F_DISCOVERY, 1));

    // add features to mass management group
    tmp[G_MASS_MANAGEMENT].m_id = G_MASS_MANAGEMENT;
    tmp[G_MASS_MANAGEMENT].m_name = G_MASS_MANAGEMENT;
    tmp[G_MASS_MANAGEMENT].m_description = TRANSLATE_ME("srr_group-mass-management");

    tmp[G_MASS_MANAGEMENT].m_fp.push_back(SrrFeaturePriorityStruct(F_MASS_MANAGEMENT, 1));

    // add features to monitoring feature group
    tmp[G_MONITORING_FEATURE_NAME].m_id = G_MONITORING_FEATURE_NAME;
    tmp[G_MONITORING_FEATURE_NAME].m_name = G_MONITORING_FEATURE_NAME;
    tmp[G_MONITORING_FEATURE_NAME].m_description = TRANSLATE_ME("srr_group-monitoring-feature-name");

    tmp[G_MONITORING_FEATURE_NAME].m_fp.push_back(SrrFeaturePriorityStruct(F_MONITORING_FEATURE_NAME, 1));

    // add features to network group
    tmp[G_NETWORK].m_id = G_NETWORK;
    tmp[G_NETWORK].m_name = G_NETWORK;
    tmp[G_NETWORK].m_description = TRANSLATE_ME("srr_group-network");

    tmp[G_NETWORK].m_fp.push_back(SrrFeaturePriorityStruct(F_NETWORK, 1));

    // add features to notification feature group
    tmp[G_NOTIFICATION_FEATURE_NAME].m_id = G_NOTIFICATION_FEATURE_NAME;
    tmp[G_NOTIFICATION_FEATURE_NAME].m_name = G_NOTIFICATION_FEATURE_NAME;
    tmp[G_NOTIFICATION_FEATURE_NAME].m_description = TRANSLATE_ME("srr_group-notification-feature-name");

    tmp[G_NOTIFICATION_FEATURE_NAME].m_fp.push_back(SrrFeaturePriorityStruct(F_NOTIFICATION_FEATURE_NAME, 1));

    // add features to user session group
    tmp[G_USER_SESSION_FEATURE_NAME].m_id = G_USER_SESSION_FEATURE_NAME;
    tmp[G_USER_SESSION_FEATURE_NAME].m_name = G_USER_SESSION_FEATURE_NAME;
    tmp[G_USER_SESSION_FEATURE_NAME].m_description = TRANSLATE_ME("srr_group-user-session-feature-name");

    tmp[G_USER_SESSION_FEATURE_NAME].m_fp.push_back(SrrFeaturePriorityStruct(F_USER_SESSION_FEATURE_NAME, 1));

    // add features to usm group
    tmp[G_USER_SESSION_MANAGEMENT].m_id = G_USER_SESSION_MANAGEMENT,
    tmp[G_USER_SESSION_MANAGEMENT].m_name = G_USER_SESSION_MANAGEMENT,
    tmp[G_USER_SESSION_MANAGEMENT].m_description = TRANSLATE_ME("srr_group-user-session-management");

    tmp[G_USER_SESSION_MANAGEMENT].m_fp.push_back(SrrFeaturePriorityStruct(F_USER_SESSION_MANAGEMENT, 1));

    return tmp;
};

static const std::map<std::string, SrrGroupStruct> SrrGroupMap = initSrrGroups();

static std::map<const std::string, const std::string> agentToQueue = {
    { ALERT_AGENT_NAME      , ALERT_AGENT_MSG_QUEUE_NAME },
    { ASSET_AGENT_NAME      , ASSET_AGENT_MSG_QUEUE_NAME },
    { CONFIG_AGENT_NAME     , CONFIG_MSG_QUEUE_NAME },
    { EMC4J_AGENT_NAME      , EMC4J_MSG_QUEUE_NAME },
    { SECU_WALLET_AGENT_NAME, SECU_WALLET_MSG_QUEUE_NAME },
    { USM_AGENT_NAME,         USM_AGENT_MSG_QUEUE_NAME }
};

}
