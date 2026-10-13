cmake_minimum_required(VERSION 3.10)
set(PROJECT_NAME "disc_type")

# Display what compiler is doing
set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON")

# Don't check for c++ compiler
project(disc_type LANGUAGES C)

# Look for include files and libraries outside of baseline /usr, but respecting install prefix first
# This is needed on FreeBSD where non-system packages are in /usr/local
include_directories(AFTER "${CMAKE_INSTALL_PREFIX}/include")
link_directories(AFTER "${CMAKE_INSTALL_PREFIX}/lib")

# Get fancy variables
include(GNUInstallDirs)

# Use pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(DVDREAD REQUIRED dvdread)
pkg_check_modules(BLURAY REQUIRED libbluray>=1.2.0)

add_executable(disc_type disc_type.c)
target_link_libraries(disc_type PRIVATE bluray)
target_link_libraries(disc_type PRIVATE dvdread)
INSTALL(FILES disc_type PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE DESTINATION bin)

# cmake doesn't have a native uninstall option
add_custom_target("uninstall")
add_custom_command(
	TARGET "uninstall"
	POST_BUILD
	COMMAND xargs < install_manifest.txt rm -v
	COMMAND rmdir -v ${CMAKE_INSTALL_PREFIX}/share/doc/${PROJECT_NAME}
)

set(CPACK_PACKAGE_CONTACT "Steve Dibb <steve.dibb@gmail.com>")
set(CPACK_PACKAGE_DESCRIPTION "Get optical disc type")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/beandog/disc_type")
