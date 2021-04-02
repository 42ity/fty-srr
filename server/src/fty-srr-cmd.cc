/*  =========================================================================
    fty-srr-cmd - Binary

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

/*
@header
    fty-srr-cmd - Binary
@discuss
@end
*/

#include "dto/request.h"
#include "dto/response.h"
#include "helpers/utilsReauth.h"

#include <cxxtools/serializationinfo.h>
#include <fty/command-line.h>
#include <fty_common.h>
#include <fty_common_dto.h>
#include <fty_common_messagebus.h>
#include <fty_log.h>
#include <fty/split.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include <cstdio>

#define END_POINT "ipc://@/malamute"
#define AGENT_NAME "fty-srr-cmd"
#define AGENT_NAME_REQUEST_DESTINATION "fty-srr-ui"
#define MSG_QUEUE_NAME "ETN.Q.IPMCORE.SRR.UI"
#define DEFAULT_TIME_OUT 3600
#define SESSION_TOKEN_ENV_VAR "USM_BEARER"


using namespace dto::srr;

template <typename T>
std::ostream &operator<< (std::ostream &os, const std::vector<T> &vec)
{
    auto it = vec.begin();
    for(; it != vec.end() -1; it++) {
        os << *it << ", ";
    }
    os << *it;
    return os;
}
// Utils
dto::UserData sendRequest (const std::string &action,
                           const dto::UserData &userData);

// operations
std::vector<std::string> opList (void);
void opSave (const std::string &passphrase,
             const std::string &sessionToken,
             const std::vector<std::string> &groupList,
             std::ostream &os);
void opRestore (const std::string &passphrase,
                const std::string &sessionToken,
                std::istream &is,
                bool force);
void opReset (void);

