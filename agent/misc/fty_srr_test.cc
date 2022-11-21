/*  =========================================================================
    fty_srr_test - Binary

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
    fty_srr_test - Binary
@discuss
@end
 */

#include "fty_srr_classes.h"


#include "fty_common_mlm_zconfig.h"

#include <cxxtools/jsonserializer.h>
#include <vector>
#include <string>

#include <fty_common.h>
#include <fty_common_json.h>
#include <fty_common_rest_helpers.h>
#include <fty_srr_dto.h>

static std::string getSaveIpm2ConfigPayload()
{
    cxxtools::SerializationInfo si;
    si.addMember("version") <<= "1.0";
    cxxtools::SerializationInfo& siFeaturesList = si.addMember("featureList");
    siFeaturesList.setCategory(cxxtools::SerializationInfo::Category::Array);

    cxxtools::SerializationInfo siTemp;
    siTemp.addMember("name") <<= "automation";
    siFeaturesList.addMember("") <<= siTemp;

    return JSON::writeToString(si, false);
}

static std::string getRestoreIpm2ConfigPayload()
{
    cxxtools::SerializationInfo si;
    si.addMember("version") <<= "1.0";

    // Data
    cxxtools::SerializationInfo& siData = si.addMember("data");
    cxxtools::SerializationInfo& siAutomation = siData.addMember("automation");
    siAutomation.addMember("version") <<= "1.0";
    cxxtools::SerializationInfo& siAutomationData = siAutomation.addMember("data");

    // Server 1
    cxxtools::SerializationInfo& siServer1 = siAutomationData.addMember("server");
    siServer1.addMember("timeout") <<= "9";
    // Server 2
    cxxtools::SerializationInfo& siServer2 = siAutomationData.addMember("server-2");
    siServer2.addMember("timeout") <<= "8";

    return JSON::writeToString(si, false);
}

dto::UserData sendRequest(const std::string& subject, const dto::UserData& userData)
{
    dto::UserData response;
    try
    {
        // Client id
        std::string clientId = messagebus::getClientId(AGENT_NAME);

        std::unique_ptr<messagebus::MessageBus> requester(messagebus::MlmMessageBus(DEFAULT_ENDPOINT /*END_POINT*/, clientId));
        requester->connect();

        // Build message
        messagebus::Message msg;
        msg.userData() = userData;
        msg.metaData().emplace(messagebus::Message::SUBJECT, subject);
        msg.metaData().emplace(messagebus::Message::FROM, clientId);
        msg.metaData().emplace(messagebus::Message::TO, "fty-srr" /*AGENT_NAME_REQUEST_DESTINATION*/);
        msg.metaData().emplace(messagebus::Message::CORRELATION_ID, messagebus::generateUuid());
        // Send request
        messagebus::Message resp = requester->request(SRR_MSG_QUEUE_NAME, msg, 5/*DEFAULT_TIME_OUT*/);
        response = resp.userData();
    }
    catch (messagebus::MessageBusException& ex)
    {
        log_error("Message bus exception %s", ex.what());
    }
    catch (...)
    {
        log_error("Unknown exception to get feature list managed");
    }
    return response;
}

static dto::srr::SrrFeaturesListDto getFeatureListManaged()
{
    dto::srr::SrrFeaturesListDto featuresListDto;
    dto::srr::SrrQueryDto query(GET_ACTION);
    dto::UserData reqData;
    reqData << query;
    // Send request
    dto::UserData respData = sendRequest(GET_ACTION, reqData);
    respData >> featuresListDto;

    return featuresListDto;
}

static std::string saveIpm2Configuration(const std::string& inputData)
{
    dto::srr::SrrQueryDto query(SAVE_ACTION, inputData);
    dto::UserData reqData;
    reqData << query;
    dto::UserData respData = sendRequest(SAVE_ACTION, reqData);

    return respData.front();
}

static dto::srr::SrrRestoreDtoList restoreIpm2Configuration(const std::string& inputData)
{
    dto::srr::SrrRestoreDtoList responseDto;

    dto::srr::SrrQueryDto query(RESTORE_ACTION, inputData);
    dto::UserData reqData;
    reqData << query;
    dto::UserData respData = sendRequest(RESTORE_ACTION, reqData);
    respData >> responseDto;

    return responseDto;
}

