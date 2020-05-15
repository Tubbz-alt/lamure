##############################################################################
# search paths
##############################################################################
SET(SCHISM_INCLUDE_SEARCH_DIRS
  ${GLOBAL_EXT_DIR}/schism/include
  ${GLOBAL_EXT_DIR}/include/schism
  ${SCHISM_INCLUDE_SEARCH_DIR}
  /opt/schism/current
  ../schism/include
  ../schism
)

SET(SCHISM_LIBRARY_SEARCH_DIRS
  ${GLOBAL_EXT_DIR}/lib
  ${GLOBAL_EXT_DIR}/schism/lib
  ${SCHISM_LIBRARY_SEARCH_DIR}
  ../
  /opt/schism/current/lib/linux_x86
  ../schism/lib
  ../schism/lib/linux_x86
)

message(${GLOBAL_EXT_DIR})

##############################################################################
# check for schism
##############################################################################
message(STATUS "-- checking for schism")

IF ( NOT SCHISM_INCLUDE_DIRS )

    FOREACH(_SEARCH_DIR ${SCHISM_INCLUDE_SEARCH_DIRS})
        FIND_PATH(_CUR_SEARCH
                NAMES scm_gl_core/src/scm/gl_core.h
                PATHS ${_SEARCH_DIR}
                NO_DEFAULT_PATH)
        IF (_CUR_SEARCH)
            LIST(APPEND _SCHISM_FOUND_INC_DIRS ${_CUR_SEARCH})
        ENDIF(_CUR_SEARCH)
        SET(_CUR_SEARCH _CUR_SEARCH-NOTFOUND CACHE INTERNAL "internal use")
    ENDFOREACH(_SEARCH_DIR ${SCHISM_INCLUDE_SEARCH_DIRS})

    IF (NOT _SCHISM_FOUND_INC_DIRS)
        request_schism_search_directories()
    ENDIF (NOT _SCHISM_FOUND_INC_DIRS)

    FOREACH(_INC_DIR ${_SCHISM_FOUND_INC_DIRS})
        LIST(APPEND _SCHISM_INCLUDE_DIRS ${_INC_DIR}/scm_cl_core/src)
        LIST(APPEND _SCHISM_INCLUDE_DIRS ${_INC_DIR}/scm_core/src)
        LIST(APPEND _SCHISM_INCLUDE_DIRS ${_INC_DIR}/scm_gl_core/src)
        LIST(APPEND _SCHISM_INCLUDE_DIRS ${_INC_DIR}/scm_gl_util/src)
        LIST(APPEND _SCHISM_INCLUDE_DIRS ${_INC_DIR}/scm_input/src)
    ENDFOREACH(_INC_DIR ${_BOOST_FOUND_INC_DIRS})

    IF (_SCHISM_FOUND_INC_DIRS)
        SET(SCHISM_INCLUDE_DIRS ${_SCHISM_INCLUDE_DIRS} CACHE PATH "path to schism headers.")
    ENDIF (_SCHISM_FOUND_INC_DIRS)

ENDIF ( NOT SCHISM_INCLUDE_DIRS )

# release libraries
find_library(SCHISM_CORE_LIBRARY 
             NAMES scm_core libscm_core
             PATHS ${SCHISM_LIBRARY_SEARCH_DIRS}
             SUFFIXES release
            )

find_library(SCHISM_GL_CORE_LIBRARY 
             NAMES scm_gl_core libscm_gl_core
             PATHS ${SCHISM_LIBRARY_SEARCH_DIRS}
             SUFFIXES release
            )

find_library(SCHISM_GL_UTIL_LIBRARY 
             NAMES scm_gl_util libscm_gl_util
             PATHS ${SCHISM_LIBRARY_SEARCH_DIRS}
             SUFFIXES release
            )

find_library(SCHISM_INPUT_LIBRARY 
             NAMES scm_input libscm_input
             PATHS ${SCHISM_LIBRARY_SEARCH_DIRS}
             SUFFIXES release
            )

# find debug libraries
find_library(SCHISM_CORE_LIBRARY_DEBUG
             NAMES scm_core-gd libscm_core-gd scm_core libscm_core
             PATHS ${SCHISM_LIBRARY_SEARCH_DIRS}
             SUFFIXES debug
            )

find_library(SCHISM_GL_CORE_LIBRARY_DEBUG
             NAMES scm_gl_core-gd libscm_gl_core-gd scm_gl_core libscm_gl_core
             PATHS ${SCHISM_LIBRARY_SEARCH_DIRS}
             SUFFIXES debug
            )

find_library(SCHISM_GL_UTIL_LIBRARY_DEBUG
             NAMES scm_gl_util-gd libscm_gl_util-gd scm_gl_util libscm_gl_util
             PATHS ${SCHISM_LIBRARY_SEARCH_DIRS}
             SUFFIXES debug
            )

find_library(SCHISM_INPUT_LIBRARY_DEBUG
             NAMES scm_input-gd libscm_input-gd scm_input libscm_input
             PATHS ${SCHISM_LIBRARY_SEARCH_DIRS}
             SUFFIXES debug
            )
