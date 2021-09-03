/*  =========================================================================
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

#include "utils.h"
#include "dto/common.h"
#include "fty_srr_exception.h"
#include "fty_srr_groups.h"
#include <fty_common.h>
#include <fty_common_messagebus.h>
#include <thread>
#include <unistd.h>

namespace srr {
// restart method
void restartBiosService(const unsigned restartDelay)
{
    for (unsigned i = restartDelay; i > 0; i--) {
        log_info("Rebooting in %d seconds...", i);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    log_info("Reboot");
    // write out buffer to disk
    sync();
    int ret = std::system("sudo /usr/sbin/fty-srr-reboot.sh");
    if (ret) {
        log_error("failed to run reboot procedure");
    }
}


std::map<std::string, std::set<dto::srr::FeatureName>> groupFeaturesByAgent(
    const std::list<dto::srr::FeatureName>& features)
{
    std::map<std::string, std::set<dto::srr::FeatureName>> map;

    for (const auto& feature : features) {
        try {
            const std::string& agentName = g_srrFeatureMap.at(feature).m_agent;
            map[agentName].insert(feature);
        } catch (std::out_of_range&) {
            log_warning("Feature %s not found", feature.c_str());
        }
    }

    return map;
}

/**
 * Send a response on the message bus.
 * @param msg
 * @param payload
 * @param subject
 */
messagebus::Message sendRequest(messagebus::MessageBus& msgbus, const dto::UserData& userData,
    const std::string& action, const std::string& from, const std::string& queueNameDest,
    const std::string& agentNameDest, int timeout)
{
    log_debug("Send message from %s to %s:%s with action %s", from.c_str(), agentNameDest.c_str(),
        queueNameDest.c_str(), action.c_str());
    // for(const auto& msg : userData) {
    //     log_debug("data:\n%s\n", msg.c_str());
    // }

    messagebus::Message resp;
    try {
        messagebus::Message req;
        req.userData() = userData;
        req.metaData().emplace(messagebus::Message::SUBJECT, action);
        req.metaData().emplace(messagebus::Message::FROM, from);
        req.metaData().emplace(messagebus::Message::TO, agentNameDest);
        req.metaData().emplace(messagebus::Message::CORRELATION_ID, messagebus::generateUuid());
        resp = msgbus.request(queueNameDest, req, timeout);
    } catch (messagebus::MessageBusException& ex) {
        throw SrrException(ex.what());
    } catch (...) {
        throw SrrException("Unknown error on send response to the message bus");
    }

    log_debug("Message received from %s with action %s", resp.metaData().at(messagebus::Message::FROM).c_str(),
        resp.metaData().at(messagebus::Message::SUBJECT).c_str());

    // for(const auto& msg : resp.userData()) {
    //     log_debug("data:\n%s\n", msg.c_str());
    // }
    return resp;
}

} // namespace srr
