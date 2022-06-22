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

#include <fty_common_dto.h>
#include <fty_common_messagebus.h>
#include <fty_userdata_dto.h>
#include <map>
#include <set>
#include <string>

namespace srr {

class SrrWorker
{
public:
    SrrWorker(messagebus::MessageBus& msgBus, const std::map<std::string, std::string>& parameters,
        const std::set<std::string>& supportedVersions);
    ~SrrWorker() = default;

    // UI interface
    dto::UserData getGroupList();
    dto::UserData requestSave(const std::string& json);
    dto::UserData requestRestore(const std::string& json, bool force = false);
    dto::UserData requestReset(const std::string& json);

private:
    messagebus::MessageBus&            m_msgBus;
    std::map<std::string, std::string> m_parameters;
    std::string                        m_srrVersion;

    std::set<std::string> m_supportedVersions;

    int m_sendTimeout_s; // sec.

    void init();

    // SRR methods
    dto::srr::SaveResponse saveFeature(const dto::srr::FeatureName& featureName, const std::string& passphrase, const std::string& sessionToken);
    dto::srr::RestoreResponse restoreFeature(const dto::srr::FeatureName& featureName, const dto::srr::RestoreQuery& query);
    dto::srr::ResetResponse resetFeature(const dto::srr::FeatureName& featureName);

    bool rollback(const dto::srr::SaveResponse& rollbackSaveResponse, const std::string& passphrase);
};

} // namespace srr
