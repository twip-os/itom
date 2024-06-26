# - Try to find XSD
# Once done this will define
#
#  XSD_FOUND - system has Xerces-C
#  XSD_INCLUDE_DIR - the Xerces-C include directory
#  XSD_LIBARY_DIR - the Xerces-C include directory
#  XSD_VERSION - Xerces-C found version
#
if((CMAKE_MAJOR_VERSION GREATER 2) AND (CMAKE_MAJOR_VERSION LESS 4) AND (CMAKE_MINOR_VERSION GREATER 1))
	cmake_policy(SET CMP0053 NEW)
endif((CMAKE_MAJOR_VERSION GREATER 2) AND (CMAKE_MAJOR_VERSION LESS 4) AND (CMAKE_MINOR_VERSION GREATER 1))

if(XSD_INCLUDE_DIR AND XSD_EXECUTABLE)
# in cache already
set(XSD_FIND_QUIETLY TRUE)
endif(XSD_INCLUDE_DIR AND XSD_EXECUTABLE)

if(BUILD_TARGET64)
    set(POSTFIX, "64")
else (BUILD_TARGET64)
    set(POSTFIX, "")
endif(BUILD_TARGET64)

set(XSD_POSSIBLE_ROOT_DIRS
  "$ENV{XSD_ROOT}"
  "$ENV{XSDDIR}"
  /usr/local
  /usr
  "$ENV{ProgramFiles}/CodeSynthesis XSD 3.3"
  "$ENV{ProgramFiles}(x86)/CodeSynthesis XSD 3.3"
  "$ENV{ProgramW6432}/CodeSynthesis XSD 3.3"
  "$ENV{ProgramFiles}/CodeSynthesis XSD 3.3/bin"
  "$ENV{ProgramFiles}(x86)/CodeSynthesis XSD 3.3/bin"
  "$ENV{ProgramW6432}/CodeSynthesis XSD 3.3/bin"
  "$ENV{ProgramFiles}/CodeSynthesis XSD 4.0"
  "$ENV{ProgramFiles}(x86)/CodeSynthesis XSD 4.0"
  "$ENV{ProgramW6432}/CodeSynthesis XSD 4.0"
  "$ENV{ProgramFiles}/CodeSynthesis XSD 4.0/bin$POSTFIX"
  "$ENV{ProgramFiles}(x86)/CodeSynthesis XSD 4.0/bin$POSTFIX"
  "$ENV{ProgramW6432}/CodeSynthesis XSD 4.0/bin$POSTFIX"
  ${CMAKE_SOURCE_DIR}/../xsd/libxsd
  "$ENV{PATH}"
  )

 find_path(XSD_ROOT_DIR
  NAMES
  include/xsd/cxx/parser/elements.hxx
  PATHS ${XSD_POSSIBLE_ROOT_DIRS}
  )

if(WIN32)
  set(XSD_EXE_NAME xsd)
endif(WIN32)
if(UNIX)
  set(XSD_EXE_NAME xsdcxx)
endif(UNIX)
if(APPLE)
  set(XSD_EXE_NAME xsd)
endif(APPLE)

find_path(XSD_INCLUDE_DIR xsd/cxx/parser/elements.hxx
  PATHS "[HKEY_CURRENT_USER\\software\\xsd\\include]"
  "[HKEY_CURRENT_USER]\\xsd\\include]"
  "$ENV{XSD_ROOT}/include"
  "$ENV{XSDDIR}/include"
  "$ENV{XSDDIR}/libxsd"
  /usr/local/include
  /usr/include
  "$ENV{ProgramFiles}/CodeSynthesis XSD 3.3/include"
  "$ENV{ProgramFiles}(x86)/CodeSynthesis XSD 3.3/include"
  "$ENV{ProgramW6432}/CodeSynthesis XSD 3.3/include"
  "$ENV{ProgramFiles}/CodeSynthesis XSD 4.0/include"
  "$ENV{ProgramFiles}(x86)/CodeSynthesis XSD 4.0/include"
  "$ENV{ProgramW6432}/CodeSynthesis XSD 4.0/include"
  ${CMAKE_SOURCE_DIR}/../xsd/libxsd
  "${XSD_ROOT_DIR}/include"
  "${XSD_ROOT_DIR}/libxsd"
  "${XSD_ROOT_DIR}"
)

