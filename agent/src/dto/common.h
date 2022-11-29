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

#include <cxxtools/serializationinfo.h>
#include <fty_common_dto.h>
#include <string>
#include <vector>

namespace srr {

// si common fields
static constexpr const char* SI_VERSION     = "version";
static constexpr const char* SI_CHECKSUM    = "checksum";
static constexpr const char* SI_STATUS      = "status";
static constexpr const char* SI_ERROR       = "error";
static constexpr const char* SI_DATA        = "data";
static constexpr const char* SI_NAME        = "name";
static constexpr const char* SI_DESCRIPTION = "description";
static constexpr const char* SI_GROUPS      = "groups";
static constexpr const char* SI_FEATURES    = "features";
static constexpr const char* SI_PASSPHRASE  = "passphrase";

// si group fields
static constexpr const char* SI_GROUP_ID       = "group_id";
static constexpr const char* SI_GROUP_NAME     = "group_name";
static constexpr const char* SI_DATA_INTEGRITY = "data_integrity";

void operator<<=(cxxtools::SerializationInfo& si, const dto::srr::FeatureAndStatus& fs);
void operator>>=(const cxxtools::SerializationInfo& si, dto::srr::FeatureAndStatus& fs);

class SrrFeature
{
public:
    SrrFeature(){};

    std::string                m_feature_name;
    dto::srr::FeatureAndStatus m_feature_and_status;
};

void operator<<=(cxxtools::SerializationInfo& si, const SrrFeature& f);
void operator>>=(const cxxtools::SerializationInfo& si, SrrFeature& f);

class FeatureInfo
{
public:
    FeatureInfo(){};

    std::string m_name;
    std::string m_description;
};

void operator<<=(cxxtools::SerializationInfo& si, const FeatureInfo& resp);
void operator>>=(const cxxtools::SerializationInfo& si, FeatureInfo& resp);

class Group
{
public:
    Group(){};

    std::string             m_group_id;
    std::string             m_group_name;
    std::string             m_data_integrity;
    std::vector<SrrFeature> m_features;
};

void operator<<=(cxxtools::SerializationInfo& si, const Group& resp);
void operator>>=(const cxxtools::SerializationInfo& si, Group& resp);

class GroupInfo
{
public:
    GroupInfo(){};

    std::string m_group_id;
    std::string m_group_name;
    std::string m_description;
    std::vector<FeatureInfo> m_features;
};

void operator<<=(cxxtools::SerializationInfo& si, const GroupInfo& resp);
void operator>>=(const cxxtools::SerializationInfo& si, GroupInfo& resp);

class RestoreStatus
{
public:
    RestoreStatus(){};

    std::string m_name;
    std::string m_status;
    std::string m_error;
};

void operator<<=(cxxtools::SerializationInfo& si, const RestoreStatus& resp);
void operator>>=(const cxxtools::SerializationInfo& si, RestoreStatus& resp);

} // namespace srr
