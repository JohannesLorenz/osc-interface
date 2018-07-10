# WARNING: Not working, and *may* be removed in future versions
# use pkg config instead (or fix this file...)

# - Try to find LibSpa
# Once done this will define
#  LIBSPA_FOUND - System has LibSPA
#  LIBSPA_INCLUDE_DIRS - The LibSPA include directories
#  LIBSPA_LIBRARIES - The libraries needed to use LibSPA
#  LIBSPA_DEFINITIONS - Compiler switches required for using LibSPA


message(STATUS "FOUND SPA CONFIG!!!!!")
set(LIBSPA_INCLUDE_DIR include)
set(LIBSPA_LIBRARY libspa)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBSPA_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(LibSPA  DEFAULT_MSG
				  LIBSPA_LIBRARY LIBSPA_INCLUDE_DIR)

mark_as_advanced(LIBSPA_INCLUDE_DIR LIBSPA_LIBRARY )

set(LIBSPA_LIBRARIES ${LIBSPA_LIBRARY} )
set(LIBSPA_INCLUDE_DIRS ${LIBSPA_INCLUDE_DIR} )

