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

#include "dto/request.h"

namespace srr
{
  using namespace dto::srr;

  SrrRestoreRequestData::~SrrRestoreRequestData ()
  {
  }

  ////////////////////////////////////////////////////////////////////////////////

  const std::vector<SrrFeature> SrrRestoreRequestDataV1::getSrrFeatures () const
  {
      return m_data;
  }

  const std::vector<SrrFeature> SrrRestoreRequestDataV2::getSrrFeatures () const
  {
      std::vector<SrrFeature> features;
      for (const auto &group : m_data) {
          std::copy (group.m_features.begin (), group.m_features.end (),
                    std::back_inserter (features));
      }

      return features;
  }

  ////////////////////////////////////////////////////////////////////////////////

  void operator<<= (cxxtools::SerializationInfo &si, const SrrSaveRequest &req)
  {
      si.addMember (SI_PASSPHRASE) <<= req.m_passphrase;
      si.addMember (SI_GROUP_LIST) <<= req.m_group_list;
      si.addMember (SESSION_TOKEN) <<= req.m_sessionToken;
  }

  void operator>>= (const cxxtools::SerializationInfo &si, SrrSaveRequest &req)
  {
      si.getMember (SI_PASSPHRASE) >>= req.m_passphrase;
      si.getMember (SI_GROUP_LIST) >>= req.m_group_list;
      si.getMember (SESSION_TOKEN) >>= req.m_sessionToken;
  }

  void operator<<= (cxxtools::SerializationInfo &si, const SrrRestoreRequest &req)
  {
      si.addMember (SI_VERSION) <<= req.m_version;
      si.addMember (SI_PASSPHRASE) <<= req.m_passphrase;
      si.addMember (SI_CHECKSUM) <<= req.m_checksum;
      si.addMember (SESSION_TOKEN) <<= req.m_sessionToken;

      if (req.m_version == "1.0") {
          auto dataPtr =
            std::dynamic_pointer_cast<SrrRestoreRequestDataV1> (req.m_data_ptr);
          if (dataPtr) {
              si.addMember (SI_DATA) <<= dataPtr->m_data;
          } else {
              throw std::runtime_error ("Invalid data pointer");
          }
      } else if (req.m_version == "2.0" || req.m_version == "2.1") {
          auto dataPtr =
            std::dynamic_pointer_cast<SrrRestoreRequestDataV2> (req.m_data_ptr);
          if (dataPtr) {
              si.addMember (SI_DATA) <<= dataPtr->m_data;
          } else {
              throw std::runtime_error ("Invalid data pointer");
          }
      } else {
          throw std::runtime_error ("Data version is not supported");
      }
  }

  void operator>>= (const cxxtools::SerializationInfo &si, SrrRestoreRequest &req)
  {
      si.getMember (SI_VERSION) >>= req.m_version;
      si.getMember (SI_PASSPHRASE) >>= req.m_passphrase;
      si.getMember (SI_CHECKSUM) >>= req.m_checksum;
      si.getMember (SESSION_TOKEN) >>= req.m_sessionToken;

      if (req.m_version == "1.0") {
          std::shared_ptr<SrrRestoreRequestData> dataPtr (
            new SrrRestoreRequestDataV1);

          si.getMember (SI_DATA) >>=
            std::dynamic_pointer_cast<SrrRestoreRequestDataV1> (dataPtr)->m_data;
          req.m_data_ptr = dataPtr;
      } else if (req.m_version == "2.0" || req.m_version == "2.1") {
          std::shared_ptr<SrrRestoreRequestData> dataPtr (
            new SrrRestoreRequestDataV2);

          si.getMember (SI_DATA) >>=
            std::dynamic_pointer_cast<SrrRestoreRequestDataV2> (dataPtr)->m_data;
          req.m_data_ptr = dataPtr;
      } else {
          throw std::runtime_error ("Data version is not supported");
      }
  }

}
