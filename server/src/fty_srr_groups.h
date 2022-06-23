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

#include "fty-srr.h"
#include <fty_common_macros.h>
#include <list>
#include <map>
#include <string>
#include <vector>
#include <sstream>

namespace srr {

std::string  getGroupFromFeature(const std::string& featureName);
unsigned int getPriority(const std::string& featureName);

struct SrrFeatureStruct
{
    std::string m_id;
    std::string m_name;
    std::string m_description;
    std::string m_agent;
    std::list<std::string> m_requiredIn;

    bool m_restart;
    bool m_reset;

    //dump dbg
    std::string str() const
    {
        std::ostringstream s;
        s << "id(" << m_id << ")"
          << ", name(" << m_name << ")"
          << ", description(" << m_description << ")"
          << ", agent(" << m_agent << ")";

        s << ", requiredIn(";
        std::string sep;
        for (auto& it : m_requiredIn)
            { s << sep << it; sep = ", "; }
        s << ")";

        return s.str();
    }
};

struct SrrFeaturePriorityStruct
{
    SrrFeaturePriorityStruct(const std::string& feature, unsigned int priority)
        : m_feature(feature)
        , m_priority(priority)
    {};

    std::string  m_feature; // SrrFeatureStruct.m_id
    unsigned int m_priority;

    //dump dbg
    std::string str() const
    {
        std::ostringstream s;
        s << "feature(" << m_feature << ")"
          << ", priority(" << m_priority << ")";

        return s.str();
    }
};

struct SrrGroupStruct
{
    std::string m_id;
    std::string m_name;
    std::string m_description;
    unsigned    m_restoreOrder; // define restore order (lower first)

    std::vector<SrrFeaturePriorityStruct> m_fp;

    //dump dbg
    std::string str() const
    {
        std::ostringstream s;
        s << "id(" << m_id << ")"
          << ", name(" << m_name << ")"
          << ", description(" << m_description << ")"
          << ", restoreOrder(" << m_restoreOrder << ")";

        s << ", fp(";
        std::string sep;
        for (auto& it : m_fp)
            { s << sep << it.str(); sep = ", "; }
        s << ")";

        return s.str();
    }
};

extern const std::map<std::string, SrrFeatureStruct> g_srrFeatureMap;

extern const std::map<std::string, SrrGroupStruct> g_srrGroupMap;

extern const std::map<const std::string, const std::string> g_agentToQueue;

} // namespace srr
