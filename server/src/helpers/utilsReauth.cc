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

#include "helpers/utilsReauth.h"

#include <cxxtools/base64codec.h>
#include <iostream>


namespace srr::utils
{
  bool isPasswordValidated(const std::string& passwd)
  {
    auto checkPasswd{"sudo -k; echo '" + passwd + "' | sudo -S test true 1>/dev/null 2>/dev/null"};

    int ret = std::system(checkPasswd.c_str());
    bool passwdOk = false;

    if (WEXITSTATUS(ret) == 0)
    {
      passwdOk = true;
    }
    return passwdOk;
  }

  std::string buildReauthToken(const std::string& sessionToken, const std::string& passwd)
  {
    return cxxtools::Base64Codec::encode(sessionToken + ":" + passwd);
  }
}
