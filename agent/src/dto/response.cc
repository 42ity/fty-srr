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

#include "dto/response.h"

namespace srr {
void operator<<=(cxxtools::SerializationInfo& si, const SrrListResponse& resp)
{
    si.addMember(SI_VERSION) <<= resp.m_version;
    si.addMember(SI_PASSPHRASE_DESCRIPTION) <<= resp.m_passphrase_description;
    si.addMember(SI_PASSPHRASE_VALIDATION) <<= resp.m_passphrase_validation;
    si.addMember(SI_GROUPS) <<= resp.m_groups;
}

void operator>>=(const cxxtools::SerializationInfo& si, SrrListResponse& resp)
{
    si.getMember(SI_VERSION) >>= resp.m_version;
    si.getMember(SI_PASSPHRASE_DESCRIPTION) >>= resp.m_passphrase_description;
    si.getMember(SI_PASSPHRASE_VALIDATION) >>= resp.m_passphrase_validation;
    si.getMember(SI_GROUPS) >>= resp.m_groups;
}

void operator<<=(cxxtools::SerializationInfo& si, const SrrSaveResponse& resp)
{
    si.addMember(SI_VERSION) <<= resp.m_version;
    si.addMember(SI_STATUS) <<= resp.m_status;
    if (resp.m_status != dto::srr::statusToString(dto::srr::Status::SUCCESS)) {
        si.addMember(SI_ERROR) <<= resp.m_error;
    }
    si.addMember(SI_CHECKSUM) <<= resp.m_checksum;
    si.addMember(SI_DATA) <<= resp.m_data;
}

void operator>>=(const cxxtools::SerializationInfo& si, SrrSaveResponse& resp)
{
    si.getMember(SI_VERSION) >>= resp.m_version;
    si.getMember(SI_STATUS) >>= resp.m_status;
    if (si.findMember(SI_ERROR) != nullptr) {
        si.getMember(SI_ERROR) >>= resp.m_error;
    }
    si.getMember(SI_CHECKSUM) >>= resp.m_checksum;
    si.getMember(SI_DATA) >>= resp.m_data;
}

void operator<<=(cxxtools::SerializationInfo& si, const SrrRestoreResponse& resp)
{
    si.addMember(SI_STATUS) <<= resp.m_status;
    if (resp.m_status != dto::srr::statusToString(dto::srr::Status::SUCCESS)) {
        si.addMember(SI_ERROR) <<= resp.m_error;
    }
    si.addMember(SI_STATUS_LIST) <<= resp.m_status_list;
}

void operator>>=(const cxxtools::SerializationInfo& si, SrrRestoreResponse& resp)
{
    si.getMember(SI_STATUS) >>= resp.m_status;
    if (si.findMember(SI_ERROR) != nullptr) {
        si.getMember(SI_ERROR) >>= resp.m_error;
    }
    si.getMember(SI_STATUS_LIST) >>= resp.m_status_list;
}

} // namespace srr