find_program(XSD_EXECUTABLE
  NAMES xsdcxx xsd
  PATHS "${XSD_ROOT_DIR}"
  "${XSD_ROOT_DIR}/bin"
  "[HKEY_CURRENT_USER\\software\\xsd\\bin]"
  "$ENV{ProgramFiles}/CodeSynthesis XSD 3.3/bin"
  "$ENV{ProgramFiles}(x86)/CodeSynthesis XSD 3.3/bin"
  "$ENV{ProgramW6432}/CodeSynthesis XSD 3.3/bin"
  "$ENV{ProgramFiles}/CodeSynthesis XSD 4.0/bin$POSTFIX"
  "$ENV{ProgramFiles}(x86)/CodeSynthesis XSD 4.0/bin$POSTFIX"
  "$ENV{ProgramW6432}/CodeSynthesis XSD 4.0/bin$POSTFIX"
  "$ENV{PATH}"
  "$ENV{XSDDIR}/bin$POSTFIX"
)

# XSD VERSION
file(READ "${XERCESC_INCLUDE}/xsd/cxx/version.hxx" XSD_VER_FILE)
string(REGEX MATCH "XSD_INT_VERSION ([0-9]0[0-9]0000L*)" _ ${XSD_VER_FILE})
set(XSD_VERSION ${CMAKE_MATCH_1})

if(DEFINED MSVC_VERSION)
    # Library postfix/ prefix for different vs version
    #   1300 = VS  7.0
    #   1400 = VS  8.0
    #   1500 = VS  9.0
    #   1600 = VS 10.0
    if(MSVC_VERSION EQUAL 1300)
        set(XSD_LIB_POSTFIX "_vc70")
        set(XSD_LIBPATH_VERS_POSTFIX "vc-7.1/")
		# since we don't know whether we are on windows or not, we just undefined and see what happens
		unset(XERCES_LIB_PATH_POSTFIX)
    elseif(MSVC_VERSION EQUAL 1400)
        set(XSD_LIB_POSTFIX "_vc80")
        set(XSD_LIBPATH_VERS_POSTFIX "vc-8.0/")
    elseif(MSVC_VERSION EQUAL 1500)
        set(XSD_LIB_POSTFIX "_vc90")
        set(XSD_LIBPATH_VERS_POSTFIX "vc-9.0/")
    elseif(MSVC_VERSION EQUAL 1600)
        set(XSD_LIB_POSTFIX "_vc100")
        set(XSD_LIBPATH_VERS_POSTFIX "vc-10.0/")
	elseif(MSVC_VERSION EQUAL 1800)
		if(XSD_VERSION EQUAL "4000000L")
			set(XSD_LIB_POSTFIX "_vc120")
			set(XSD_LIBPATH_VERS_POSTFIX "vc-12.0/")
		elseif(XSD_VERSION EQUAL "3030000L")
			set(XSD_LIB_POSTFIX "_vc100")
			set(XSD_LIBPATH_VERS_POSTFIX "vc-10.0/")
		endif(XSD_VERSION EQUAL "4000000L")
	elseif(MSVC)
		#for all newer versions than VS 2013, use the libraries for VS 2013
		if(XSD_VERSION EQUAL "4000000L")
			set(XSD_LIB_POSTFIX "_vc120")
			set(XSD_LIBPATH_VERS_POSTFIX "vc-12.0/")
		elseif(XSD_VERSION EQUAL "3030000L")
			set(XSD_LIB_POSTFIX "_vc100")
			set(XSD_LIBPATH_VERS_POSTFIX "vc-10.0/")
		endif(XSD_VERSION EQUAL "4000000L")
	endif(MSVC_VERSION EQUAL 1300)

	# Wiora: Set 64 bit target dir (currently this is windows only. How does this work on linux/mac?)
    if(CMAKE_CL_64)
        set(XSD_LIBPATH_POSTFIX lib64/)
    else(CMAKE_CL_64)
        set(XSD_LIBPATH_POSTFIX lib/)
    endif(CMAKE_CL_64)

    set(XSD_LIBPATH_POSTFIX ${XSD_LIBPATH_POSTFIX} ${XSD_LIBPATH_VERS_POSTFIX})

else(DEFINED MSVC_VERSION)
    set(XSD_LIBPATH_POSTFIX "")
	set(XSD_LIBPATH_VERS_POSTFIX "")
    set(XSD_LIB_POSTFIX "")
endif(DEFINED MSVC_VERSION)

set(XSD_LIBRARY_DIR "${XSD_INCLUDE_DIR}/${XSD_LIBPATH_POSTFIX}")

if(NOT XSD_INCLUDE_DIR)
 set(XSD_VERSION "0")
