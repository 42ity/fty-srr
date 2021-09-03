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

#ifndef FTY_SRR_MANAGER_H_INCLUDED
#define FTY_SRR_MANAGER_H_INCLUDED

#include <fty_common_dto.h>
#include <fty_common_messagebus.h>
#include <fty_userdata_dto.h>
#include <map>
#include <memory>
#include <string>

/**
 * \brief Agent srr server
 */

namespace srr {
class SrrWorker;

enum class RequestType
{
    REQ_UNKNOWN,
    REQ_LIST,
    REQ_SAVE,
    REQ_RESTORE,
    REQ_RESET
};

class SrrRequestProcessor
{
public:
    static const std::map<const std::string, RequestType> m_requestType;

    std::function<dto::UserData()>                         listHandler;
    std::function<dto::UserData(const std::string&)>       saveHandler;
    std::function<dto::UserData(const std::string&, bool)> restoreHandler;
    std::function<dto::UserData(const std::string&)>       resetHandler;

    dto::UserData processRequest(const std::string& operation, const dto::UserData& data);
};

class SrrManager
{
public:
    explicit SrrManager(const std::map<std::string, std::string>& parameters);
    ~SrrManager() = default;

private:
    std::map<std::string, std::string> m_parameters;
    // back end bus handles communication with all the agents (can't receive requests)
    std::unique_ptr<messagebus::MessageBus> m_backEndBus;
    // UI bus handles incoming requests from UI
    std::unique_ptr<messagebus::MessageBus> m_uiBus;
    std::unique_ptr<srr::SrrWorker>         m_srrworker;

    SrrRequestProcessor m_processor;

    void init();
    void handleRequest(messagebus::Message msg);

    void sendResponse(const messagebus::Message& msg, const dto::UserData& userData);
    void sendUiResponse(const messagebus::Message& msg, const dto::UserData& userData);

    void uiMsgHandler(const messagebus::Message& msg);
};

} // namespace srr

#endif
