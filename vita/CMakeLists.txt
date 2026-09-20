cmake_minimum_required(VERSION 3.2)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    set(CMAKE_TOOLCHAIN_FILE "/usr/local/vitasdk/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  endif()
endif()

project(psybnc_vita C)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(VITA_APP_NAME "psyBNC")
set(VITA_TITLEID  "PSYB00001")
set(VITA_VERSION  "01.00")

set(PSYBNC_ROOT ${CMAKE_SOURCE_DIR}/..)
set(PSYBNC_SRC  ${PSYBNC_ROOT}/src)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g -DVITA -DHAVE_CONFIG \
-Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-function -Wno-format-truncation")

include_directories(
  ${CMAKE_SOURCE_DIR}/compat/include  # sys/socket.h, netinet/in.h, arpa/inet.h, netdb.h shims
  ${CMAKE_SOURCE_DIR}/compat          # vita_net_compat.h, debugScreen.h
  ${CMAKE_SOURCE_DIR}/src             # vita_platform.h
  ${PSYBNC_SRC}                       # p_global.h and friends, included as <p_global.h>
  ${PSYBNC_ROOT}                      # config.h
)

file(GLOB PSYBNC_SOURCES ${PSYBNC_SRC}/*.c)
# newlib already provides setenv/getenv/unsetenv; this file would redefine them.
list(REMOVE_ITEM PSYBNC_SOURCES ${PSYBNC_SRC}/bsd-setenv.c)

add_executable(${PROJECT_NAME}
  ${PSYBNC_SOURCES}
  src/vita_platform.c
  compat/vita_net_compat.c
  compat/vita_libc_stubs.c
  compat/debugScreen.c
)

target_link_libraries(${PROJECT_NAME}
  SceNet_stub
  SceNetCtl_stub
  SceDisplay_stub
  SceCtrl_stub
  SceSysmodule_stub
  ScePower_stub
  pthread
  m
)

vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME} UNSAFE)
vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
  FILE sce_sys/icon0.png sce_sys/icon0.png
  FILE sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
  FILE sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
  FILE sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
  FILE ${PSYBNC_ROOT}/lang assets/lang
  FILE ${PSYBNC_ROOT}/help assets/help
  FILE ${PSYBNC_ROOT}/motd assets/motd
  FILE ${CMAKE_SOURCE_DIR}/data_template/psybnc.conf assets/conf/psybnc.conf
)
