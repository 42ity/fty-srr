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

#ifndef FTY_SRR_EXCEPTION_H_INCLUDED
#define FTY_SRR_EXCEPTION_H_INCLUDED

#include <stdexcept>
#include <string>

namespace srr {

    /**
     * Srr exception Class
     * @param 
     * @return 
     */
    class SrrException : public std::runtime_error {
      public:
        SrrException(const std::string& what) : std::runtime_error(what) {}
        SrrException(const char* what) : std::runtime_error(what) {}
        virtual ~SrrException() = default;
    };

    struct SrrInvalidVersion : public std::exception
    {
        SrrInvalidVersion() {};
        SrrInvalidVersion(const std::string& err) : m_err(err) {};

        std::string m_err = "Invalid SRR version";

        const char * what () const throw ()
        {
            return m_err.c_str();
        }
    };

    struct SrrIntegrityCheckFailed : public std::exception
    {
        SrrIntegrityCheckFailed() {};
        SrrIntegrityCheckFailed(const std::string& err) : m_err(err) {};

        std::string m_err = "Integrity Check Failed";

        const char * what () const throw ()
        {
            return m_err.c_str();
        }
    };

    struct SrrSaveFailed : public std::exception
    {
        SrrSaveFailed() {};
        SrrSaveFailed(const std::string& err) : m_err(err) {};

        std::string m_err = "Save failed";

        const char * what () const throw ()
        {
            return m_err.c_str();
        }
    };

    struct SrrRestoreFailed : public std::exception
    {
        SrrRestoreFailed() {};
        SrrRestoreFailed(const std::string& err) : m_err(err) {};

        std::string m_err = "Restore failed";

        const char * what () const throw ()
        {
            return m_err.c_str();
        }
    };

    struct SrrResetFailed : public std::exception
    {
        SrrResetFailed() {};
        SrrResetFailed(const std::string& err) : m_err(err) {};

        std::string m_err = "Reset failed";

        const char * what () const throw ()
        {
            return m_err.c_str();
        }
    };
}

#endif
