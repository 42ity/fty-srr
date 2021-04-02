cmake_minimum_required(VERSION 3.13)
cmake_policy(VERSION 3.13)

##############################################################################################################

etn_target(exe ${PROJECT_NAME}
    SOURCES
        src/fty-srr.cc
        src/fty_srr_groups.cc
        src/fty_srr_manager.cc
        src/fty_srr_worker.cc
        src/dto/common.cc
        src/dto/request.cc
        src/dto/response.cc
        src/helpers/data_integrity.cc
        src/helpers/utils.cc

    INCLUDE_DIRS
        include

    USES_PRIVATE
        cxxtools
        fty_common
        fty_common_dto
        fty_common_logging
        fty_common_messagebus
        fty_common_mlm
        fty_lib_certificate
        openssl
        protobuf
        pthread
)

##############################################################################################################

etn_target(exe ${PROJECT_NAME}-cmd
    SOURCES
        src/fty-srr-cmd.cc
        src/dto/common.cc
        src/dto/request.cc
        src/dto/response.cc
        src/helpers/utilsReauth.cc
    INCLUDE_DIRS
        include
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

##############################################################################################################

#install files

configure_file(fty-srr.service.in
  "${PROJECT_BINARY_DIR}/fty-srr.service" @ONLY)

install(FILES
  "fty-srr.cfg"
  DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/fty-srr/
)

install(FILES
  "fty-srr-reboot.sh"
  PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_READ
  DESTINATION /usr/${CMAKE_INSTALL_SBINDIR}/
)

install(FILES
  "sudoers/fty_20_srr"
  PERMISSIONS OWNER_READ GROUP_READ
  DESTINATION /etc/sudoers.d/
)

install(FILES
  "${PROJECT_BINARY_DIR}/fty-srr.service"
  DESTINATION "${CMAKE_INSTALL_SYSCONFDIR}/systemd/system/"
)

##############################################################################################################
