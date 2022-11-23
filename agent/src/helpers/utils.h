/*  =========================================================================
    fty_common_messagebus_exception - class description

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

#include <fty_common_dto.h>
#include <map>
#include <string>

namespace messagebus {
class Message;
class MessageBus;
} // namespace messagebus

namespace srr {

// reboot bios
void restartBiosService(const unsigned restartDelay);

// start/stop the certificate manager service
void certmanager(bool start);

std::map<std::string, std::set<dto::srr::FeatureName>> groupFeaturesByAgent(
    const std::list<dto::srr::FeatureName>& features);

messagebus::Message sendRequest(messagebus::MessageBus& msgbus, const dto::UserData& userData,
    const std::string& action, const std::string& from, const std::string& queueNameDest,
    const std::string& agentNameDest, int timeout = 60);

} // namespace srr
