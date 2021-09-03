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

#include "dto/common.h"

namespace srr {
void operator<<=(cxxtools::SerializationInfo& si, const dto::srr::FeatureAndStatus& fs)
{
    si.addMember(SI_VERSION) <<= fs.feature().version();
    si.addMember(SI_STATUS) <<= dto::srr::statusToString(fs.status().status());
    si.addMember(SI_ERROR) <<= fs.status().error();

    dto::srr::Feature            feature = fs.feature();
    cxxtools::SerializationInfo& data    = si.addMember(SI_DATA);
    try {
        // try to unserialize the data if they are on Json format
        cxxtools::SerializationInfo dataSi = dto::srr::deserializeJson(feature.data());

        if (dataSi.category() == cxxtools::SerializationInfo::Category::Void ||
            dataSi.category() == cxxtools::SerializationInfo::Category::Value || dataSi.isNull()) {
            data <<= feature.data();
        } else {
            dataSi.setName(SI_DATA);
            data = dataSi;
            data.setName(SI_DATA);
            data.setCategory(cxxtools::SerializationInfo::Category::Object);
        }
    } catch (const std::exception& /* e */) {
        // put the data as a string if they are not in Json
        data <<= feature.data();
    }
}

void operator>>=(const cxxtools::SerializationInfo& si, dto::srr::FeatureAndStatus& fs)
{
    si.getMember(SI_VERSION) >>= *(fs.mutable_feature()->mutable_version());

    std::string tmpStr;

    si.getMember(SI_STATUS) >>= tmpStr;
    fs.mutable_status()->set_status(dto::srr::stringToStatus(tmpStr));

    si.getMember(SI_ERROR) >>= tmpStr;
    fs.mutable_status()->set_error(tmpStr);

    cxxtools::SerializationInfo dataSi = si.getMember(SI_DATA);

    std::string data;

    if (dataSi.category() == cxxtools::SerializationInfo::Category::Value) {
        dataSi >>= data;
    } else {
        dataSi.setName("");
        data = dto::srr::serializeJson(dataSi);
    }

    fs.mutable_feature()->set_data(data);
}

void operator<<=(cxxtools::SerializationInfo& si, const SrrFeature& f)
{
    si.addMember(f.m_feature_name) <<= f.m_feature_and_status;
}

void operator>>=(const cxxtools::SerializationInfo& si, SrrFeature& f)
{
    auto& tmpSi = si.getMember(0);

    f.m_feature_name = tmpSi.name();
    tmpSi >>= f.m_feature_and_status;
}

void operator<<=(cxxtools::SerializationInfo& si, const FeatureInfo& resp)
{
    si.addMember(SI_NAME) <<= resp.m_name;
    si.addMember(SI_DESCRIPTION) <<= resp.m_description;
}

void operator>>=(const cxxtools::SerializationInfo& si, FeatureInfo& resp)
{
    si.getMember(SI_NAME) >>= resp.m_name;
    si.getMember(SI_DESCRIPTION) >>= resp.m_description;
}

void operator<<=(cxxtools::SerializationInfo& si, const Group& resp)
{
    si.addMember(SI_GROUP_ID) <<= resp.m_group_id;
    si.addMember(SI_GROUP_NAME) <<= resp.m_group_name;
    si.addMember(SI_DATA_INTEGRITY) <<= resp.m_data_integrity;
    si.addMember(SI_FEATURES) <<= resp.m_features;
}

void operator>>=(const cxxtools::SerializationInfo& si, Group& resp)
{
    si.getMember(SI_GROUP_ID) >>= resp.m_group_id;
    si.getMember(SI_GROUP_NAME) >>= resp.m_group_name;
    si.getMember(SI_DATA_INTEGRITY) >>= resp.m_data_integrity;
    si.getMember(SI_FEATURES) >>= resp.m_features;
}

void operator<<=(cxxtools::SerializationInfo& si, const GroupInfo& resp)
{
    si.addMember(SI_GROUP_ID) <<= resp.m_group_id;
    si.addMember(SI_GROUP_NAME) <<= resp.m_group_name;
    si.addMember(SI_DESCRIPTION) <<= resp.m_description;
    si.addMember(SI_FEATURES) <<= resp.m_features;
}

void operator>>=(const cxxtools::SerializationInfo& si, GroupInfo& resp)
{
    si.getMember(SI_GROUP_ID) >>= resp.m_group_id;
    si.getMember(SI_GROUP_NAME) >>= resp.m_group_name;
    si.getMember(SI_DESCRIPTION) >>= resp.m_description;
    si.getMember(SI_FEATURES) >>= resp.m_features;
}

void operator<<=(cxxtools::SerializationInfo& si, const RestoreStatus& resp)
{
    si.addMember(SI_NAME) <<= resp.m_name;
    si.addMember(SI_STATUS) <<= resp.m_status;
    si.addMember(SI_ERROR) <<= resp.m_error;
}

void operator>>=(const cxxtools::SerializationInfo& si, RestoreStatus& resp)
{
    si.getMember(SI_NAME) >>= resp.m_name;
    si.getMember(SI_STATUS) >>= resp.m_status;
    si.getMember(SI_ERROR) >>= resp.m_error;
}

} // namespace srr