else(NOT XSD_INCLUDE_DIR)
 file(READ ${XSD_INCLUDE_DIR}/xsd/cxx/version.hxx XVERHXX)
 string(REGEX MATCHALL "\n *#define XSD_INT_VERSION +[0-9]+" XVERINT ${XVERHXX})
 string(REGEX REPLACE "\n *#define XSD_INT_VERSION +" "" XVERINT ${XVERINT})
 string(REGEX REPLACE "....$" "" XVERINT ${XVERINT})
 string(REGEX MATCHALL "..$" XVERMIN ${XVERINT})
 string(REGEX REPLACE "..$" "" XVERMAJ ${XVERINT})

 set(XSD_VERMAJ ${XVERMAJ})
 set(XSD_VERMIN ${XVERMIN})
endif(NOT XSD_INCLUDE_DIR)

if(NOT XSD_INCLUDE_DIR)
    if(XSD_FIND_REQUIRED)
        message(FATAL_ERROR "Unable to find xsd include files (xsd/cxx/parser/elements.hxx)")
    endif()
else (NOT XSD_INCLUDE_DIR)
  if(NOT XSD_FIND_QUIETLY)
    message(STATUS "Found xsd: " ${XSD_INCLUDE_DIR})
    message(STATUS "         : " ${XSD_EXECUTABLE})
    message(STATUS "xsd Ver. : " ${XSD_VERMAJ} "." ${XSD_VERMIN})
  endif(NOT XSD_FIND_QUIETLY)
endif(NOT XSD_INCLUDE_DIR)

if(NOT XSD_EXECUTABLE)
    if(XSD_FIND_REQUIRED)
        message(FATAL_ERROR "Unable to find xsd or xsdcxx executable")
    endif()
    set(XSD_EXECUTABLE)
    unset(XSD_EXECUTABLE CACHE)
endif(NOT XSD_EXECUTABLE)

if((NOT (XSD_VERMAJ GREATER 3)) AND (NOT ((XSD_VERMAJ GREATER 2) AND (XSD_VERMIN GREATER 2))))
    set(XSD_INCLUDE_DIR )
    unset(XSD_INCLUDE_DIR CACHE)
    if(XSD_FIND_REQUIRED)
        message(FATAL_ERROR "XSD version number mismatch")
    endif()
endif((NOT (XSD_VERMAJ GREATER 3)) AND (NOT ((XSD_VERMAJ GREATER 2) AND (XSD_VERMIN GREATER 2))))

#
# General CMake package configuration.
#
include( FindPackageHandleStandardArgs )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( XSD DEFAULT_MSG XSD_EXECUTABLE XSD_INCLUDE_DIR )

mark_as_advanced( XSD_INCLUDE_DIR XSD_EXECUTABLE )

#
# Macro that attempts to generate C++ files from an XML schema. The NAME
# argument is the name of the CMake variable to use to store paths to the
# derived C++ source file. The FILE argument is the path of the schema file to
# process. Additional arguments should be XSD command-line options.
#
# Example:
#
# XSD_SCHEMA( FOO_SRCS Foo.xsd --root-element-first --generate-serialization )
#
# On return, FOO_SRCS will contain Foo.cxx.
#
MACRO( XSD_SCHEMA NAME FILE )
  #
  # Make a full path from the source directory
  #
  set( xs_SRC "${FILE}" )

  #
  # XSD will generate two or three C++ files (*.cxx,*.hxx,*.ixx). Get the
  # destination file path sans any extension and then build paths to the
  # generated files.
  #
  get_filename_component( xs_FILE "${FILE}" NAME_WE )
  set( xs_CXX "${CMAKE_CURRENT_BINARY_DIR}/${xs_FILE}.cxx" )
  set( xs_HXX "${CMAKE_CURRENT_BINARY_DIR}/${xs_FILE}.hxx" )
#  set( xs_IXX "${CMAKE_CURRENT_BINARY_DIR}/${xs_FILE}.ixx" )


  #
  # Add the source files to the NAME variable, which presumably will be used to
  # define the source of another target.
  #
  list( APPEND ${NAME} ${xs_CXX} )

  #
  # Set up a generator for the output files from the given schema file using
  # the XSD cxx-tree command.
  #
  add_custom_target( genxmlxsd ALL)
  add_custom_command( TARGET genxmlxsd PRE_BUILD
			COMMAND ${XSD_EXECUTABLE}
			ARGS "cxx-tree" ${ARGN} ${xs_SRC}
			DEPENDS ${xs_SRC} )

  #
  # Don't fail if a generated file does not exist.
  #
  set_source_files_properties( "${xs_CXX}" "${xs_HXX}" "${xs_IXX}"
  							   PROPERTIES GENERATED TRUE )

ENDMACRO( XSD_SCHEMA )