void usage()
{
    puts(AGENT_NAME " [options] ...");
    puts("  -v|--verbose        verbose test output");
    puts("  -h|--help           this information");
    puts("  -g|--get            get feature list");
    puts("  -s|--save           Save");
    puts("  -r|--restore        Restore");
}

void srrTest(const std::string& action)
{
    log_info(AGENT_NAME " test starting");

    if (action.compare("get") == 0)
    {
        dto::srr::SrrFeaturesListDto featuresListDto = getFeatureListManaged();
        // Build the json result
        for (const auto& feature : featuresListDto.featuresList)
        {
            log_debug("  ** feature: '%s'", feature.c_str());
        }
    }
    else if (action.compare("save") == 0)
    {
        const std::string inputPayload = getSaveIpm2ConfigPayload();
        std::vector<dto::srr::SrrSaveDto> responseDto;
        std::string ipm2Config = saveIpm2Configuration(inputPayload);
        log_debug("Ipm2 configuration: '%s'", ipm2Config.c_str());

        cxxtools::SerializationInfo jsonResp;
        JSON::readFromString (ipm2Config, jsonResp);

        log_debug("Ipm2 configuration: '%s'", ipm2Config.c_str());
    }
    else if (action.compare("restore") == 0)
    {
        const std::string inputData = getRestoreIpm2ConfigPayload();
        dto::srr::SrrRestoreDtoList responseDto = restoreIpm2Configuration(inputData);

        // Output serialization
        cxxtools::SerializationInfo siResp;
        siResp.addMember("status") <<= responseDto.status;
        cxxtools::SerializationInfo siStatusList;
        siStatusList.setCategory(cxxtools::SerializationInfo::Category::Array);

        for (const auto& resp : responseDto.responseList)
        {
            cxxtools::SerializationInfo siTemp;

            siTemp.addMember("name") <<= resp.name;
            siTemp.addMember("status") <<= resp.status;
            siTemp.addMember("error") <<= resp.error;
            siStatusList.addMember("") <<= siTemp;
        }

        siResp.addMember("") <<= siStatusList;

        std::string returnString = JSON::writeToString (siResp, false);
        log_debug("Restore response: %s", returnString.c_str());
    }
}

int main(int argc, char *argv [])
{
    using Parameters = std::map<std::string, std::string>;
    Parameters paramsConfig;

    try
    {
        ftylog_setInstance(AGENT_NAME, FTY_COMMON_LOGGING_DEFAULT_CFG);

        int argn;
        char *config_file = NULL;
        bool verbose = false;
        std::string action = "";
        // Parse command line
        for (argn = 1; argn < argc; argn++)
        {
            char *param = NULL;
            if (argn < argc - 1) param = argv [argn + 1];

            if (streq(argv [argn], "--help") || streq(argv [argn], "-h"))
            {
                usage();
                return EXIT_SUCCESS;
            }
            else if (streq(argv [argn], "--verbose") || streq(argv [argn], "-v"))
            {
                verbose = true;
            }
            else if (streq(argv [argn], "--config") || streq(argv [argn], "-c"))
            {
                if (param)
                {
                    config_file = param;
                }
                ++argn;
            }
            else if (streq(argv [argn], "--get") || streq(argv [argn], "-g"))
            {
                action = "get";
            }
            else if (streq(argv [argn], "--save") || streq(argv [argn], "-s"))
            {
                action = "save";
            }
            else if (streq(argv [argn], "--restore") || streq(argv [argn], "-r"))
            {
                action = "restore";
            }
        }

        if (verbose)
        {
            ftylog_setVerboseMode(ftylog_getInstance());
            log_trace("Verbose mode OK");
        }

        log_info(AGENT_NAME " starting");

        srrTest(action);

        return EXIT_SUCCESS;
    }
    catch (const std::exception & e)
    {
        log_error(AGENT_NAME ": Error '%s'", e.what());
    }
    catch (...)
    {
        log_error(AGENT_NAME ": Unknown error");
    }

    return EXIT_FAILURE;
}
