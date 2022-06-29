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

#pragma once

#include "common.h"
#include <cxxtools/serializationinfo.h>
#include <memory>
#include <string>
#include <vector>

//see SRR_VERSION_ALL SRR_ACTIVE_VERSION
//std::string version, X.Y format (major/minor)
#define IS_VERSION_1(version) (version.find("1.") == 0)
#define IS_VERSION_2(version) (version.find("2.") == 0)

namespace srr {
// si save request fields
static constexpr const char* SI_GROUP_LIST = "group_list";

class SrrSaveRequest
{
public:
    SrrSaveRequest() = default;

    std::string              m_passphrase;
    std::string              m_sessionToken;
    std::vector<std::string> m_group_list;
};

void operator<<=(cxxtools::SerializationInfo& si, const SrrSaveRequest& req);
void operator>>=(const cxxtools::SerializationInfo& si, SrrSaveRequest& req);

class SrrRestoreRequestData
{
public:
    virtual ~SrrRestoreRequestData()                             = 0;
    virtual const std::vector<SrrFeature> getSrrFeatures() const = 0;
};

class SrrRestoreRequestDataV1 : public SrrRestoreRequestData
{
public:
    ~SrrRestoreRequestDataV1(){};
    std::vector<SrrFeature>       m_data;
    const std::vector<SrrFeature> getSrrFeatures() const override;
};

class SrrRestoreRequestDataV2 : public SrrRestoreRequestData
{
public:
    ~SrrRestoreRequestDataV2(){};
    std::vector<Group>            m_data;
    const std::vector<SrrFeature> getSrrFeatures() const override;
};

using SrrRestoreRequestDataPtr = std::shared_ptr<SrrRestoreRequestData>;

class SrrRestoreRequest
{
public:
    SrrRestoreRequest() = default;
    std::string              m_version;
    std::string              m_passphrase;
    std::string              m_sessionToken;
    std::string              m_checksum;
    SrrRestoreRequestDataPtr m_data_ptr;
};

void operator<<=(cxxtools::SerializationInfo& si, const SrrRestoreRequest& req);
void operator>>=(const cxxtools::SerializationInfo& si, SrrRestoreRequest& req);

} // namespace srr
