# - Find the xkbcommon library
# This Module defines
#   XKBCOMMON_FOUND, system has xkbcommon
#   XKBCOMMON_INCLUDE_DIR, the include directory
#   XKBCOMMON_LIBRARY, the xkbcommon libraries

find_package(PkgConfig)
pkg_check_modules(XKBCOMMON QUIET xkbcommon)

if (XKBCOMMON_FOUND)
    set(XKBCOMMON_LIBRARY "${XKBCOMMON_LIBRARIES}")
    set(XKBCOMMON_INCLUDE_DIR "${XKBCOMMON_INCLUDE_DIRS}")
endif()


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    XkbCommon
    DEFAULT_MSG
    XKBCOMMON_LIBRARY
)
