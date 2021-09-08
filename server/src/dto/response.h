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

#pragma once

#include "common.h"
#include <cxxtools/serializationinfo.h>
#include <string>
#include <vector>

namespace srr {
// si list reponse fields
static constexpr const char* SI_PASSPHRASE_DESCRIPTION = "passphrase_description";
static constexpr const char* SI_PASSPHRASE_VALIDATION  = "passphrase_validation";

// si restore response fields
static constexpr const char* SI_STATUS_LIST = "status_list";

class SrrListResponse
{
public:
    SrrListResponse(){};

    std::string m_version;
    std::string m_passphrase_description;
    std::string m_passphrase_validation;

    std::vector<GroupInfo> m_groups;
};

void operator<<=(cxxtools::SerializationInfo& si, const SrrListResponse& resp);
void operator>>=(const cxxtools::SerializationInfo& si, SrrListResponse& resp);

class SrrSaveResponse
{
public:
    SrrSaveResponse(){};
    std::string m_status;
    std::string m_error;

    std::string        m_version;
    std::string        m_checksum;
    std::vector<Group> m_data;
};

void operator<<=(cxxtools::SerializationInfo& si, const SrrSaveResponse& resp);
void operator>>=(const cxxtools::SerializationInfo& si, SrrSaveResponse& resp);

class SrrRestoreResponse
{
public:
    SrrRestoreResponse(){};
    std::string                m_status;
    std::string                m_error;
    std::vector<RestoreStatus> m_status_list;
};

void operator<<=(cxxtools::SerializationInfo& si, const SrrRestoreResponse& resp);
void operator>>=(const cxxtools::SerializationInfo& si, SrrRestoreResponse& resp);

} // namespace srr