int main (int argc, char **argv)
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    //remove the zmq logs
    std::shared_ptr<FILE> stdNull(fopen("/dev/null", "w"), &fclose);
    zsys_set_logstream (stdNull.get());

    //remove log from fty-log
    ftylog_setLogLevelError(ftylog_getInstance());

    bool help = false;
    bool force = false;

    std::string fileName;
    std::string groups;
    std::string passphrase;
    std::string passwd{};
    std::string sessionToken{};

    if(std::getenv(SESSION_TOKEN_ENV_VAR))
    {
      sessionToken = std::getenv(SESSION_TOKEN_ENV_VAR);
    }

    // clang-format off
    fty::CommandLine cmd("### - SRR command line\n      Usage: fty-srr-cmd <list|save|restore|reset> [options]", {
        {"--help|-h", help, "Show this help"},
        {"--passphrase|-p", passphrase, "Passhphrase to save/restore groups"},
        {"--password|-pwd", passwd, "Password to restore groups (reauthentication)"},
        {"--token|-t", sessionToken, "Session token to save/restore groups if needed"},
        {"--groups|-g", groups, "Select groups to save (default to all groups)"},
        {"--file|-f", fileName, "Path to the JSON file to save/restore. If not specified, standard input/output is used"},
        {"--force|-F", force, "Force restore (discards data integrity check)"}
    });

    if(argc < 2) {
        std::cout << "### - No operation selected" << std::endl;
        std::cout << cmd.help() << std::endl;
        return EXIT_FAILURE;
    }

    std::string operation(argv[1]);

    if (auto res = cmd.parse(argc, argv); !res) {
        std::cerr << res.error() << std::endl;
        std::cout << std::endl;
        std::cout << cmd.help() << std::endl;
        return EXIT_FAILURE;
    }

    if(help) {
        std::cout << cmd.help() << std::endl;
        return EXIT_SUCCESS;
    }

    if(operation == "list") {
        opList();
    } else if(operation == "save") {
        if(passphrase.empty()) {
            std::cerr << "### - Passphrase is required with save operation" << std::endl;
            std::cout << cmd.help() << std::endl;
            return EXIT_FAILURE;
        }
        std::ofstream outputFile;
        if(!fileName.empty()) {
            try{
                outputFile.open(fileName);
            } catch(const std::exception& e) {
                std::cerr << "### - Can't open output file: " << e.what() << std::endl;
                return EXIT_FAILURE;
            }
        }
        std::vector<std::string> groupList;
        if(!groups.empty()) {
            groupList = fty::split(groups, ",", fty::SplitOption::Trim);
            std::cout << "### - Saving groups: " << groupList << std::endl;
        } else {
            std::cout << "### - No group option specified\nSaving all groups" << std::endl;
            groupList = opList();
        }
        opSave(passphrase, sessionToken, groupList, outputFile.is_open() ? outputFile : std::cout);
        if(outputFile.is_open()) {
            outputFile.close();
        }
    } else if(operation == "restore") {
        if(passphrase.empty()) {
            std::cerr << "### - Passphrase is required with restore operation" << std::endl;
            std::cout << cmd.help() << std::endl;
            return EXIT_FAILURE;
        }
        if(passwd.empty()) {
            std::cerr << "### - Password for reauthentication is required with restore operation" << std::endl;
            std::cout << cmd.help() << std::endl;
            return EXIT_FAILURE;
        } else if(!srr::utils::isPasswordValidated(passwd)) {
            std::cerr << "### - Wrong password, please retry" << std::endl;
            return EXIT_FAILURE;
        }
        std::ifstream inputFile;
        if(!fileName.empty()) {
            try{
                inputFile.open(fileName);
            } catch(const std::exception& e) {
                std::cerr << "### - Can't open input file: " << e.what() << std::endl;
                return EXIT_FAILURE;
            }
        } else {
            std::cout << "### - No input file specified, waiting for input from stdin" << std::endl;
        }
        std::string reauthToken = srr::utils::buildReauthToken(sessionToken, passwd);
        opRestore(passphrase, reauthToken, inputFile.is_open() ? inputFile : std::cin, force);
        if(inputFile.is_open()) {
            inputFile.close();
        }
    } else if(operation == "reset") {
        opReset();
    } else {
        std::cout << "### - Unknown operation" << std::endl;
        std::cout << std::endl;
        std::cout << cmd.help() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

dto::UserData sendRequest (const std::string &action,
                           const dto::UserData &userData)
{
    log_debug ("sendRequest <%s> action", action.c_str());
    // Client id
    std::string clientId = messagebus::getClientId (AGENT_NAME);
    std::unique_ptr<messagebus::MessageBus> requester (
      messagebus::MlmMessageBus (END_POINT, clientId));
    requester->connect ();

    // Build message
    messagebus::Message msg;
    msg.userData () = userData;
    msg.metaData ().emplace (messagebus::Message::SUBJECT, action);
    msg.metaData ().emplace (messagebus::Message::FROM, clientId);
    msg.metaData ().emplace (messagebus::Message::TO,
                             AGENT_NAME_REQUEST_DESTINATION);
    msg.metaData ().emplace (messagebus::Message::CORRELATION_ID,
                             messagebus::generateUuid ());
    // Send request
    messagebus::Message resp =
      requester->request (MSG_QUEUE_NAME, msg, DEFAULT_TIME_OUT);
    // Return the data response
    return resp.userData ();
}

std::vector<std::string> opList() {
    std::vector<std::string> groupList;

    try {
        dto::UserData reqData;

        // Send request
        dto::UserData respData = sendRequest ("list", reqData);
        if (respData.empty ()) {
            throw std::runtime_error (
              "Impossible to get the list of features");
        }

        srr::SrrListResponse resp;

        cxxtools::SerializationInfo si;
        JSON::readFromString(respData.front(), si);

        si >>= resp;

        std::cout << "### Groups available:" << std::endl;
        for(auto g : resp.m_groups) {
            std::cout << " - " << g.m_group_id << std::endl;
            groupList.push_back(g.m_group_id);
        }
    }
    catch (std::exception &e) {
        std::cerr << "### - Error: " << e.what () << std::endl;
    }

    return groupList;
}

void opSave(const std::string& passphrase, const std::string& sessionToken, const std::vector<std::string>& groupList, std::ostream& os) {
    srr::SrrSaveRequest req;
    req.m_group_list = groupList;
    req.m_passphrase = passphrase;
    req.m_sessionToken = sessionToken;

    cxxtools::SerializationInfo reqSi;

    reqSi <<= req;

    try {
        dto::UserData reqData;
        reqData.push_back(JSON::writeToString(reqSi, false));

        // Send request
        dto::UserData respData = sendRequest ("save", reqData);
        if (respData.empty ()) {
            throw std::runtime_error (
              "Impossible to save requested features");
        }

        srr::SrrSaveResponse resp;

        cxxtools::SerializationInfo respSi;
        JSON::readFromString(respData.back(), respSi);

        respSi >>= resp;

        std::cout << "Request status: " << resp.m_status << std::endl;

        if(!resp.m_error.empty()) {
            std::cerr << "Error: " << resp.m_error << std::endl;
        }

        os << respData.back() << std::endl;
    }
    catch (std::exception &e) {
        std::cerr << "### - Error: " << e.what () << std::endl;
    }
}

void opRestore(const std::string& passphrase, const std::string& sessionToken, std::istream& is, bool force) {
    std::string reqJson;
    while(!is.eof()) {
        reqJson += static_cast<char>(is.get());
    }

    cxxtools::SerializationInfo siJson;
    try{
        JSON::readFromString(reqJson, siJson);
    } catch(const std::exception& e) {
        std::cerr << "### - Error: " << e.what () << std::endl;
        return;
    }

    srr::SrrRestoreRequest req;
    req.m_passphrase = passphrase;
    req.m_sessionToken = sessionToken;
    siJson.getMember("version") >>= req.m_version;
    siJson.getMember("checksum") >>= req.m_checksum;

    if(req.m_version == "1.0") {
        srr::SrrRestoreRequestDataV1 reqData;
        siJson.getMember("data") >>= reqData.m_data;
        req.m_data_ptr = std::shared_ptr<srr::SrrRestoreRequestData>(new srr::SrrRestoreRequestDataV1(reqData));
    } else if(req.m_version == "2.0" || req.m_version == "2.1") {
        srr::SrrRestoreRequestDataV2 reqData;
        siJson.getMember("data") >>= reqData.m_data;
        req.m_data_ptr = std::shared_ptr<srr::SrrRestoreRequestData>(new srr::SrrRestoreRequestDataV2(reqData));
    } else {
        std::cerr << "### - Invalid SRR version" << std::endl;
        return;
    }

    cxxtools::SerializationInfo reqSi;
    reqSi <<= req;

    try {
        dto::UserData reqData;
        reqData.push_back(JSON::writeToString(reqSi, false));

        if(force) {
            std::cout << "### - Restoring with force option" << std::endl;
            reqData.push_back("force");
        }

        // Send request
        dto::UserData respData = sendRequest ("restore", reqData);
        if (respData.empty ()) {
            throw std::runtime_error (
              "Impossible to restore requested features");
        }

        srr::SrrRestoreResponse resp;

        cxxtools::SerializationInfo respSi;
        JSON::readFromString(respData.back(), respSi);

        respSi >>= resp;

        std::cout << "Request status: " << resp.m_status << std::endl;

        if(!resp.m_error.empty()) {
            std::cerr << "### - Error: " << resp.m_error << std::endl;
        }
    }
    catch (std::exception &e) {
        std::cerr << "### - Error: " << e.what () << std::endl;
    }
}

void opReset() {
    std::cerr << "Srr daemon does not handle reset operation" << std::endl;
}
