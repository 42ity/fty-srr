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
        logInfo("Rebooting in {} seconds...", i);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    logInfo("Reboot");

    // write out buffer to disk
    sync();

    int r = std::system("sudo /usr/sbin/fty-srr-reboot.sh");
    if (r != 0) {
        logError("failed to run reboot procedure (r: {})", r);
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
            logWarn("Feature {} not found in g_srrFeatureMap", feature);
        }
    }

    return map;
}

/// Send a request to agentNameDest on queueNameDest with action as subject
messagebus::Message sendRequest(messagebus::MessageBus& msgbus, const dto::UserData& userData,
    const std::string& action, const std::string& from, const std::string& queueNameDest,
    const std::string& agentNameDest, int timeout_s)
{
    logDebug("Send '{}' request to {}/{} (from: {}, timeout: {} sec.)",
        action, agentNameDest, queueNameDest, from, timeout_s);

    messagebus::Message resp;
    try {
        messagebus::Message req;
        req.userData() = userData;
        req.metaData().emplace(messagebus::Message::SUBJECT, action);
        req.metaData().emplace(messagebus::Message::FROM, from);
        req.metaData().emplace(messagebus::Message::TO, agentNameDest);
        req.metaData().emplace(messagebus::Message::CORRELATION_ID, messagebus::generateUuid());

        resp = msgbus.request(queueNameDest, req, timeout_s);
    } catch (messagebus::MessageBusException& e) {
        logError("Send '{}' request to {}/{} failed (e: {})", action, agentNameDest, queueNameDest, e.what());
        throw SrrException(e.what());
    } catch (...) {
        logError("Send '{}' request to {}/{} failed", action, agentNameDest, queueNameDest);
        throw SrrException("Unexpected error on send request");
    }

    logDebug("Receive '{}' response from {}",
        resp.metaData().at(messagebus::Message::SUBJECT),
        resp.metaData().at(messagebus::Message::FROM));

    return resp;
}

} // namespace srr
