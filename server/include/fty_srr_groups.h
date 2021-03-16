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

namespace srr
{
std::string getGroupFromFeature(const std::string& featureName);
unsigned int getPriority(const std::string& featureName);

typedef struct SrrFeatureStruct
{
    std::string m_id;
    std::string m_name;
    std::string m_description;

    std::string m_agent;

    std::list<std::string> m_requiredIn;

    bool m_restart;
    bool m_reset;
} SrrFeatureStruct;

typedef struct SrrFeaturePriorityStruct
{
    SrrFeaturePriorityStruct(const std::string& f, unsigned int p) : m_feature(f), m_priority(p) {};
    std::string m_feature;
    unsigned int m_priority;
} SrrFeaturePriorityStruct;

typedef struct SrrGroupStruct
{
    std::string m_id;
    std::string m_name;
    std::string m_description;
    unsigned    m_restoreOrder; // define restore order (lower is restored before)

    std::vector<SrrFeaturePriorityStruct> m_fp;
} SrrGroupStruct;

extern const std::map<std::string, SrrFeatureStruct> g_srrFeatureMap;

extern const std::map<std::string, SrrGroupStruct> g_srrGroupMap;

extern const std::map<const std::string, const std::string> g_agentToQueue;

}
