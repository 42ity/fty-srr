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

/*
@header
    fty_srr_worker - Fty srr worker
@discuss
@end
 */

// clang-format off

#include "fty-srr.h"
#include "fty_srr_exception.h"
#include "fty_srr_groups.h"
#include "fty_srr_worker.h"

#include "dto/request.h"
#include "dto/response.h"

#include "helpers/data_integrity.h"
#include "helpers/utils.h"

#include <fty_common.h>
#include <fty_lib_certificate_library.h>

#include <chrono>
#include <cstdlib>
#include <numeric>
#include <vector>
#include <thread>

#define SRR_RESTART_DELAY_SEC     5
#define FEATURE_RESTORE_DELAY_SEC 6

using namespace dto::srr;

namespace srr
{
    /**
     * Constructor
     * @param msgBus
     * @param parameters
     */
    SrrWorker::SrrWorker(messagebus::MessageBus& msgBus, const std::map<std::string, std::string>& parameters, const std::set<std::string> supportedVersions) :
        m_msgBus(msgBus), m_parameters(parameters), m_supportedVersions(supportedVersions)
    {
        init();
    }

    /**
     * Init srr worker
     */
    void SrrWorker::init()
    {
        try
        {
            m_srrVersion = m_parameters.at(SRR_VERSION_KEY);
            m_sendTimeout = std::stoi (m_parameters.at (REQUEST_TIMEOUT_KEY)) / 1000;
        }
        catch (const std::exception & ex)
        {
            throw SrrException(ex.what());
        }
    }

    dto::srr::SaveResponse SrrWorker::saveFeature(const dto::srr::FeatureName& featureName, const std::string& passphrase, const std::string& sessionToken)
    {
        dto::srr::SaveResponse response;

        std::string agentNameDest;
        std::string queueNameDest;

        try {
            agentNameDest = g_srrFeatureMap.at(featureName).m_agent;
            queueNameDest = g_agentToQueue.at(agentNameDest);
        }
        catch (std::exception& ex) {
            log_error("Feature %s not found", featureName.c_str());
            throw SrrSaveFailed("Feature " +  featureName + " not found");
        }

        log_debug("Request save of feature %s to agent %s", featureName.c_str(), agentNameDest.c_str());

        dto::srr::Query saveQuery = dto::srr::createSaveQuery({featureName}, passphrase, sessionToken);

        dto::UserData data;
        data << saveQuery;
        // Send message to agent
        messagebus::Message message;
        try{
            message = sendRequest(m_msgBus, data, "save", m_parameters.at (AGENT_NAME_KEY), queueNameDest, agentNameDest);
        }
        catch (SrrException& ex) {
            throw (SrrSaveFailed("Request to agent " + agentNameDest + ":" + queueNameDest + " failed: " + ex.what()));
        }
        log_debug("Save done by agent %s", agentNameDest.c_str());

        dto::srr::Response featureResponse;
        message.userData() >> featureResponse;

        // check all features in the map of the response. If one failed, the save operation fails
        for(const auto& f : featureResponse.save().map_features_data()) {
            if (f.second.status().status() != Status::SUCCESS) {
                throw (SrrSaveFailed("Save failed for feature " + featureName));
            }
        }

        response = featureResponse.save();

        return response;
    }

    dto::srr::RestoreResponse SrrWorker::restoreFeature(const dto::srr::FeatureName& featureName, const dto::srr::RestoreQuery& query)
    {
        const std::string agentNameDest = g_srrFeatureMap.at(featureName).m_agent;
        const std::string queueNameDest = g_agentToQueue.at(agentNameDest);

        Query restoreQuery;
        *(restoreQuery.mutable_restore()) = query;
        log_debug("Request restore of feature %s to agent %s ", featureName.c_str(), agentNameDest.c_str());

        // Send message
        dto::UserData data;
        data << restoreQuery;
        messagebus::Message message;
        try {
            message = sendRequest(m_msgBus, data, "restore", m_parameters.at (AGENT_NAME_KEY), queueNameDest, agentNameDest, m_sendTimeout);
        }
        catch (SrrException& ex) {
            throw (SrrRestoreFailed("Request to agent " + agentNameDest + ":" + queueNameDest + " failed: " + ex.what()));
        }

        Response response;
        message.userData() >> response;

        // restore procedure failed -> rollback
        // check all features in the map of the response. If one failed, the save operation fails
        bool restoreOk = true;
        for(const auto& f : response.restore().map_features_status()) {
            if (f.second.status() != Status::SUCCESS) {
                restoreOk = false;
            }
        }

        if(!restoreOk) {
            throw SrrRestoreFailed("Restore procedure failed for feature " + featureName);
        }

        return response.restore();
    }

