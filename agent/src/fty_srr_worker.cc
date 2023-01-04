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

#include "fty_srr_worker.h"
#include "dto/request.h"
#include "dto/response.h"
#include "fty-srr.h"
#include "fty_srr_exception.h"
#include "fty_srr_groups.h"
#include "helpers/data_integrity.h"
#include "helpers/passPhrase.h"
#include "helpers/utils.h"

#include <fty_common.h>
#include <fty_common_mlm.h>
#include <fty_common_mlm_pool.h>
#include <fty-lib-certificate.h>
#include <pack/serialization.h>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <thread>

#define SRR_RESTART_DELAY_SEC     5
#define FEATURE_RESTORE_DELAY_SEC 6

using namespace dto::srr;

namespace srr {
/**
 * Constructor
 * @param msgBus
 * @param parameters
 */
SrrWorker::SrrWorker(messagebus::MessageBus& msgBus, const std::map<std::string, std::string>& parameters,
    const std::set<std::string>& supportedVersions)
    : m_msgBus(msgBus)
    , m_parameters(parameters)
    , m_supportedVersions(supportedVersions)
{
    init();
}

/**
 * Init srr worker
 */
void SrrWorker::init()
{
    try {
        m_srrVersion    = m_parameters.at(SRR_VERSION_KEY);
        m_sendTimeout_s = std::stoi(m_parameters.at(REQUEST_TIMEOUT_KEY)) / 1000; //sec.
        logDebug("srrVersion: {}, sendTimeout: {} sec", m_srrVersion, m_sendTimeout_s);
    }
    catch (const std::exception& ex) {
        throw SrrException(ex.what());
    }
}

dto::srr::SaveResponse SrrWorker::saveFeature(
    const dto::srr::FeatureName& featureName, const std::string& passphrase, const std::string& sessionToken)
{
    logInfo("saveFeature {}", featureName);

    std::string agentNameDest;
    std::string queueNameDest;

    try {
        agentNameDest = g_srrFeatureMap.at(featureName).m_agent;
        queueNameDest = g_agentToQueue.at(agentNameDest);
    }
    catch (std::exception& ex) {
        logError("Feature {} not found", featureName);
        throw SrrSaveFailed("Feature " + featureName + " not found");
    }

    logDebug("Request save of feature {} to agent {}/{}", featureName, agentNameDest, queueNameDest);

    // Send message to agent
    messagebus::Message message;
    try {
        dto::srr::Query saveQuery;
        saveQuery = dto::srr::createSaveQuery({featureName}, passphrase, sessionToken);

        dto::UserData data;
        data << saveQuery;

        auto agentName = m_parameters.at(AGENT_NAME_KEY);
        message = sendRequest(m_msgBus, data, "save", agentName, queueNameDest, agentNameDest, m_sendTimeout_s);
    }
    catch (SrrException& ex) {
        logError("Request save of feature {} by agent {}/{} has failed (e: {})", featureName, agentNameDest, queueNameDest, ex.what());
        throw(SrrSaveFailed("Request to agent " + agentNameDest + ":" + queueNameDest + " failed: " + ex.what()));
    }

    logDebug("Request save of {} done by agent {}", featureName, agentNameDest);

/**
    logDebug("response frames: {}", message.userData().size());
    if (ftylog_getInstance()->isLogDebug()) { //dump response frames
        int i = 0;
        for (auto& s : message.userData())
            { logDebug("frame #{}: {}", i, s); i++; }
    }
**/

    dto::srr::Response featureResponse;
    message.userData() >> featureResponse;
    dto::srr::SaveResponse response;
    response = featureResponse.save();

    // check all features in the map of the response. If one failed, the save operation fails
    for (const auto& f : response.map_features_data()) {
        if (f.second.status().status() != Status::SUCCESS) {
            logError("Save of feature {} failed (status: {})", featureName, statusToString(f.second.status().status()));
            throw(SrrSaveFailed("Save failed for feature " + featureName));
        }
    }

    logInfo("saveFeature {} completed", featureName);
    return response;
}

dto::srr::RestoreResponse SrrWorker::restoreFeature(
    const dto::srr::FeatureName& featureName, const dto::srr::RestoreQuery& query)
{
    logInfo("restoreFeature {}", featureName);

    const std::string agentNameDest = g_srrFeatureMap.at(featureName).m_agent;
    const std::string queueNameDest = g_agentToQueue.at(agentNameDest);

    log_debug("Request restore of feature %s to agent %s ", featureName.c_str(), agentNameDest.c_str());

    // IPMVAL-4505: restore exception on network hostname
    // We must stop certificate manager to avoid cert rebuild and nginx restart
    if (featureName == F_NETWORK_HOST_NAME) {
        certmanager(false); // stop
    }

    // Send message
    messagebus::Message message;
    try {
        Query restoreQuery;
        *(restoreQuery.mutable_restore()) = query;

        dto::UserData data;
        data << restoreQuery;

        auto agentName = m_parameters.at(AGENT_NAME_KEY);
        message = sendRequest(m_msgBus, data, "restore", agentName, queueNameDest, agentNameDest, m_sendTimeout_s);
    }
    catch (SrrException& ex) {
        throw(SrrRestoreFailed("Request to agent " + agentNameDest + ":" + queueNameDest + " failed: " + ex.what()));
    }

    Response featureResponse;
    message.userData() >> featureResponse;
    dto::srr::RestoreResponse response;
    response = featureResponse.restore();

    // check all features in the map of the response. If one failed, the restore operation fails
    bool restoreOk = true;
    for (const auto& f : response.map_features_status()) {
        if (f.second.status() != Status::SUCCESS) {
            restoreOk = false;
        }
    }

    if (!restoreOk) {
        // if restore network hostname failed, restart certificate manager
        if (featureName == F_NETWORK_HOST_NAME) {
            certmanager(true); // start
        }

        throw SrrRestoreFailed("Restore procedure failed for feature " + featureName);
    }

    logInfo("restoreFeature {} completed", featureName);
    return response;
}

dto::srr::ResetResponse SrrWorker::resetFeature(const dto::srr::FeatureName& featureName)
{
    logInfo("resetFeature {}", featureName);

    const std::string agentNameDest = g_srrFeatureMap.at(featureName).m_agent;
    const std::string queueNameDest = g_agentToQueue.at(agentNameDest);

    log_debug("Request reset of feature %s to agent %s ", featureName.c_str(), agentNameDest.c_str());

    messagebus::Message message;
    try {
        Query       query;
        ResetQuery& resetQuery          = *(query.mutable_reset());
        *(resetQuery.mutable_version()) = m_srrVersion;
        resetQuery.add_features(featureName);

        dto::UserData data;
        data << query;

        auto agentName = m_parameters.at(AGENT_NAME_KEY);
        message = sendRequest(m_msgBus, data, "reset", agentName, queueNameDest, agentNameDest, m_sendTimeout_s);
    }
    catch (SrrException& ex) {
        throw(SrrResetFailed("Request to agent " + agentNameDest + ":" + queueNameDest + " failed: " + ex.what()));
    }

    Response featureResponse;
    message.userData() >> featureResponse;
    dto::srr::ResetResponse response;
    response = featureResponse.reset();

    bool resetOk = true;
    for (const auto& f : response.map_features_status()) {
        if (f.second.status() != Status::SUCCESS) {
            resetOk = false;
        }
    }
    if (!resetOk) {
        throw SrrResetFailed("Reset procedure failed for feature " + featureName);
    }

    logInfo("resetFeature {} completed", featureName);
    return response;
}

bool SrrWorker::rollback(const dto::srr::SaveResponse& rollbackSaveResponse, const std::string& passphrase, const std::string& sessionToken)
{
    logInfo("Features roll back...");

    bool restart = false;

    std::map<std::string, FeatureAndStatus> rollbackMap(
        rollbackSaveResponse.map_features_data().begin(), rollbackSaveResponse.map_features_data().end());

    std::vector<FeatureName> featuresToRestore;

    for (const auto& entry : rollbackMap) {
        featuresToRestore.push_back(entry.first);
    }

    // in version 1.0 sorting has no practical effect, as there is no concept of groups
    // in version 2.0, a rollbackSaveResponse will contain only features from the same group -> ordering is meaningful
    std::sort(featuresToRestore.begin(), featuresToRestore.end(), [&](FeatureName l, FeatureName r) {
        return getPriority(l) < getPriority(r);
    });

    // reset features in reverse order
    for (auto revIt = featuresToRestore.rbegin(); revIt != featuresToRestore.rend(); revIt++) {
        if (g_srrFeatureMap.at(*revIt).m_reset) {
            try {
                resetFeature(*revIt);
            }
            catch (SrrResetFailed& ex) {
                log_warning("resetFeature: %s", ex.what());
            }
        }
    }

    for (const auto& featureName : featuresToRestore)
    {
        const dto::srr::Feature& featureData = rollbackMap.at(featureName).feature();

        // Build restore query
        RestoreQuery restoreQuery;
        restoreQuery.set_version(m_srrVersion);
        restoreQuery.set_checksum(fty::encrypt(passphrase, passphrase));
        restoreQuery.set_passpharse(passphrase);
        restoreQuery.set_session_token(sessionToken);

        restoreQuery.mutable_map_features_data()->insert({featureName, featureData});

        // restore backup data
        const std::string agentNameDest = g_srrFeatureMap.at(featureName).m_agent;
        log_debug("Rollback configuration of %s by agent %s ", featureName.c_str(), agentNameDest.c_str());

        try {
            restoreFeature(featureName, restoreQuery);
        }
        catch (SrrRestoreFailed& ex) {
            log_error("Feature %s is unrecoverable. May be in undefined state", featureName.c_str());
        }

        log_debug("%s rolled back by: %s ", featureName.c_str(), agentNameDest.c_str());
        restart = restart | g_srrFeatureMap.at(featureName).m_restart;

        // wait to sync feature restore
        std::this_thread::sleep_for(std::chrono::seconds(FEATURE_RESTORE_DELAY_SEC));
    }

    logInfo("Roll back completed");
    return restart;
}

// UI interface
dto::UserData SrrWorker::getGroupList()
{
    logInfo("SRR group list request");

    SrrListResponse srrListResp;

    srrListResp.m_version                = m_srrVersion;
    srrListResp.m_passphrase_description = srr::getPassphraseFormatMessage();
    srrListResp.m_passphrase_validation  = srr::getPassphraseFormat();

    for (const auto& mapEntry : g_srrGroupMap) {
        const std::string&    groupId  = mapEntry.first;
        const SrrGroupStruct& srrGroup = mapEntry.second;

        GroupInfo groupInfo;
        groupInfo.m_group_id    = groupId;
        groupInfo.m_group_name  = srrGroup.m_name;
        groupInfo.m_description = srrGroup.m_description;

        for (const auto& featureAndPriority : srrGroup.m_fp) {
            const std::string& featureId = featureAndPriority.m_feature;

            FeatureInfo featureInfo;
            featureInfo.m_name        = featureId;
            featureInfo.m_description = g_srrFeatureMap.at(featureId).m_description;

            groupInfo.m_features.push_back(featureInfo);
        }

        srrListResp.m_groups.push_back(groupInfo);
    }

    cxxtools::SerializationInfo si;
    si <<= srrListResp;

    dto::UserData response;
    response.push_back(dto::srr::serializeJson(si));

    logInfo("SRR group list request completed");
    return response;
}

dto::UserData SrrWorker::requestSave(const std::string& json)
{
    logInfo("SRR save request");

    SrrSaveResponse srrSaveResp;
    srrSaveResp.m_version = m_srrVersion;
    srrSaveResp.m_status  = statusToString(Status::FAILED);

    bool allGroupsSaved = true;

    try {
        cxxtools::SerializationInfo requestSi = dto::srr::deserializeJson(json);
        SrrSaveRequest              srrSaveReq;

        requestSi >>= srrSaveReq;

        // check that passphrase is compliant with requested format
        if (srr::checkPassphraseFormat(srrSaveReq.m_passphrase)) {
            // evalutate checksum
            srrSaveResp.m_checksum = fty::encrypt(srrSaveReq.m_passphrase, srrSaveReq.m_passphrase);

            log_debug("Save IPM2 configuration processing");

            std::map<std::string, Group> savedGroups;

            // save all the features for each required group
            for (const auto& groupId : srrSaveReq.m_group_list) {
                log_debug("Saving features from group %s ", groupId.c_str());

                srr::SrrGroupStruct group;
                try {
                    group = g_srrGroupMap.at(groupId);
                }
                catch (std::out_of_range& /* ex */) {
                    allGroupsSaved = false;
                    log_error("Group %s not found", groupId.c_str());
                    // do not save features from the current group, as it would be incomplete
                    continue;
                }

                try {
                    for (const auto& entry : group.m_fp) {
                        const auto& featureName = entry.m_feature;
                        log_debug("Saving feature %s of group %s ", featureName.c_str(), groupId.c_str());

                        SaveResponse saveResp =
                            saveFeature(featureName, srrSaveReq.m_passphrase, srrSaveReq.m_sessionToken);
                        // convert ProtoBuf save response to UI DTO
                        const auto& mapFeaturesData = saveResp.map_features_data();

                        for (const auto& fs : mapFeaturesData) {
                            SrrFeature f;
                            f.m_feature_name       = fs.first;
                            f.m_feature_and_status = fs.second;

                            // save each feature into its group
                            savedGroups[groupId].m_features.push_back(f);
                        }
                    }
                }
                catch (const std::exception& e) {
                    log_error("Error saving group %s (e: %s). It will be excluded from the payload",
                        groupId.c_str(), e.what());

                    // delete the current group, as it would be incomplete
                    allGroupsSaved = false;
                    savedGroups.erase(groupId);
                    continue;
                }
            }

            // update group info and evaluate data integrity
            for (auto groupElement : savedGroups) {
                const auto& groupId = groupElement.first;
                auto&       group   = groupElement.second;

                group.m_group_id   = groupId;
                group.m_group_name = groupId;

                // evaluate data integrity
                evalDataIntegrity(group);

                srrSaveResp.m_data.push_back(group);
            }

            if (allGroupsSaved) {
                srrSaveResp.m_status = statusToString(Status::SUCCESS);
            } else {
                srrSaveResp.m_status = statusToString(Status::PARTIAL_SUCCESS);
            }
        } else {
            srrSaveResp.m_error =
                TRANSLATE_ME("Passphrase must have %s characters", (fty::getPassphraseFormat()).c_str());
            log_error(srrSaveResp.m_error.c_str());
        }
    } catch (const std::exception& e) {
        srrSaveResp.m_error = TRANSLATE_ME("Exception on save Ipm2 configuration: (%s)", e.what());
        log_error(srrSaveResp.m_error.c_str());
    }

    cxxtools::SerializationInfo responseSi;
    responseSi <<= srrSaveResp;
    std::string jsonResp = serializeJson(responseSi);

    dto::UserData response;
    response.push_back(srrSaveResp.m_status);
    response.push_back(jsonResp);

    logInfo("SRR save request completed (status: {})", srrSaveResp.m_status);
    return response;
}

static std::string zmsg_popstring(zmsg_t* resp)
{
    char* popstr = zmsg_popstr(resp);
    // copies the null-terminated character sequence (C-string) pointed by popstr
    std::string string_rv = popstr ? popstr : "";
    zstr_free(&popstr);
    return string_rv;
}

static bool getLicenseCapabilities()
{
    MlmClientPool::Ptr client = mlm_pool.get();
    if (!client.getPointer()) {
        log_error("fty-srr: mlm_pool.get () failed.");
        return false;
    }

    zmsg_t* req = zmsg_new();
    zmsg_addstr(req, "CAPABILITIES");
    zmsg_addstr(req, "configurability");

    zmsg_t* resp = client->requestreply("etn-licensing", "licensing", 5, &req);
    zmsg_destroy(&req);
    if (!resp) {
        log_error("fty-srr: client->requestreply etn-licensing (timeout = '5') returned NULL");
        return false;
    }

    std::string command = zmsg_popstring(resp);
    if (command != "CAPABILITIES") {
        zmsg_destroy(&resp);
        log_debug("fty-srr: Unknown command received from etn-licensing (%s)", command.c_str());
        return false;
    }

    std::string code = zmsg_popstring(resp);
    if (code != "OK") {
        zmsg_destroy(&resp);
        log_debug("fty-srr: recv '%s' from etn-licensing", code.c_str());
        return false;
    }

    std::string configurabilityStr = zmsg_popstring(resp);
    zmsg_destroy(&resp);

    bool configurability;

    std::istringstream(configurabilityStr) >> configurability;

    return configurability;
}

dto::UserData SrrWorker::requestRestore(const std::string& json, bool force)
{
    logInfo("SRR restore request");

    const bool IMPVAL_3680_HOTFIX{true}; // 2.4.0, see also IPMVAL-3668
    bool restart = false;

    SrrRestoreResponse srrRestoreResp;

    srrRestoreResp.m_status = statusToString(Status::FAILED);

    try {
        if (!getLicenseCapabilities()) {
            log_error("Restore not allowed by licensing limitations");
            throw std::runtime_error("Restore not allowed by licensing limitations");
        }

        cxxtools::SerializationInfo requestSi = dto::srr::deserializeJson(json);
        SrrRestoreRequest           srrRestoreReq;

        requestSi >>= srrRestoreReq;

        std::string passphrase = fty::decrypt(srrRestoreReq.m_checksum, srrRestoreReq.m_passphrase);

        if (passphrase.compare(srrRestoreReq.m_passphrase) != 0) {
            throw std::runtime_error("Invalid passphrase");
        }

        log_info("Restore request (version: %s, force: %s)", srrRestoreReq.m_version.c_str(), (force ? "true" : "false"));

        if (IS_VERSION_1(srrRestoreReq.m_version))
        {
            const auto& features = srrRestoreReq.m_data_ptr->getSrrFeatures();

            bool allFeaturesRestored = true;

            std::string featureName;

            for (const auto& feature : features) {
                featureName            = feature.m_feature_name;
                const auto& dtoFeature = feature.m_feature_and_status.feature();
                // prepare restore query
                RestoreQuery query;
                query.set_passpharse(srrRestoreReq.m_passphrase);
                query.set_session_token(srrRestoreReq.m_sessionToken);
                query.mutable_map_features_data()->insert({featureName, dtoFeature});

                RestoreStatus restoreStatus;
                restoreStatus.m_name = featureName;

                if (IMPVAL_3680_HOTFIX) {
                    // WA missing m_description, m_name is used as a translation key
                    if (restoreStatus.m_name.find(SRR_PREFIX_TRANSLATE_KEY) != 0)
                        restoreStatus.m_name.insert(0, SRR_PREFIX_TRANSLATE_KEY);
                }

                // save feature to perform a rollback in case of error
                SaveResponse rollbackSaveResponse;
                log_debug("Saving feature %s current status", feature.m_feature_name.c_str());
                try {
                    rollbackSaveResponse +=
                        saveFeature(feature.m_feature_name, srrRestoreReq.m_passphrase, srrRestoreReq.m_sessionToken);
                }
                catch (std::exception& ex) {
                    allFeaturesRestored = false;

                    restoreStatus.m_status = statusToString(Status::FAILED);
                    restoreStatus.m_error =
                        TRANSLATE_ME("Could not backup feature %s. Restore will be skipped", featureName.c_str());

                    log_error(restoreStatus.m_error.c_str());

                    srrRestoreResp.m_status_list.push_back(restoreStatus);

                    continue;
                }

                // reset feature before restore (do not stop on fail -> reset is not supported by every feature yet)
                if (g_srrFeatureMap.at(featureName).m_reset) {
                    try {
                        resetFeature(featureName);
                    }
                    catch (SrrResetFailed& ex) {
                        log_warning(ex.what());
                    }
                }

                // perform restore
                try {
                    RestoreResponse resp   = restoreFeature(featureName, query);
                    restoreStatus.m_status = statusToString(resp.status().status());
                    restoreStatus.m_error  = TRANSLATE_ME(resp.status().error().c_str());
                }
                catch (SrrRestoreFailed& ex) {
                    allFeaturesRestored = false;

                    restoreStatus.m_status = statusToString(Status::FAILED);
                    restoreStatus.m_error  = TRANSLATE_ME(ex.what());

                    log_error(restoreStatus.m_error.c_str());

                    srrRestoreResp.m_status_list.push_back(restoreStatus);

                    // start rollback
                    restart = restart | rollback(rollbackSaveResponse, srrRestoreReq.m_passphrase, srrRestoreReq.m_sessionToken);

                    continue;
                }

                srrRestoreResp.m_status_list.push_back(restoreStatus);
                // wait to sync feature restore
                std::this_thread::sleep_for(std::chrono::seconds(FEATURE_RESTORE_DELAY_SEC));
            }

            if (allFeaturesRestored) {
                srrRestoreResp.m_status = statusToString(Status::SUCCESS);
            } else {
                srrRestoreResp.m_status = statusToString(Status::PARTIAL_SUCCESS);
            }
        }
        else if (IS_VERSION_2(srrRestoreReq.m_version))
        {
            std::shared_ptr<SrrRestoreRequestDataV2> dataPtr =
                std::dynamic_pointer_cast<SrrRestoreRequestDataV2>(srrRestoreReq.m_data_ptr);
            auto& groups = dataPtr->m_data;

            // sort groups by restore order
            std::sort(groups.begin(), groups.end(), [&](const Group& l, const Group& r) {
                unsigned priorityL = 0;
                unsigned priorityR = 0;
                try {
                    // unknown groups will be placed at the end and skipped
                    priorityL = g_srrGroupMap.at(l.m_group_id).m_restoreOrder;
                    priorityR = g_srrGroupMap.at(r.m_group_id).m_restoreOrder;
                }
                catch (const std::exception& e) {
                    return false;
                }
                return priorityL < priorityR;
            });

            // sort features in each group by priority
            for (auto& group : groups) {
                std::sort(group.m_features.begin(), group.m_features.end(), [&](SrrFeature l, SrrFeature r) {
                    return getPriority(l.m_feature_name) < getPriority(r.m_feature_name);
                });
            }

            // data integrity check
            std::list<std::string> groupsIntegrityCheckFailed; // groups where integrity check failed
            if (force) {
                log_warning("Restoring with force option: data integrity check will be skipped");
            } else {
                // features in each group must be sorted by priority to evaluate correctly the data integrity
                for (auto& group : groups) {
                    // check data integrity
                    if (!checkDataIntegrity(group)) {
                        log_error("Integrity check failed for group %s", group.m_group_id.c_str());
                        groupsIntegrityCheckFailed.push_back(group.m_group_id);
                    }
                }
            }

            // if force option is not set, verify that all group data is valid
            if (!force && !groupsIntegrityCheckFailed.empty()) {
                std::string groupList, sep;
                for (auto& it : groupsIntegrityCheckFailed)
                    { groupList += sep + it; sep = ", "; }
                throw srr::SrrIntegrityCheckFailed("Data integrity check failed for groups: " + groupList);
            }

            // start restore procedure
            RestoreResponse response;
            bool            allGroupsRestored = true;

            for (const auto& group : groups) {
                const auto& groupId = group.m_group_id;

                if (g_srrGroupMap.find(group.m_group_id) == g_srrGroupMap.end()) {
                    RestoreStatus restoreStatus;
                    restoreStatus.m_name   = groupId;
                    restoreStatus.m_status = statusToString(Status::FAILED);
                    restoreStatus.m_error  = TRANSLATE_ME("Group %s is not supported. Will not be restored", groupId.c_str());

                    srrRestoreResp.m_status_list.push_back(restoreStatus);

                    log_error(restoreStatus.m_error.c_str());

                    allGroupsRestored = false;
                    continue;
                }

                std::map<std::string, dto::srr::FeatureAndStatus> ftMap;
                for (const auto& feature : group.m_features) {
                    ftMap[feature.m_feature_name] = feature.m_feature_and_status;
                }

                // create all restore queries related to the current group
                // it helps to detect at an early stage if there are features missing in the restore payload
                std::map<FeatureName, RestoreQuery> restoreQueriesMap;

                try {
                    // loop through all required features to create the restore queries
                    for (const auto& feature : g_srrGroupMap.at(groupId).m_fp) {
                        const auto& featureName = feature.m_feature;
                        try {
                            const auto& dtoFeature = ftMap.at(featureName).feature();

                            // prepare restore queries
                            RestoreQuery& request = restoreQueriesMap[featureName];
                            request.set_passpharse(srrRestoreReq.m_passphrase);
                            request.set_session_token(srrRestoreReq.m_sessionToken);
                            request.mutable_map_features_data()->insert({featureName, dtoFeature});
                        }
                        catch (const std::out_of_range& e) {
                            // missing feature, check if it is required in restore payload version
                            const auto requiredIn = g_srrFeatureMap.at(featureName).m_requiredIn;
                            auto found = std::find(requiredIn.begin(), requiredIn.end(), srrRestoreReq.m_version);
                            if (found != requiredIn.end()) {
                                log_error("Feature %s is required in version %s", featureName.c_str(), srrRestoreReq.m_version.c_str());
                                throw std::runtime_error(
                                    "Feature " + featureName + " is required in version " + srrRestoreReq.m_version);
                            }
                        }
                        catch (const std::exception& e) {
                            throw std::runtime_error(e.what());
                        }
                    }
                }
                catch (std::out_of_range& ex) {
                    // if one feature is missing, set the error for the whole group and skip the group
                    RestoreStatus restoreStatus;
                    restoreStatus.m_name   = groupId;
                    restoreStatus.m_status = statusToString(Status::FAILED);
                    restoreStatus.m_error  = TRANSLATE_ME("Group %s cannot be restored. Missing features", groupId.c_str());

                    srrRestoreResp.m_status_list.push_back(restoreStatus);

                    log_error(restoreStatus.m_error.c_str());

                    allGroupsRestored = false;
                    continue;
                }

                // get list of features in the group (based on current version)
                const auto featureList = g_srrGroupMap.at(group.m_group_id).m_fp;

                // save group status to perform a rollback in case of error
                SaveResponse rollbackSaveResponse;
                try {
                    for (const auto& feature : featureList) {
                        log_debug("Saving feature %s current status", feature.m_feature.c_str());
                        rollbackSaveResponse +=
                            saveFeature(feature.m_feature, srrRestoreReq.m_passphrase, srrRestoreReq.m_sessionToken);
                    }
                }
                catch (std::exception& ex) {
                    log_error("Could not backup feature %s", groupId.c_str());
                }

                // reset features in reverse order before restore
                // WARNING: currently reset is not implemented by all features, hence it will not be mandatory
                for (auto revIt = featureList.rbegin(); revIt != featureList.rend(); revIt++) {
                    if (g_srrFeatureMap.at(revIt->m_feature).m_reset) {
                        try {
                            resetFeature(revIt->m_feature);
                        } catch (SrrResetFailed& ex) {
                            log_warning(ex.what());
                        }
                    }
                }

                bool restoreFailed = false;

                RestoreStatus restoreStatus;
                restoreStatus.m_name   = groupId;
                restoreStatus.m_status = statusToString(Status::SUCCESS);

                if (IMPVAL_3680_HOTFIX) {
                    // WA missing m_description, m_name is used as a translation key
                    if (restoreStatus.m_name.find(SRR_PREFIX_TRANSLATE_KEY) != 0)
                        restoreStatus.m_name.insert(0, SRR_PREFIX_TRANSLATE_KEY);
                }

                // restore features in order
                for (const auto& feature : group.m_features) {
                    const auto& featureName = feature.m_feature_name;

                    try {
                        // Restore feature
                        response += restoreFeature(featureName, restoreQueriesMap[featureName]);

                        // update restart flag
                        restart = restart | g_srrFeatureMap.at(featureName).m_restart;
                    }
                    catch (const std::exception& ex) {
                        // restore failed -> rolling back the whole group
                        restoreFailed = true;

                        restoreStatus.m_status = statusToString(Status::FAILED);
                        restoreStatus.m_error =
                            TRANSLATE_ME("Restore failed for feature %s: ", featureName.c_str(), ex.what());

                        allGroupsRestored = false;
                        log_error(restoreStatus.m_error.c_str());

                        // stop group restore
                        break;
                    }

                    // wait to sync feature restore
                    std::this_thread::sleep_for(std::chrono::seconds(FEATURE_RESTORE_DELAY_SEC));
                }

                // if restore failed -> rollback
                if (restoreFailed) {
                    restart = restart | rollback(rollbackSaveResponse, srrRestoreReq.m_passphrase, srrRestoreReq.m_sessionToken);
                }

                // push group status into restore response
                srrRestoreResp.m_status_list.push_back(restoreStatus);
            }

            if (allGroupsRestored) {
                srrRestoreResp.m_status = statusToString(Status::SUCCESS);
            } else {
                srrRestoreResp.m_status = statusToString(Status::PARTIAL_SUCCESS);
            }
        }
        else
        {
            throw SrrInvalidVersion();
        }
    }
    catch (const SrrIntegrityCheckFailed& e) {
        srrRestoreResp.m_status = statusToString(Status::UNKNOWN);
        srrRestoreResp.m_error  = TRANSLATE_ME(e.what());

        log_error(srrRestoreResp.m_error.c_str());
    }
    catch (const std::exception& e) {
        srrRestoreResp.m_status = statusToString(Status::FAILED);
        srrRestoreResp.m_error  = TRANSLATE_ME(e.what());

        log_error(srrRestoreResp.m_error.c_str());
    }

    cxxtools::SerializationInfo responseSi;
    responseSi <<= srrRestoreResp;

    dto::UserData response;
    std::string   jsonResp = serializeJson(responseSi);
    response.push_back(srrRestoreResp.m_status);
    response.push_back(jsonResp);

    logInfo("SRR restore request status: {}", srrRestoreResp.m_status);

    if (restart) {
        if (m_parameters.at(SRR_ENABLE_REBOOT_KEY) == "true") {
            logInfo("Reboot required");
            std::thread restartThread(restartBiosService, SRR_RESTART_DELAY_SEC);
            restartThread.detach();
        } else {
            log_warning("Reboot is disabled in current configuration");
        }
    }

    logInfo("SRR restore request done");
    return response;
}

dto::UserData SrrWorker::requestReset(const std::string& /* json */)
{
    logInfo("SRR reset request");

    throw SrrException("Not implemented yet!");
}

} // namespace srr
