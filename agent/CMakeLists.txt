cmake_minimum_required(VERSION 3.13)
cmake_policy(VERSION 3.13)

##############################################################################################################

etn_target(exe ${PROJECT_NAME}
    SOURCES
        src/fty-srr.cc
        src/fty-srr.h
        src/fty_srr_groups.cc
        src/fty_srr_groups.h
        src/fty_srr_manager.cc
        src/fty_srr_manager.h
        src/fty_srr_worker.cc
        src/fty_srr_worker.h
        src/dto/common.cc
        src/dto/common.h
        src/dto/request.cc
        src/dto/request.h
        src/dto/response.cc
        src/dto/response.h
        src/helpers/data_integrity.cc
        src/helpers/data_integrity.h
        src/helpers/utils.cc
        src/helpers/utils.h
        src/helpers/passPhrase.h
        src/helpers/passPhrase.cpp
    INCLUDE_DIRS
        src
    USES_PRIVATE
        czmq
        cxxtools
        fty_common
        fty_common_dto
        fty_common_logging
        fty_common_messagebus
        fty_common_mlm
        fty_lib_certificate
        fty-pack
        openssl
        protobuf
        pthread
)

##############################################################################################################

etn_target(exe ${PROJECT_NAME}-cmd
    SOURCES
        src/fty-srr-cmd.cc
        src/dto/common.cc
        src/dto/common.h
        src/dto/request.cc
        src/dto/request.h
        src/dto/response.cc
        src/dto/response.h
        src/helpers/utilsReauth.cc
        src/helpers/utilsReauth.h
    INCLUDE_DIRS
        src
    USES_PRIVATE
        cxxtools
        fty_common
        fty_common_dto
        fty_common_logging
        fty_common_messagebus
        fty_common_mlm
        fty-utils
        protobuf
        czmq
)
