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

#include "helpers/data_integrity.h"
#include "fty_srr_exception.h"
#include "fty_srr_groups.h"
#include <cxxtools/serializationinfo.h>
#include <dto/common.h>
#include <fty_common.h>
#include <iomanip>
#include <openssl/sha.h>

namespace srr {

std::string evalSha256(const std::string& data)
{
    unsigned char result[SHA256_DIGEST_LENGTH];
    SHA256(const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(data.c_str())), data.length(), result);

    std::ostringstream sout;
    sout << std::hex << std::setfill('0');
    for (long long c : result) {
        sout << std::setw(2) << c;
    }

    return sout.str();
}

void evalDataIntegrity(Group& group)
{
    // sort features by priority
    std::sort(group.m_features.begin(), group.m_features.end(), [&](SrrFeature l, SrrFeature r) {
        return getPriority(l.m_feature_name) < getPriority(r.m_feature_name);
    });

    // evaluate data integrity
    cxxtools::SerializationInfo tmpSi;
    tmpSi <<= group.m_features;
    const std::string data = dto::srr::serializeJson(tmpSi, false);

    group.m_data_integrity = evalSha256(data);
}

bool checkDataIntegrity(const Group& group)
{
    cxxtools::SerializationInfo tmpSi;
    tmpSi <<= group.m_features;
    const std::string data = dto::srr::serializeJson(tmpSi, false);

    std::string checksum = evalSha256(data);

    return checksum == group.m_data_integrity;
}

} // namespace srr
