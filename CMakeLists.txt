CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(smack C)

SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(EXEC_PREFIX "\${prefix}")
SET(LIBDIR "\${prefix}/lib")
SET(INCLUDEDIR "\${prefix}/include")
SET(VERSION_MAJOR 1)
SET(VERSION ${VERSION_MAJOR}.0.0)

SET(smack_dir "./")
SET(libsmack_dir "./libsmack")
SET(smackutils_dir "./utils")

SET(EXTRA_CFLAGS "${EXTRA_CFLAGS}")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")

###################################################################################################
## for libsmack.so (library)
SET(libsmack_LDFLAGS " -version-info 1:0:0 -Wl,--version-script=${libsmack_dir}/libsmack.sym ")
SET(libsmack_CFLAGS  " ${CFLAGS} -fPIC -I${libsmack_dir}/sys ")
ADD_LIBRARY(smack SHARED ${libsmack_dir}/libsmack.c)
SET_TARGET_PROPERTIES(smack PROPERTIES COMPILE_FLAGS ${libsmack_LDFLAGS})
SET_TARGET_PROPERTIES(smack PROPERTIES SOVERSION ${VERSION_MAJOR})
SET_TARGET_PROPERTIES(smack PROPERTIES VERSION ${VERSION})
SET_TARGET_PROPERTIES(smack PROPERTIES COMPILE_FLAGS "${libsmack_CFLAGS}")
###################################################################################################

###################################################################################################
## for smack-utils (binary)
SET(smackctl_SOURCES ${smackutils_dir}/smackctl.c ${smackutils_dir}/common.c)
SET(smackctl_CFLAGS " -I${libsmack_dir} -I${smackutils_dir} -D_GNU_SOURCE ")
ADD_EXECUTABLE(smackctl ${smackctl_SOURCES})
TARGET_LINK_LIBRARIES(smackctl smack)
SET_TARGET_PROPERTIES(smackctl PROPERTIES COMPILE_FLAGS "${smackctl_CFLAGS}")

SET(smackaccess_SOURCES ${smackutils_dir}/smackaccess.c)
SET(smackaccess_CFLAGS " -I${libsmack_dir} ")
ADD_EXECUTABLE(smackaccess ${smackaccess_SOURCES})
TARGET_LINK_LIBRARIES(smackaccess smack)
SET_TARGET_PROPERTIES(smackaccess PROPERTIES COMPILE_FLAGS "${smackaccess_CFLAGS}")

SET(smackload_SOURCES ${smackutils_dir}/smackload.c ${smackutils_dir}/common.c)
SET(smackload_CFLAGS " -I${libsmack_dir} -I${smackutils_dir} -D_GNU_SOURCE")
ADD_EXECUTABLE(smackload ${smackload_SOURCES})
TARGET_LINK_LIBRARIES(smackload smack)
SET_TARGET_PROPERTIES(smackload PROPERTIES COMPILE_FLAGS "${smackload_CFLAGS}")

SET(smackcipso_SOURCES ${smackutils_dir}/smackcipso.c ${smackutils_dir}/common.c)
SET(smackcipso_CFLAGS " -I${libsmack_dir} -I${smackutils_dir} -D_GNU_SOURCE ")
ADD_EXECUTABLE(smackcipso ${smackcipso_SOURCES})
TARGET_LINK_LIBRARIES(smackcipso smack)
SET_TARGET_PROPERTIES(smackcipso PROPERTIES COMPILE_FLAGS "${smackcipso_CFLAGS}")

SET(smackd_SOURCES ${smackutils_dir}/smackd.c ${smackutils_dir}/common.c)
SET(smackd_CFLAGS " -I${libsmack_dir} -I${smackutils_dir} -D_GNU_SOURCE ")
ADD_EXECUTABLE(smackd ${smackd_SOURCES})
TARGET_LINK_LIBRARIES(smackd smack)
SET_TARGET_PROPERTIES(smackd PROPERTIES COMPILE_FLAGS "${smackd_CFLAGS}")

SET(chsmack_SOURCES ${smackutils_dir}/chsmack.c)
SET(chsmack_CFLAGS " -I${libsmack_dir} ")
ADD_EXECUTABLE(chsmack ${chsmack_SOURCES})
TARGET_LINK_LIBRARIES(chsmack smack)
SET_TARGET_PROPERTIES(chsmack PROPERTIES COMPILE_FLAGS "${chsmack_CFLAGS}")
####################################################################################################

CONFIGURE_FILE(libsmack.pc.in libsmack.pc @ONLY)

INSTALL(TARGETS smack DESTINATION lib)
INSTALL(PROGRAMS ${CMAKE_BINARY_DIR}/smackctl DESTINATION bin)
INSTALL(PROGRAMS ${CMAKE_BINARY_DIR}/smackaccess DESTINATION bin)
INSTALL(PROGRAMS ${CMAKE_BINARY_DIR}/smackload DESTINATION /sbin)
INSTALL(PROGRAMS ${CMAKE_BINARY_DIR}/smackcipso DESTINATION /sbin)
INSTALL(PROGRAMS ${CMAKE_BINARY_DIR}/smackd DESTINATION bin)
INSTALL(PROGRAMS ${CMAKE_BINARY_DIR}/chsmack DESTINATION /bin)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/libsmack.pc DESTINATION lib/pkgconfig)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/libsmack/sys/smack.h DESTINATION include/sys/)
INSTALL(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/smack-def DESTINATION /etc/rc.d/init.d)
INSTALL(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/smack-app DESTINATION /etc/rc.d/init.d)