    dto::srr::ResetResponse SrrWorker::resetFeature(const dto::srr::FeatureName& featureName)
    {
        const std::string agentNameDest = g_srrFeatureMap.at(featureName).m_agent;
        const std::string queueNameDest = g_agentToQueue.at(agentNameDest);

        log_debug("Request reset of feature %s to agent %s ", featureName.c_str(), agentNameDest.c_str());

        Query query;
        ResetQuery& resetQuery = *(query.mutable_reset());
        *(resetQuery.mutable_version()) = m_srrVersion;
        resetQuery.add_features(featureName);

        dto::UserData data;
        data << query;
        messagebus::Message message;
        try {
            message = sendRequest(m_msgBus, data, "reset", m_parameters.at (AGENT_NAME_KEY), queueNameDest, agentNameDest, m_sendTimeout);
        }
        catch (SrrException& ex) {
            throw (SrrResetFailed("Request to agent " + agentNameDest + ":" + queueNameDest + " failed: " + ex.what()));
        }

        Response response;
        message.userData() >> response;

        bool resetOk = true;
        for(const auto& f : response.reset().map_features_status()) {
            if (f.second.status() != Status::SUCCESS) {
                resetOk = false;
            }
        }
        if(!resetOk) {
            throw SrrResetFailed("Reset procedure failed for feature " + featureName);
        }

        return response.reset();
    }

