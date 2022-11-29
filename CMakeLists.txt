cmake_minimum_required(VERSION 3.13)
cmake_policy(VERSION 3.13)

project(fty-srr
    VERSION 1.0.0
)

##############################################################################################################

find_package(fty-cmake PATHS ${CMAKE_BINARY_DIR}/fty-cmake)

##############################################################################################################

add_subdirectory(agent)

##############################################################################################################

set(res "resources")

## .cfg -> etc/
set(file "fty-srr.cfg")
configure_file("${PROJECT_SOURCE_DIR}/${res}/${file}.in" "${PROJECT_BINARY_DIR}/${res}/${file}" @ONLY)
install(FILES "${PROJECT_BINARY_DIR}/${res}/${file}" DESTINATION "${CMAKE_INSTALL_FULL_SYSCONFDIR}/${PROJECT_NAME}")

## .sh -> usr/sbin
set(file "fty-srr-reboot.sh")
install(FILES "${PROJECT_SOURCE_DIR}/${res}/${file}"
  PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_READ
  DESTINATION "${CMAKE_INSTALL_FULL_SBINDIR}/"
)

## fty_20_srr -> etc/sudoers.d
set(file "fty_20_srr")
install(FILES
  "${PROJECT_SOURCE_DIR}/${res}/sudoers.d/${file}"
  PERMISSIONS OWNER_READ GROUP_READ
  DESTINATION "${CMAKE_INSTALL_FULL_SYSCONFDIR}/sudoers.d/"
)

## .service -> lib/systemd/system
set(file "fty-srr.service")
configure_file("${PROJECT_SOURCE_DIR}/${res}/${file}.in" "${PROJECT_BINARY_DIR}/${res}/${file}" @ONLY)
install(FILES "${PROJECT_BINARY_DIR}/${res}/fty-srr.service" DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/systemd/system/")

##############################################################################################################
