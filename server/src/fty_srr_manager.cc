/*  =========================================================================
    fty_srr_server - Fty srr server

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
    fty_srr_server - Fty srr server
@discuss
@end
 */

#include "fty-srr.h"
#include "fty_srr_exception.h"
#include "fty_srr_manager.h"
#include "fty_srr_worker.h"

#include <algorithm>
#include <functional>
#include <thread>

using namespace std::placeholders;
using namespace dto::srr;

// clang-format off

namespace srr
{
    
    const std::map<const std::string, RequestType> SrrRequestProcessor::m_requestType = {
        {"list"    , RequestType::REQ_LIST},
        {"save"    , RequestType::REQ_SAVE},
        {"restore" , RequestType::REQ_RESTORE},
        {"reset"   , RequestType::REQ_RESET}
    };

    dto::UserData SrrRequestProcessor::processRequest(const std::string& operation, const dto::UserData& data)
    {
        dto::UserData response;

        RequestType op = m_requestType.find(operation) != m_requestType.end() ? m_requestType.at(operation) : RequestType::REQ_UNKNOWN;

        switch(op)
        {
            case RequestType::REQ_LIST :
                if(!listHandler) throw std::runtime_error("No list feature handler!");
                response = listHandler();
                break;

            case RequestType::REQ_SAVE :
                if(!saveHandler) throw std::runtime_error("No save handler!");
                response = saveHandler(data.front());
                break;

            case RequestType::REQ_RESTORE :
                if(!restoreHandler) throw std::runtime_error("No restore handler!");
                response = restoreHandler(data.front(), data.size() > 1);
                break;
            
            case RequestType::REQ_RESET :
                if(!resetHandler) throw std::runtime_error("No reset handler!");
                response = resetHandler(data.front());
                break;
            
            case RequestType::REQ_UNKNOWN:
            default:
                throw std::runtime_error("Unknown query!");
                break;
        }

        return response;
    }

    /**
     * Constructor
     * @param parameters
     * @param streamPublisher
     */
    SrrManager::SrrManager(const std::map<std::string, std::string> & parameters)
    : m_parameters(parameters)
    {
        init();
    }
    
    /**
     * Class initialization 
     */
    void SrrManager::init()
    {
        try
        {
            // Back end bus init
            m_backEndBus = std::unique_ptr<messagebus::MessageBus>(messagebus::MlmMessageBus(m_parameters.at(ENDPOINT_KEY), m_parameters.at(AGENT_NAME_KEY)));
            m_backEndBus->connect();
            
            // UI messagebus bus init
            m_uiBus = std::unique_ptr<messagebus::MessageBus>(messagebus::MlmMessageBus(m_parameters.at(ENDPOINT_KEY), m_parameters.at(AGENT_NAME_KEY) + "-ui"));
            m_uiBus->connect();
            
            // Worker creation.
            m_srrworker = std::unique_ptr<srr::SrrWorker>(new srr::SrrWorker(*m_backEndBus, m_parameters, {"1.0", "2.0", "2.1"}));
            
            // Bind all processor handler.
            m_processor.listHandler = std::bind(&SrrWorker::getGroupList, m_srrworker.get());
            m_processor.saveHandler = std::bind(&SrrWorker::requestSave, m_srrworker.get(), _1);
            m_processor.restoreHandler = std::bind(&SrrWorker::requestRestore, m_srrworker.get(), _1, _2);
            m_processor.resetHandler = std::bind(&SrrWorker::requestReset, m_srrworker.get(), _1);
            
            // Listen all incoming UI requests           
            auto uiFct = std::bind(&SrrManager::handleRequest, this, _1);
            m_uiBus->receive(m_parameters.at(SRR_QUEUE_NAME_KEY) + ".UI", uiFct);
        }        
        catch (messagebus::MessageBusException& ex)
        {
            log_error("Message bus error: %s", ex.what());
            throw SrrException("Failed to open connection with message bus!");
        } catch (...)
        {
            log_error("Unexpected error: unknown");
            throw SrrException("Unexpected error: unknown");
        }
    }

    void SrrManager::uiMsgHandler(const messagebus::Message& msg)
    {
        log_debug("uiMsgHandler");

        dto::UserData response;

        try
        {
            const std::string& op = msg.metaData().at(messagebus::Message::SUBJECT);

            response = m_processor.processRequest(op, msg.userData());
            // Send response
        }        
        catch (std::exception& ex)
        {
            response.push_back(ex.what());
            log_error(ex.what());
        }

        sendUiResponse(msg, response);
    }

    /**
     * Handle incoming requests from UI
     * @param sender
     * @param payloadea
     * @return 
     */
    void SrrManager::handleRequest(messagebus::Message msg)
    {
        log_debug("handle request");
        
        auto handler = std::bind(&SrrManager::uiMsgHandler, this, msg);
        std::thread t(handler);

        t.detach();
    }

    /**
     * Send response on message bus
     * @param msg
     * @param userData
     */
    void SrrManager::sendResponse(const messagebus::Message& msg, const dto::UserData& userData)
    {
        try
        {
            messagebus::Message respMsg;
            respMsg.userData() = userData;
            respMsg.metaData().emplace(messagebus::Message::SUBJECT, msg.metaData().at(messagebus::Message::SUBJECT));
            respMsg.metaData().emplace(messagebus::Message::FROM, m_parameters.at(AGENT_NAME_KEY));
            respMsg.metaData().emplace(messagebus::Message::TO, msg.metaData().find(messagebus::Message::FROM)->second);
            respMsg.metaData().emplace(messagebus::Message::CORRELATION_ID, msg.metaData().find(messagebus::Message::CORRELATION_ID)->second);
            m_backEndBus->sendReply(msg.metaData().find(messagebus::Message::REPLY_TO)->second, respMsg);
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
     * Send response to UI
     * @param msg
     * @param userData
     */
    void SrrManager::sendUiResponse(const messagebus::Message& msg, const dto::UserData& userData)
    {
        try
        {
            messagebus::Message respMsg;
            respMsg.userData() = userData;
            respMsg.metaData().emplace(messagebus::Message::SUBJECT, msg.metaData().at(messagebus::Message::SUBJECT));
            respMsg.metaData().emplace(messagebus::Message::FROM, m_parameters.at(AGENT_NAME_KEY));
            respMsg.metaData().emplace(messagebus::Message::TO, msg.metaData().find(messagebus::Message::FROM)->second);
            respMsg.metaData().emplace(messagebus::Message::CORRELATION_ID, msg.metaData().find(messagebus::Message::CORRELATION_ID)->second);
            m_uiBus->sendReply(msg.metaData().find(messagebus::Message::REPLY_TO)->second, respMsg);
        }
        catch (messagebus::MessageBusException& ex)
        {
            throw SrrException(ex.what());
        } catch (...)
        {
            throw SrrException("Unknown error on send response to the message bus");
        }
    }
}

