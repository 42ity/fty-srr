/*  =========================================================================
    fty_srr_worker - Fty srr worker

    Copyright (C) 2014 - 2018 Eaton

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

#include <fty_srr_dto.h>
#include <fty_lib_certificate_library.h>

#include "fty_srr_classes.h"

using namespace dto::srr;

namespace srr
{    
    /**
     * Constructor
     * @param msgBus
     * @param parameters
     */
    SrrWorker::SrrWorker(messagebus::MessageBus& msgBus, const std::map<std::string, std::string>& parameters) :
        m_msgBus(msgBus), m_parameters(parameters)
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
            // Build map associations.
            buildMapAssociation();
            // Srr version
            m_srrVersion = m_parameters.at(SRR_VERSION_KEY);
        }        
        catch (messagebus::MessageBusException& ex)
        {
            throw SrrException(ex.what());
        } catch (...)
        {
            throw SrrException("Unexpected error: unknown");
        }
    }
    
    /**
     * Set all associations
     */
    void SrrWorker::buildMapAssociation()
    {
        // Feature -> Agent (fty-config)
        m_featuresToAgent [MONITORING_FEATURE_NAME] = {CONFIG_AGENT_NAME, TRANSLATE_ME((std::string(SRR_PREFIX_TRANSLATE_KEY) + MONITORING_FEATURE_NAME).c_str())};
        m_featuresToAgent [NOTIFICATION_FEATURE_NAME] = {CONFIG_AGENT_NAME, TRANSLATE_ME((std::string(SRR_PREFIX_TRANSLATE_KEY) + NOTIFICATION_FEATURE_NAME).c_str())};
        m_featuresToAgent [AUTOMATION_SETTINGS] = {CONFIG_AGENT_NAME, TRANSLATE_ME((std::string(SRR_PREFIX_TRANSLATE_KEY) + AUTOMATION_SETTINGS).c_str())};
        m_featuresToAgent [USER_SESSION_FEATURE_NAME] = {CONFIG_AGENT_NAME, TRANSLATE_ME((std::string(SRR_PREFIX_TRANSLATE_KEY) + USER_SESSION_FEATURE_NAME).c_str())};
        m_featuresToAgent [DISCOVERY] = {CONFIG_AGENT_NAME, TRANSLATE_ME((std::string(SRR_PREFIX_TRANSLATE_KEY) + DISCOVERY).c_str())};
        m_featuresToAgent [MASS_MANAGEMENT] = {CONFIG_AGENT_NAME, TRANSLATE_ME((std::string(SRR_PREFIX_TRANSLATE_KEY) + MASS_MANAGEMENT).c_str())};
        m_featuresToAgent [NETWORK] = {CONFIG_AGENT_NAME, TRANSLATE_ME((std::string(SRR_PREFIX_TRANSLATE_KEY) + NETWORK).c_str())};
        // Feature -> Agent (etn-malamute-translator EMC4J)
        m_featuresToAgent [AUTOMATIONS] = {EMC4J_AGENT_NAME, TRANSLATE_ME((std::string(SRR_PREFIX_TRANSLATE_KEY) + AUTOMATIONS).c_str())};
        m_featuresToAgent [VIRTUAL_ASSETS] = {EMC4J_AGENT_NAME, TRANSLATE_ME((std::string(SRR_PREFIX_TRANSLATE_KEY) + VIRTUAL_ASSETS).c_str())};
        // Feature -> Agent (security-wallet)
        m_featuresToAgent [SECURITY_WALLET] = {SECU_WALLET_AGENT_NAME, TRANSLATE_ME((std::string(SRR_PREFIX_TRANSLATE_KEY) + SECURITY_WALLET).c_str())};
        // Agent -> Queue
        m_agentToQueue [CONFIG_AGENT_NAME] = CONFIG_MSG_QUEUE_NAME;
        m_agentToQueue [EMC4J_AGENT_NAME] = EMC4J_MSG_QUEUE_NAME;
        m_agentToQueue [SECU_WALLET_AGENT_NAME] = SECU_WALLET_MSG_QUEUE_NAME;
    }
   
    /**
     * Get feature list managed
     * @param msg
     * @param subject
     */
    ListFeatureResponse SrrWorker::getFeatureListManaged(const ListFeatureQuery& query)
    {
        std::map<const std::string, config> features = m_featuresToAgent;
        ListFeatureResponse response;
        
        // Automation with dependencies
        ListFeatureResponse featureAutomation;
        FeatureDependencies automationDep;
        automationDep.set_description(m_featuresToAgent[AUTOMATIONS].featureDescription);
        automationDep.add_dependencies(AUTOMATION_SETTINGS);
        automationDep.add_dependencies(VIRTUAL_ASSETS);
        // TODO remove the following comment, when asset UUID will be implemented, and automation could use it.
        //featureAutomation.mutable_map_features_dependencies()->insert({AUTOMATIONS, automationDep});
        
        // Remove it to do not add twice.
        features.erase(AUTOMATIONS);
        // TODO remove this when fty-asset will be refactored.
        features.erase(VIRTUAL_ASSETS);

        // Remaining features
        ListFeatureResponse otherFeatures;
        for (const auto& feature : features)
        {
            FeatureDependencies featDep;
            featDep.set_description(feature.second.featureDescription);
            otherFeatures.mutable_map_features_dependencies()->insert({feature.first, featDep});
        }
        
        // Features concatenation        
        response += featureAutomation;
        response += otherFeatures;
        response.set_version(m_srrVersion);
        response.set_passphrass_definition(fty::getPassphraseFormat());
        response.set_passphrass_description(TRANSLATE_ME("Passphrase must have %s characters", (fty::getPassphraseFormat()).c_str()));
        return response;
    }
    
    /**
     * Save an Ipm2 configuration
     * @param msg
     * @param query
     */
    SaveResponse SrrWorker::saveIpm2Configuration(const SaveQuery& query)
    {
        SaveResponse response;
        FeatureStatus status;
        status.set_status(Status::FAILED);
        try
        {
            bool checkPassphraseFormat = fty::checkPassphraseFormat(query.passpharse());
            if (checkPassphraseFormat)
            {
                log_info("Save IPM2 configuration processing");
                // Try to factorize all call.
                std::map<std::string, std::set<FeatureName>> agentAssoc = factorizationSaveCall(query);

                for(auto const& agent: agentAssoc)
                {
                      // Get queue name from agent name
                      std::string agentNameDest = agent.first;
                      std::string queueNameDest = m_agentToQueue.at(agentNameDest);

                      log_info("Saving configuration by: %s ", agentNameDest.c_str());
                      // Build query
                      Query saveQuery = createSaveQuery({agent.second}, query.passpharse());
                      // Send message
                      dto::UserData reqData;
                      reqData << saveQuery;
                      messagebus::Message resp = sendRequest(reqData, "save", queueNameDest, agentNameDest);
                      Response partialResp;
                      resp.userData() >> partialResp;

                      log_info("Save done by: %s: response: %s ", agentNameDest.c_str(), responseToUiJson(partialResp, false).c_str());
                      response += partialResp.save();
                }
                response.set_version(m_srrVersion);
                status.set_status(Status::SUCCESS);
                *(response.mutable_status()) = status;
                response.set_checksum(fty::encrypt(query.passpharse(), query.passpharse()));
            }
            else
            {
                std::string errorMsg = TRANSLATE_ME("Passphrase must have %s characters", (fty::getPassphraseFormat()).c_str());
                log_error(errorMsg.c_str());
                status.set_error(errorMsg);
                response = (createSaveResponse(m_srrVersion, status)).save();
            }
        }
        catch (const std::exception& e)
        {
            std::string errorMsg = TRANSLATE_ME("Exception on save Ipm2 configuration: (%s)", e.what());
            log_error(errorMsg.c_str());
            status.set_error(errorMsg);
            response = (createSaveResponse(m_srrVersion, status)).save();
        }
        return response;
    }
    
    /**
     * Restore an Ipm2 Configuration
     * @param msg
     * @param query
     */
    RestoreResponse SrrWorker::restoreIpm2Configuration(const RestoreQuery& query)
    {
        RestoreResponse response;
        FeatureStatus status;
        status.set_status(Status::FAILED);
        try
        {
            std::string decryptedData = fty::decrypt(query.checksum(), query.passpharse());
            if (decryptedData.compare(query.passpharse()) == 0)
            {
                log_info("Restore IPM2 configuration processing");
                std::string version = query.version();
                // Test version compatibility.
                bool compatible = isVerstionCompatible(query.version());
                if (compatible)
                {
                    // Try to factorize all call.
                    std::map<std::string, RestoreQuery> agentAssoc = factorizationRestoreCall(query);
                    for(auto const& agent: agentAssoc)
                    {
                        // Get queue name from agent name
                        std::string agentNameDest = agent.first;
                        std::string queueNameDest = m_agentToQueue.at(agentNameDest);
                        // Build query
                        Query restoreQuery;
                        *(restoreQuery.mutable_restore()) = agent.second;
                        log_info("Restoring configuration by: %s ", agentNameDest.c_str());
                        // Send message
                        dto::UserData reqData;
                        reqData << restoreQuery;
                        messagebus::Message resp = sendRequest(reqData, "restore", queueNameDest, agentNameDest);
                        Response partialResp;
                        resp.userData() >> partialResp;
                        
                        log_info("Restore done by: %s: response: %s ", agentNameDest.c_str(), responseToUiJson(partialResp, false).c_str());
                        response += partialResp.restore();
                    }
                    status.set_status(Status::SUCCESS);
                    *(response.mutable_status()) = status;
                }
                else
                {
                    std::string errorMsg = TRANSLATE_ME("Srr version (%s) is not compatible with the restore version request: (%s)", m_srrVersion.c_str(), version.c_str());
                    log_error(errorMsg.c_str());
                    status.set_error(errorMsg);
                    // Set response
                    response = (createRestoreResponse(status)).restore();
                }
            }
            else
            {
                std::string errorMsg = TRANSLATE_ME("Passphrase does not match");
                log_error(errorMsg.c_str());
                status.set_error(errorMsg);
                // Set response
                response = (createRestoreResponse(status)).restore();
            }
        }
        catch (const std::exception& e)
        {
            std::string errorMsg = TRANSLATE_ME("Exception on restore Ipm2 configuration: (%s)", e.what());
            log_error(errorMsg.c_str());
            std::cout << e.what() << std::endl;
            status.set_error(errorMsg);
            response = (createRestoreResponse(status)).restore();
        }
        return response;
    }

    
    /**
     * Reset an Ipm2 Configuration
     * @param msg
     * @param query
     */
    ResetResponse SrrWorker::resetIpm2Configuration(const dto::srr::ResetQuery& query)
    {
        throw SrrException("Not implemented yet!");
    }
    
    /**
     * Save factorization by agent name.
     * @param siFeatureList
     * @param association
     */
    std::map<std::string, std::set<FeatureName>> SrrWorker::factorizationSaveCall(const SaveQuery query)
    {
        std::map<std::string, std::set<FeatureName>> assoc;
        for(const auto& featureName: query.features())
        {
            std::string agentName = m_featuresToAgent[featureName].agentName;
            assoc[agentName].insert(featureName);
        }
        return assoc;
    }
    
    /**
     * Restore factorization by agent name.
     * @param siFeatureList
     * @param association
     */
    std::map<std::string, RestoreQuery> SrrWorker::factorizationRestoreCall(const RestoreQuery query)
    {  
        std::map<std::string, RestoreQuery> assoc;
        std::map<FeatureName, Feature> map1(query.map_features_data().begin(), query.map_features_data().end());
        for(const auto& item:  map1)
        {
            const std::string & agentName = m_featuresToAgent[item.first].agentName;
            RestoreQuery& request = assoc[agentName];
            request.set_passpharse(query.passpharse());
            request.mutable_map_features_data()->insert({item.first, item.second});
            
        }
        return assoc;
    }
    
    /**
     * Send a response on the message bus.
     * @param msg
     * @param payload
     * @param subject
     */
    messagebus::Message SrrWorker::sendRequest(const dto::UserData& userData, const std::string& action, const std::string& queueNameDest, const std::string& agentNameDest)
    {
        messagebus::Message resp;
        try
        {
            int timeout = std::stoi(m_parameters.at(REQUEST_TIMEOUT_KEY)) / 1000;
            messagebus::Message req;
            req.userData() = userData;
            req.metaData().emplace(messagebus::Message::SUBJECT, action);
            req.metaData().emplace(messagebus::Message::FROM, m_parameters.at(AGENT_NAME_KEY));
            req.metaData().emplace(messagebus::Message::TO, agentNameDest);
            req.metaData().emplace(messagebus::Message::CORRELATION_ID, messagebus::generateUuid());
            resp = m_msgBus.request(queueNameDest, req, timeout);
        }
        catch (messagebus::MessageBusException& ex)
        {
            throw SrrException(ex.what());
        } catch (...)
        {
            throw SrrException("Unknown error on send response to the message bus");
        }
        return resp;
    }
    
    bool SrrWorker::isVerstionCompatible(const std::string& version)
    {
        bool comptible = false;
        int srrVersion = std::stoi(m_srrVersion);
        int requestVersion = std::stoi(version);
        
        if (srrVersion >= requestVersion)
        {
            comptible = true;
        }
        return comptible;
    }
    
} // namespace srr