    bool SrrWorker::rollback(const dto::srr::SaveResponse& rollbackSaveResponse, const std::string& passphrase)
    {
        bool restart = false;

        log_debug("Starting features roll back...");

        std::map<std::string, FeatureAndStatus> rollbackMap(rollbackSaveResponse.map_features_data().begin(), rollbackSaveResponse.map_features_data().end());

        std::vector<FeatureName> featuresToRestore;

        for(const auto& entry : rollbackMap) {
            featuresToRestore.push_back(entry.first);
        }

        // in version 1.0 sorting has no practical effect, as there is no concept of groups
        // in version 2.0, a rollbackSaveResponse will contain only features from the same group -> ordering is meaningful
        std::sort(featuresToRestore.begin(), featuresToRestore.end(), [&] (FeatureName l, FeatureName r) {
            return getPriority(l) < getPriority(r);
        });

        // reset features in reverse order
        for(auto revIt = featuresToRestore.rbegin(); revIt != featuresToRestore.rend(); revIt++) {
            if(g_srrFeatureMap.at(*revIt).m_reset) {
                try{
                    resetFeature(*revIt);
                }
                catch (SrrResetFailed& ex) {
                    log_warning(ex.what());
                }
            }
        }

        for(const auto& featureName : featuresToRestore) {
            const dto::srr::Feature& featureData= rollbackMap.at(featureName).feature();

            const std::string agentNameDest = g_srrFeatureMap.at(featureName).m_agent;
            const std::string queueNameDest = g_agentToQueue.at(agentNameDest);

            // Build restore query
            RestoreQuery restoreQuery;

            *(restoreQuery.mutable_version()) = m_srrVersion;
            *(restoreQuery.mutable_checksum()) = fty::encrypt(passphrase, passphrase);
            *(restoreQuery.mutable_passpharse()) = passphrase;
            restoreQuery.mutable_map_features_data()->insert({featureName, featureData});

            // restore backup data
            log_debug("Rollback configuration of %s by agent %s ", featureName.c_str(), agentNameDest.c_str());
            try{
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

        log_debug("Roll back completed");

        return restart;
    }

    // UI interface
    dto::UserData SrrWorker::getGroupList()
    {
        SrrListResponse srrListResp;

        log_debug("SRR group list request");

        srrListResp.m_version = m_srrVersion;
        srrListResp.m_passphrase_description = TRANSLATE_ME("Passphrase must have %s characters", (fty::getPassphraseFormat()).c_str());
        srrListResp.m_passphrase_validation = fty::getPassphraseFormat();

        for (const auto& mapEntry : g_srrGroupMap) {
            const std::string& groupId = mapEntry.first;
            const SrrGroupStruct& srrGroup = mapEntry.second;

            GroupInfo groupInfo;
            groupInfo.m_group_id = groupId;
            groupInfo.m_group_name = srrGroup.m_name;
            groupInfo.m_description = srrGroup.m_description;

            for (const auto& featureAndPriority : srrGroup.m_fp) {
                const std::string& featureId = featureAndPriority.m_feature;

                FeatureInfo featureInfo;

                featureInfo.m_name = featureId;
                featureInfo.m_description = g_srrFeatureMap.at(featureId).m_description;

                groupInfo.m_features.push_back(featureInfo);
            }

            srrListResp.m_groups.push_back(groupInfo);
        }

        cxxtools::SerializationInfo si;
        si <<= srrListResp;

        dto::UserData response;
        response.push_back(dto::srr::serializeJson(si));

        return response;
    }

    dto::UserData SrrWorker::requestSave(const std::string& json)
    {
        SrrSaveResponse srrSaveResp;

        log_debug("SRR save request");

        srrSaveResp.m_version = m_srrVersion;
        srrSaveResp.m_status = statusToString(Status::FAILED);

        bool allGroupsSaved = true;

        try
        {
            cxxtools::SerializationInfo requestSi = dto::srr::deserializeJson(json);
            SrrSaveRequest srrSaveReq;

            requestSi >>= srrSaveReq;

            // check that passphrase is compliant with requested format
            if (fty::checkPassphraseFormat(srrSaveReq.m_passphrase))
            {
                // evalutate checksum
                srrSaveResp.m_checksum = fty::encrypt(srrSaveReq.m_passphrase, srrSaveReq.m_passphrase);

                log_debug("Save IPM2 configuration processing");

                std::map<std::string, Group> savedGroups;

                // save all the features for each required group
                for(const auto& groupId : srrSaveReq.m_group_list) {
                    log_debug("Saving features from group %s ", groupId.c_str());
                    srr::SrrGroupStruct group;
                    try {
                        group = g_srrGroupMap.at(groupId);
                    } catch (std::out_of_range& /* ex */) {
                        allGroupsSaved = false;
                        log_error("Group %s not found", groupId.c_str());

                        // do not save features from the current group, as it would be incomplete
                        continue;
                    }

                    try {
                        for(const auto& entry : group.m_fp) {
                            const auto& featureName = entry.m_feature;

                            SaveResponse saveResp = saveFeature(featureName, srrSaveReq.m_passphrase, srrSaveReq.m_sessionToken);
                            // convert ProtoBuf save response to UI DTO
                            const auto& mapFeaturesData = saveResp.map_features_data();

                            for (const auto& fs : mapFeaturesData) {
                                SrrFeature f;
                                f.m_feature_name = fs.first;
                                f.m_feature_and_status = fs.second;

                                // save each feature into its group
                                savedGroups[groupId].m_features.push_back(f);
                            }
                        }
                    } catch (std::exception& e) {
                        allGroupsSaved = false;
                        log_error("Error while saving group %s: %s. Will not be included in the payload", groupId.c_str(), e.what());
                        // delete the current group, as it would be incomplete
                        savedGroups.erase(groupId);
                        continue;
                    }
                }

                // update group info and evaluate data integrity
                for(auto groupElement : savedGroups) {
                    const auto& groupId = groupElement.first;
                    auto& group = groupElement.second;

                    group.m_group_id = groupId;
                    group.m_group_name = groupId;

                    // evaluate data integrity
                    evalDataIntegrity(group);

                    srrSaveResp.m_data.push_back(group);
                }

                if(allGroupsSaved) {
                    srrSaveResp.m_status = statusToString(Status::SUCCESS);
                } else {
                    srrSaveResp.m_status = statusToString(Status::PARTIAL_SUCCESS);
                }
            }
            else
            {
                srrSaveResp.m_error = TRANSLATE_ME("Passphrase must have %s characters", (fty::getPassphraseFormat()).c_str());
                log_error(srrSaveResp.m_error.c_str());
            }
        }
        catch (const std::exception& e)
        {
            srrSaveResp.m_error = TRANSLATE_ME("Exception on save Ipm2 configuration: (%s)", e.what());;
            log_error(srrSaveResp.m_error.c_str());
        }

        dto::UserData response;

        cxxtools::SerializationInfo responseSi;
        responseSi <<= srrSaveResp;

        std::string jsonResp = serializeJson(responseSi);

        response.push_back(srrSaveResp.m_status);
        response.push_back(jsonResp);

        return response;
    }

    dto::UserData SrrWorker::requestRestore(const std::string& json, bool force)
    {
        bool restart = false;

        log_debug("SRR restore request");

        SrrRestoreResponse srrRestoreResp;

        srrRestoreResp.m_status = statusToString(Status::FAILED);

        try
        {
            cxxtools::SerializationInfo requestSi = dto::srr::deserializeJson(json);
            SrrRestoreRequest srrRestoreReq;

            requestSi >>= srrRestoreReq;



            std::string passphrase = fty::decrypt(srrRestoreReq.m_checksum, srrRestoreReq.m_passphrase);

            if(passphrase.compare(srrRestoreReq.m_passphrase) != 0) {
                throw std::runtime_error("Invalid passphrase");
            }

            if(srrRestoreReq.m_version == "1.0") {
                const auto& features = srrRestoreReq.m_data_ptr->getSrrFeatures();

                bool allFeaturesRestored = true;

                std::string featureName;

                for(const auto& feature : features) {
                    featureName = feature.m_feature_name;
                    const auto& dtoFeature = feature.m_feature_and_status.feature();
                    // prepare restore query
                    RestoreQuery query;
                    query.set_passpharse(srrRestoreReq.m_passphrase);
                    query.set_session_token(srrRestoreReq.m_sessionToken);
                    query.mutable_map_features_data()->insert({featureName, dtoFeature});

                    RestoreStatus restoreStatus;
                    restoreStatus.m_name = featureName;

                    // save feature to perform a rollback in case of error
                    SaveResponse rollbackSaveResponse;
                    log_debug("Saving feature %s current status", feature.m_feature_name.c_str());
                    try {
                        rollbackSaveResponse += saveFeature(feature.m_feature_name, srrRestoreReq.m_passphrase, srrRestoreReq.m_sessionToken);
                    }
                    catch (std::exception& ex) {
                        allFeaturesRestored = false;

                        restoreStatus.m_status = statusToString(Status::FAILED);
                        restoreStatus.m_error = TRANSLATE_ME("Could not backup feature %s. Restore will be skipped", featureName.c_str());

                        log_error(restoreStatus.m_error.c_str());

                        srrRestoreResp.m_status_list.push_back(restoreStatus);

                        continue;
                    }

                    // reset feature before restore (do not stop on fail -> reset is not supported by every feature yet)
                    if(g_srrFeatureMap.at(featureName).m_reset) {
                        try{
                            resetFeature(featureName);
                        }
                        catch (SrrResetFailed& ex) {
                            log_warning(ex.what());
                        }
                    }

                    // perform restore
                    try {
                        RestoreResponse resp = restoreFeature(featureName, query);
                        restoreStatus.m_status = statusToString(resp.status().status());
                        restoreStatus.m_error = TRANSLATE_ME(resp.status().error().c_str());
                    }
                    catch(SrrRestoreFailed& ex) {
                        allFeaturesRestored = false;

                        restoreStatus.m_status = statusToString(Status::FAILED);
                        restoreStatus.m_error = TRANSLATE_ME(ex.what());

                        log_error(restoreStatus.m_error.c_str());

                        srrRestoreResp.m_status_list.push_back(restoreStatus);

                        // start rollback
                        restart = restart | rollback(rollbackSaveResponse, srrRestoreReq.m_passphrase);

                        continue;
                    }

                    srrRestoreResp.m_status_list.push_back(restoreStatus);
                    // wait to sync feature restore
                    std::this_thread::sleep_for(std::chrono::seconds(FEATURE_RESTORE_DELAY_SEC));
                }

                if(allFeaturesRestored) {
                    srrRestoreResp.m_status = statusToString(Status::SUCCESS);
                } else {
                    srrRestoreResp.m_status = statusToString(Status::PARTIAL_SUCCESS);
                }
            } else if(srrRestoreReq.m_version == "2.0" || srrRestoreReq.m_version == "2.1") {
                std::list<std::string> groupsIntegrityCheckFailed;  // stores groups for which integrity check failed

                std::shared_ptr<SrrRestoreRequestDataV2> dataPtr = std::dynamic_pointer_cast<SrrRestoreRequestDataV2>(srrRestoreReq.m_data_ptr);
                auto& groups = dataPtr->m_data;

                // sort groups by restore order
                std::sort(groups.begin(), groups.end(), [&] (const Group& l, const Group& r) {
                    unsigned priorityL = 0;
                    unsigned priorityR = 0;
                    try{
                        // unknown groups will be placed at the end and skipped
                        priorityL = g_srrGroupMap.at(l.m_group_id).m_restoreOrder;
                        priorityR = g_srrGroupMap.at(r.m_group_id).m_restoreOrder;
                    } catch(const std::exception& e) {
                        return false;
                    }
                    return priorityL < priorityR;
                });

                // sort features in each group by priority
                for(auto& group : groups) {
                    std::sort(group.m_features.begin(), group.m_features.end(), [&] (SrrFeature l, SrrFeature r) {
                        return getPriority(l.m_feature_name) < getPriority(r.m_feature_name);
                    });
                }

                // data integrity check
                if(force) {
                    log_warning("Restoring with force option: data integrity check will be skipped");
                } else {
                    // features in each group must be sorted by priority to evaluate correctly the data integrity
                    for(auto& group : groups) {
                        // check data integrity
                        if(!checkDataIntegrity(group)) {
                            log_error("Integrity check failed for group %s", group.m_group_id.c_str());
                            groupsIntegrityCheckFailed.push_back(group.m_group_id);
                        }
                    }
                }

                // if force option is not set, verify that all group data is valid
                if(!force && !groupsIntegrityCheckFailed.empty()) {
                    throw srr::SrrIntegrityCheckFailed(
                        "Data integrity check failed for groups:" + std::accumulate(groupsIntegrityCheckFailed.begin(),groupsIntegrityCheckFailed.end(), std::string(" ")));
                }


                // start restore procedure
                RestoreResponse response;
                bool allGroupsRestored = true;

                for(const auto& group : groups) {
                    const auto& groupId = group.m_group_id;

                    if(g_srrGroupMap.find(group.m_group_id) == g_srrGroupMap.end()) {
                        RestoreStatus restoreStatus;
                        restoreStatus.m_name = groupId;
                        restoreStatus.m_status = statusToString(Status::FAILED);
                        restoreStatus.m_error = TRANSLATE_ME("Group %s is not supported. Will not be restored", groupId.c_str());

                        srrRestoreResp.m_status_list.push_back(restoreStatus);

                        log_error(restoreStatus.m_error.c_str());

                        allGroupsRestored = false;
                        continue;
                    }

                    std::map<std::string, dto::srr::FeatureAndStatus> ftMap;
                    for(const auto& feature : group.m_features) {
                        ftMap[feature.m_feature_name] = feature.m_feature_and_status;
                    }

                    // create all restore queries related to the current group
                    // it helps to detect at an early stage if there are features missing in the restore payload
                    std::map<FeatureName, RestoreQuery> restoreQueriesMap;

                    try {
                        // loop through all required features to create the restore queries
                        for (const auto& feature : g_srrGroupMap.at(groupId).m_fp) {
                            const auto& featureName = feature.m_feature;
                            try{
                                const auto& dtoFeature = ftMap.at(featureName).feature();

                                // prepare restore queries
                                RestoreQuery& request = restoreQueriesMap[featureName];
                                request.set_passpharse(srrRestoreReq.m_passphrase);
                                request.set_session_token(srrRestoreReq.m_sessionToken);
                                request.mutable_map_features_data()->insert({featureName, dtoFeature});
                            } catch(const std::out_of_range& e) {
                                // missing feature, check if it required in restore payload version
                                const auto requiredIn = g_srrFeatureMap.at(featureName).m_requiredIn;
                                if(auto found = std::find(requiredIn.begin(), requiredIn.end(), srrRestoreReq.m_version); found != requiredIn.end()) {
                                    log_error("Feature %s is required in version %s", featureName.c_str(), srrRestoreReq.m_version);
                                    throw std::runtime_error("Feature " + featureName + " is required in version " + srrRestoreReq.m_version);
                                }
                            } catch (const std::exception& e) {
                                throw std::runtime_error(e.what());
                            }
                        }
                    }   // if one feature is missing, set the error for the whole group and skip the group
                    catch (std::out_of_range& ex) {
                        RestoreStatus restoreStatus;
                        restoreStatus.m_name = groupId;
                        restoreStatus.m_status = statusToString(Status::FAILED);
                        restoreStatus.m_error = TRANSLATE_ME("Group %s cannot be restored. Missing features", groupId.c_str());

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
                        for(const auto& feature : featureList) {
                            log_debug("Saving feature %s current status", feature.m_feature.c_str());
                            rollbackSaveResponse += saveFeature(feature.m_feature, srrRestoreReq.m_passphrase, srrRestoreReq.m_sessionToken);
                        }
                    }
                    catch (std::exception& ex) {
                        log_error("Could not backup feature %s", groupId.c_str());

                    }

                    // reset features in reverse order before restore
                    // WARNING: currently reset is not implemented by all features, hence it will not be mandatory
                    for(auto revIt = featureList.rbegin(); revIt != featureList.rend(); revIt++) {
                        if(g_srrFeatureMap.at(revIt->m_feature).m_reset) {
                            try{
                                resetFeature(revIt->m_feature);
                            }
                            catch (SrrResetFailed& ex) {
                                log_warning(ex.what());
                            }
                        }
                    }

                    bool restoreFailed = false;

                    RestoreStatus restoreStatus;
                    restoreStatus.m_name = groupId;
                    restoreStatus.m_status = statusToString(Status::SUCCESS);

                    // restore features in order
                    for(const auto& feature : group.m_features) {
                        const auto& featureName = feature.m_feature_name;

                        try{
                            // Restore feature
                            response += restoreFeature(featureName, restoreQueriesMap[featureName]);

                            // update restart flag
                            restart = restart | g_srrFeatureMap.at(featureName).m_restart;
                        }
                        catch (const std::exception& ex) {
                            // restore failed -> rolling back the whole group
                            restoreFailed = true;

                            restoreStatus.m_status = statusToString(Status::FAILED);
                            restoreStatus.m_error = TRANSLATE_ME("Restore failed for feature %s: ", featureName.c_str(), ex.what());

                            allGroupsRestored = false;
                            log_error(restoreStatus.m_error.c_str());

                            // stop group restore
                            break;
                        }

                        // wait to sync feature restore
                        std::this_thread::sleep_for(std::chrono::seconds(FEATURE_RESTORE_DELAY_SEC));
                    }

                    // if restore failed -> rollback
                    if(restoreFailed) {
                        restart = restart | rollback(rollbackSaveResponse, srrRestoreReq.m_passphrase);
                    }

                    // push group status into restore response
                    srrRestoreResp.m_status_list.push_back(restoreStatus);
                }

                if(allGroupsRestored) {
                    srrRestoreResp.m_status = statusToString(Status::SUCCESS);
                } else {
                    srrRestoreResp.m_status = statusToString(Status::PARTIAL_SUCCESS);
                }
            } else {
                throw SrrInvalidVersion();
            }
        }
        catch (const SrrIntegrityCheckFailed& e)
        {
            srrRestoreResp.m_status = statusToString(Status::UNKNOWN);
            srrRestoreResp.m_error = TRANSLATE_ME(e.what());

            log_error(srrRestoreResp.m_error.c_str());
        }
        catch (const std::exception& e)
        {
            srrRestoreResp.m_status = statusToString(Status::FAILED);
            srrRestoreResp.m_error = TRANSLATE_ME(e.what());

            log_error(srrRestoreResp.m_error.c_str());
        }

        cxxtools::SerializationInfo responseSi;
        responseSi <<= srrRestoreResp;

        dto::UserData response;
        std::string jsonResp = serializeJson(responseSi);
        response.push_back(srrRestoreResp.m_status);
        response.push_back(jsonResp);

        if(restart) {
            if(m_parameters.at(ENABLE_REBOOT_KEY) == "true") {
                std::thread restartThread(restartBiosService, SRR_RESTART_DELAY_SEC);
                restartThread.detach();
            } else {
                log_warning("Reboot is disabled in current configuration");
            }
        }

        return response;
    }

    dto::UserData SrrWorker::requestReset(const std::string& /* json */)
    {
        log_debug("SRR reset request");
        throw SrrException("Not implemented yet!");
    }

    bool SrrWorker::isVerstionCompatible(const std::string& version)
    {
        return m_supportedVersions.find(version) != m_supportedVersions.end();
    }

} // namespace srr
