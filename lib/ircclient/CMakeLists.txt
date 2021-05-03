include_directories(. include/)
add_definitions(-DIN_BUILDING_LIBIRC)

add_library(bundled_libircclient STATIC
	include/libirc_errors.h
	include/libirc_events.h
	include/libirc_options.h
	include/libirc_rfcnumeric.h
	include/libircclient.h
	# src/dcc.h
	# src/config.h
	# src/params.h
	# src/session.h
	# src/colors.c
	# src/dcc.c
	# src/errors.c
	libircclient.c
	# src/portable.c
	# src/sockets.c
	# src/ssl.c
	# src/utils.c
)

set(IRCCLIENT_LIBRARIES bundled_libircclient PARENT_SCOPE)
set(IRCCLIENT_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/lib/ircclient/include PARENT_SCOPE)

mark_as_advanced(IRCCLIENT_LIBRARIES IRCCLIENT_INCLUDE_DIRS)

set_property(TARGET bundled_libircclient PROPERTY POSITION_INDEPENDENT_CODE True)
