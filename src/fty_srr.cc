/*  =========================================================================
    fty_srr - Binary

    Copyright (C) 2014 - 2018 Eaton

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
    fty_srr - Binary
@discuss
@end
 */

#include <csignal>
#include <mutex>

#include "fty_srr_classes.h"

#include "fty_common_mlm_library.h"

//functions

void usage();
volatile bool g_exit = false;
std::condition_variable g_cv;
std::mutex g_cvMutex;

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
    log_error((AGENT_NAME + std::string(" Error")).c_str());
    exit(EXIT_FAILURE);
}

/**
 * Main program
 * @param argc
 * @param argv
 * @return 
 */
int main(int argc, char *argv [])
{
    using Parameters = std::map<std::string, std::string>;
    Parameters paramsConfig;
    
    // Set signal handler
    setSignalHandler();
    // Set terminate pg handler
    std::set_terminate (terminateHandler);

    ftylog_setInstance(AGENT_NAME, "");

    int argn;
    char *config_file = NULL;
    bool verbose = false;
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
    }

    // Default parameters
    paramsConfig[AGENT_NAME_KEY] = AGENT_NAME;
    paramsConfig[ENDPOINT_KEY] = DEFAULT_ENDPOINT;
    paramsConfig[SRR_QUEUE_NAME_KEY] = SRR_MSG_QUEUE_NAME;
    paramsConfig[SRR_VERSION_KEY] = ACTIVE_VERSION;

    if (config_file)
    {
        log_debug((AGENT_NAME + std::string(": loading configuration file from ") + config_file).c_str());
        mlm::ZConfig config(config_file);
        // verbose mode
        std::istringstream(config.getEntry("server/verbose", "0")) >> verbose;
        paramsConfig[ENDPOINT_KEY] = config.getEntry("srr-msg-bus/endpoint", DEFAULT_ENDPOINT);
        paramsConfig[AGENT_NAME_KEY] = config.getEntry("srr-msg-bus/address", AGENT_NAME);
        paramsConfig[SRR_QUEUE_NAME_KEY] = config.getEntry("srr-msg-bus/srrQueueName", SRR_MSG_QUEUE_NAME);
        paramsConfig[SRR_VERSION_KEY] = config.getEntry("srr/version", ACTIVE_VERSION);
    }

    if (verbose)
    {
        ftylog_setVeboseMode(ftylog_getInstance());
        log_trace("Verbose mode OK");
    }

    log_info((AGENT_NAME + std::string(" starting")).c_str());

    srr::SrrManager srrManager(paramsConfig);

    //wait until interrupt
    std::unique_lock<std::mutex> lock(g_cvMutex);
    g_cv.wait(lock, [] { return g_exit; });

    log_info((AGENT_NAME + std::string(" interrupted")).c_str());
    
    // Exit application
    return EXIT_SUCCESS;
}

void usage()
{
    puts((AGENT_NAME + std::string(" [options] ...")).c_str());
    puts("  -v|--verbose        verbose test output");
    puts("  -h|--help           this information");
    puts("  -c|--config         path to configuration file");
}
