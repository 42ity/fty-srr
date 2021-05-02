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

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/fty-srr.service.in
  ${PROJECT_BINARY_DIR}/fty-srr.service @ONLY)

# etc/<project-name>
install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/fty-srr.cfg
  DESTINATION ${CMAKE_INSTALL_FULL_SYSCONFDIR}/fty-srr/
)

# usr/sbin
install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/fty-srr-reboot.sh
  PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_READ
  DESTINATION ${CMAKE_INSTALL_FULL_SBINDIR}/
)

# etc/sudoers.d
install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/sudoers/fty_20_srr
  PERMISSIONS OWNER_READ GROUP_READ
  DESTINATION ${CMAKE_INSTALL_FULL_SYSCONFDIR}/sudoers.d/
)

# lib/systemd/system
install(FILES
  ${PROJECT_BINARY_DIR}/fty-srr.service
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/systemd/system/
)

##############################################################################################################
