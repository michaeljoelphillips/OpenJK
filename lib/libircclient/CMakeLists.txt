include_directories(. include/libircclient)
add_definitions(-DIN_BUILDING_LIBIRC -DENABLE_THREADS -D_REENTRANT)

add_library(bundled_libircclient STATIC
  include/libircclient/libirc_errors.h
  include/libircclient/libirc_events.h
  include/libircclient/libirc_options.h
  include/libircclient/libirc_rfcnumeric.h
  include/libircclient/libircclient.h
  libircclient.c
)

set(IRCCLIENT_LIBRARIES bundled_libircclient PARENT_SCOPE)
set(IRCCLIENT_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/lib/libircclient/include PARENT_SCOPE)

mark_as_advanced(IRCCLIENT_LIBRARIES IRCCLIENT_INCLUDE_DIRS)

set_property(TARGET bundled_libircclient PROPERTY POSITION_INDEPENDENT_CODE True)
