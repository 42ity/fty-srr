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

#include "fty_srr_classes.h"

namespace srr
{
    // Timout for Request/Reply in s
    const int TIME_OUT = 4;
    
    /**
     * Constructor
     * @param msgBus
     * @param parameters
     */
    SrrWorker::SrrWorker(messagebus::MessageBus* msgBus, const std::map<std::string, std::string>& parameters)
    {
        m_msgBus = msgBus;
        m_parameters = parameters;
        init();
    }

    /**
     * Destructor
     */
    SrrWorker::~SrrWorker()
    {
    }
    
    /**
     * Init srr worker
     */
    void SrrWorker::init()
    {
        try
        {
            // Set features associations.
            buildFeaturesAssociation();
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
     * Set features associations
     */
    void SrrWorker::buildFeaturesAssociation()
    {
        m_featuresAssociation ["monitoring"] = FeaturesAssociation(CONFIG_AGENT_NAME, CONFIG_MSG_QUEUE_NAME);
        m_featuresAssociation ["notification"] = FeaturesAssociation(CONFIG_AGENT_NAME, CONFIG_MSG_QUEUE_NAME);
        m_featuresAssociation ["user-session"] = FeaturesAssociation(CONFIG_AGENT_NAME, CONFIG_MSG_QUEUE_NAME);
        m_featuresAssociation ["automation-settings"] = FeaturesAssociation(CONFIG_AGENT_NAME, CONFIG_MSG_QUEUE_NAME);
        m_featuresAssociation ["network"] = FeaturesAssociation(CONFIG_AGENT_NAME, CONFIG_MSG_QUEUE_NAME);
        m_featuresAssociation ["automations"] = FeaturesAssociation(EMC4J_AGENT_NAME, EMC4J_MSG_QUEUE_NAME);
        m_featuresAssociation ["virtual-assets"] = FeaturesAssociation(EMC4J_AGENT_NAME, EMC4J_MSG_QUEUE_NAME);
        m_featuresAssociation ["automations,virtual-assets"] = FeaturesAssociation(EMC4J_AGENT_NAME, EMC4J_MSG_QUEUE_NAME);
    }
    
    /**
     * Get feature list managed
     * @param msg
     * @param subject
     */
    void SrrWorker::getFeatureListManaged(const messagebus::Message& msg, const std::string& subject)
    {
        dto::srr::SrrFeaturesListDto featuresListdto;
        for (const auto& feature : m_featuresAssociation)
        {
            log_debug("Feature name: %s", feature.first.c_str());
            featuresListdto.featuresList.push_back(feature.first);
        }
        // Send Response
        messagebus::UserData userData;
        userData << featuresListdto;
        sendResponse(msg, userData, subject);
    }
    
    /**
     * Save an Ipm2 configuration
     * @param msg
     * @param query
     */
    void SrrWorker::saveIpm2Configuration(const messagebus::Message& msg, const dto::srr::SrrQueryDto& query)
    {
        std::string saveResp;
        try
        {
            log_debug("Save IPM2 configuration processing");
            // Global serialization info.
            cxxtools::SerializationInfo ipm2ConfSi = buildIpm2ConfigurationStruct();
            
            // version check
            std::string version = m_parameters.at(SRR_VERSION_KEY);
            log_debug("Payload %s", query.data.c_str());
            
            if (query.data.size() > 0)
            {
                cxxtools::SerializationInfo si;
                JSON::readFromString(query.data, si);            
                cxxtools::SerializationInfo siFeatureList = si.getMember(FEATURE_LIST_NAME);

                for (const auto &feature : siFeatureList)
                {
                    std::string featureName = "";
                    feature.getMember(FEATURE_NAME).getValue(featureName);
                    log_debug("Try to get all settings for: %s", featureName.c_str());
                    
                    FeaturesAssociation featureAssoc = m_featuresAssociation.at(featureName);
                    log_debug("Send request at '%s', to queue '%s', from '%s'", featureAssoc.agentName.c_str(), 
                            featureAssoc.queueName.c_str(), m_parameters.at(AGENT_NAME_KEY).c_str()); 

                    messagebus::Message req;
                    dto::config::ConfigQueryDto configQuery;
                    configQuery.action = SAVE_ACTION;
                    configQuery.featureName = featureName;

                    req.userData() << configQuery;
                    req.metaData().emplace(messagebus::Message::SUBJECT, SAVE_ACTION);
                    req.metaData().emplace(messagebus::Message::FROM, m_parameters.at(AGENT_NAME_KEY));
                    req.metaData().emplace(messagebus::Message::TO, featureAssoc.agentName);
                    req.metaData().emplace(messagebus::Message::COORELATION_ID, messagebus::generateUuid());
                    // Send request
                    messagebus::Message resp = m_msgBus->request(featureAssoc.queueName, req, TIME_OUT);

                    log_debug("Settings for: '%s' retrieved", featureName.c_str());
                    // Response serialization 
                    messagebus::UserData data = resp.userData();
                    dto::config::ConfigResponseDto configResponse;
                    data >> configResponse;
                    // Get data member
                    cxxtools::SerializationInfo& si = *(ipm2ConfSi.findMember(DATA_MEMBER));
                    cxxtools::SerializationInfo siConfigResp;
                    // Get the serializationInfo from data response
                    JSON::readFromString (configResponse.data, siConfigResp);
                    // Iterate on array result
                    cxxtools::SerializationInfo::Iterator it = siConfigResp.begin();
                    do
                    {
                        si.addMember("") <<= (cxxtools::SerializationInfo)*it;
                        ++it;
                    } while (it != siConfigResp.end());
                }
                saveResp = JSON::writeToString(ipm2ConfSi, false);
            }
            else 
            {
                throw SrrException("Query data is empty");
            }
        }
        catch (messagebus::MessageBusException& ex)
        {
            log_error(ex.what());
        }
        catch (...)
        {
            log_error("Unknown error on save Ipm2 configuration");
        }
        // Send Response
        messagebus::UserData userData;
        userData.push_back(saveResp);
        sendResponse(msg, userData, query.action);
    }
    
    /**
     * Restore an Ipm2 Configuration
     * @param msg
     * @param query
     */
    void SrrWorker::restoreIpm2Configuration(const messagebus::Message& msg, const dto::srr::SrrQueryDto& query)
    {
        dto::srr::SrrRestoreDtoList respList;
        respList.status = STATUS_UNKNOWN;
        try
        {
            std::string globalStatus = "";
            log_debug("Data to set %s:", query.data.c_str());
            // Get the si from the request
            cxxtools::SerializationInfo si;
            JSON::readFromString(query.data, si);
            cxxtools::SerializationInfo siData = si.getMember(DATA_MEMBER);
            // Test if the data is well formated
            if (siData.category () != cxxtools::SerializationInfo::Array ) 
            {
                throw std::invalid_argument(DATA_MEMBER " should be an array");
            }
            // Iterate on data array
            cxxtools::SerializationInfo::Iterator it;
            for (it = siData.begin(); it != siData.end(); ++it)
            {
                cxxtools::SerializationInfo siTemp = (cxxtools::SerializationInfo)*it;
                for (auto &siFeature : siTemp)
                {   
                    std::string featureName = siFeature.name();
                    dto::srr::SrrRestoreDto srrResponseDto(featureName);
                    // Get association (queue, etc.)
                    FeaturesAssociation featureAssoc = m_featuresAssociation.at(featureName);
                    // Build query
                    dto::config::ConfigQueryDto configQuery(RESTORE_ACTION);
                    configQuery.featureName = featureName;
                    configQuery.data = "[{" + JSON::writeToString(siFeature, false) + "}]";
                    log_debug("Data to set %s:", configQuery.data.c_str());
                    //Send message
                    messagebus::Message req;
                    req.userData() << configQuery;
                    req.metaData().emplace(messagebus::Message::SUBJECT, RESTORE_ACTION);
                    req.metaData().emplace(messagebus::Message::FROM, m_parameters.at(AGENT_NAME_KEY));
                    req.metaData().emplace(messagebus::Message::TO, featureAssoc.agentName);
                    req.metaData().emplace(messagebus::Message::COORELATION_ID, messagebus::generateUuid());
                    messagebus::Message resp = m_msgBus->request(featureAssoc.queueName, req, TIME_OUT);
                    // Serialize response
                    messagebus::UserData data = resp.userData();
                    dto::config::ConfigResponseDto respDto;
                    data >> respDto;
                    srrResponseDto.status = respDto.status;
                    srrResponseDto.error = respDto.error;

                    if ((respDto.status.compare(STATUS_FAILED) == 0 && respList.status.compare(STATUS_SUCCESS) == 0 )||
                        (respDto.status.compare(STATUS_SUCCESS) == 0 && respList.status.compare(STATUS_FAILED) == 0))
                    {
                        respList.status = STATUS_PARTIAL_SUCCESS;
                    }
                    else 
                    {
                        respList.status = respDto.status;
                    }
                    respList.responseList.push_back(srrResponseDto);
                }
            }
        } 
        catch (std::exception& ex)
        {
            throw SrrException(ex.what());
        }
        catch (...)
        {
            throw SrrException("Unknown error on restore Ipm2 configuration");
        }
        // Send Response
        messagebus::UserData userData;
        userData << respList;
        sendResponse(msg, userData, query.action);
    }
    
    /**
     * Send a response on the message bus.
     * @param msg
     * @param payload
     * @param subject
     */
    void SrrWorker::sendResponse(const messagebus::Message& msg, const messagebus::UserData& userData, const std::string& subject)
    {
        try
        {
            messagebus::Message respMsg;
            respMsg.userData() = userData;
            respMsg.metaData().emplace(messagebus::Message::SUBJECT, subject);
            respMsg.metaData().emplace(messagebus::Message::FROM, m_parameters.at(AGENT_NAME_KEY));
            respMsg.metaData().emplace(messagebus::Message::TO, msg.metaData().find(messagebus::Message::FROM)->second);
            respMsg.metaData().emplace(messagebus::Message::COORELATION_ID, msg.metaData().find(messagebus::Message::COORELATION_ID)->second);
            m_msgBus->sendReply(msg.metaData().find(messagebus::Message::REPLY_TO)->second, respMsg);
        }
        catch (messagebus::MessageBusException& ex)
        {
            throw SrrException(ex.what());
        } catch (...)
        {
            throw SrrException("Unknown error on send response to the message bus");
        }
    }
    
    /**
     * Utilitary to build an Ipm2 configuration si
     * @return 
     */
    cxxtools::SerializationInfo SrrWorker::buildIpm2ConfigurationStruct()
    {
        cxxtools::SerializationInfo si;
        si.addMember(SRR_VERSION_KEY) <<= ACTIVE_VERSION;
        // Data
        cxxtools::SerializationInfo& siData = si.addMember(DATA_MEMBER);
        siData.setCategory(cxxtools::SerializationInfo::Category::Array);
        return si;
    }
    
    /**
     *  Utilitary to build a response payload.
     * @return 
     */
    void SrrWorker::buildResponsePayload(const std::string& featureName, cxxtools::SerializationInfo& siOutput, cxxtools::SerializationInfo& siInput)
    {
        // Data
        cxxtools::SerializationInfo& si = *(siOutput.findMember(DATA_MEMBER));
        si.addMember(featureName) = siInput;
    }
    
} // namespace srr
