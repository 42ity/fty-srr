/*  =========================================================================
    fty_srr - Binary

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

#include "fty-srr.h"
#include "fty_common_mlm.h"
#include "fty_srr_exception.h"
#include "fty_srr_manager.h"
#include "fty_srr_worker.h"

#include <csignal>
#include <mutex>
#include <sstream>

volatile bool           g_exit = false;
std::condition_variable g_cv;
std::mutex              g_cvMutex;

void usage()
{
    puts((AGENT_NAME + std::string(" [options] ...")).c_str());
    puts("  -v|--verbose        verbose test output");
    puts("  -h|--help           this information");
    puts("  -c|--config         path to configuration file");
}

void sigHandler(int)
{
    g_exit = true;
    g_cv.notify_one();
}

/**
 * Set Signal handler
 */
void setSignalHandler()
{
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = sigHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);
}

/**
 * Set Signal handler
 */
void terminateHandler()
{
    logError("{}: Error", AGENT_NAME);
    exit(EXIT_FAILURE);
}

/**
 * Main program
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char* argv[])
{
    // Set signal handler
    setSignalHandler();
    // Set terminate pg handler
    std::set_terminate(terminateHandler);

    ftylog_setInstance(AGENT_NAME, FTY_COMMON_LOGGING_DEFAULT_CFG);

    char* config_file = NULL;
    bool  verbose     = false;

    // Parse command line
    for (int argn = 1; argn < argc; argn++) {
        char* param = NULL;
        if (argn < argc - 1)
            param = argv[argn + 1];

        if (streq(argv[argn], "--help") || streq(argv[argn], "-h")) {
            usage();
            return EXIT_SUCCESS;
        }
        else if (streq(argv[argn], "--verbose") || streq(argv[argn], "-v")) {
            verbose = true;
        }
        else if (streq(argv[argn], "--config") || streq(argv[argn], "-c")) {
            if (!param) {
                logError("{}: {}: missing argument", AGENT_NAME, argv[argn]);
                return EXIT_FAILURE;
            }
            config_file = param;
            ++argn;
        }
    }

    // Default parameters
    std::map<std::string, std::string> paramsConfig;
    paramsConfig[AGENT_NAME_KEY]        = AGENT_NAME;
    paramsConfig[ENDPOINT_KEY]          = DEFAULT_ENDPOINT;
    paramsConfig[REQUEST_TIMEOUT_KEY]   = REQUEST_TIMEOUT_DEFAULT;
    paramsConfig[SRR_QUEUE_NAME_KEY]    = SRR_MSG_QUEUE_NAME;
    paramsConfig[SRR_VERSION_KEY]       = SRR_ACTIVE_VERSION;
    paramsConfig[SRR_ENABLE_REBOOT_KEY] = SRR_ENABLE_REBOOT_DEFAULT;

    if (config_file) {
        logDebug("{}: loading conf. file ({})", AGENT_NAME, config_file);

        mlm::ZConfig config(config_file);

        std::istringstream(config.getEntry("server/verbose", "0")) >> verbose;

        paramsConfig[REQUEST_TIMEOUT_KEY]   = config.getEntry("server/timeout", REQUEST_TIMEOUT_DEFAULT);
        paramsConfig[ENDPOINT_KEY]          = config.getEntry("srr-msg-bus/endpoint", DEFAULT_ENDPOINT);
        paramsConfig[AGENT_NAME_KEY]        = config.getEntry("srr-msg-bus/address", AGENT_NAME);
        paramsConfig[SRR_QUEUE_NAME_KEY]    = config.getEntry("srr-msg-bus/srrQueueName", SRR_MSG_QUEUE_NAME);
        paramsConfig[SRR_VERSION_KEY]       = config.getEntry("srr/version", SRR_ACTIVE_VERSION);
        paramsConfig[SRR_ENABLE_REBOOT_KEY] = config.getEntry("srr/enableReboot", SRR_ENABLE_REBOOT_DEFAULT);
    }

    if (verbose) {
        ftylog_setVerboseMode(ftylog_getInstance());
        logDebug("{}: Set verbose mode", AGENT_NAME);
    }

    logInfo("{}: started", AGENT_NAME);

    srr::SrrManager srrManager(paramsConfig);

    // wait until interrupt
    std::unique_lock<std::mutex> lock(g_cvMutex);
    g_cv.wait(lock, [] {
        return g_exit;
    });

    logInfo("{}: ended", AGENT_NAME);

    // Exit application
    return EXIT_SUCCESS;
}
