cmake_minimum_required(VERSION 3.2)

PROJECT(dmcmd4vsp)

LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
INCLUDE(cmake/ModuleImport.cmake)
INCLUDE(cmake/ModuleCompileOptions.cmake)
ModuleSetCompileOptions()
ModuleSetWinCompilerFlags()

ModuleInclude("dmformat" "thirdparty/dmformat")
ModuleImport("dmflags" "thirdparty/dmflags")
ModuleImport("dmtimer" "thirdparty/dmtimer")
LibImport("execute_module" "src/execute_module")
ExeImport("tools" "execute_module;dmflags;dmtimer")

FILE(GLOB SUBDIRS
  "${CMAKE_CURRENT_SOURCE_DIR}/config/*.*"
)
ADD_CUSTOM_COMMAND(
    TARGET dmcmd4vsp
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${SUBDIRS}
    $<TARGET_FILE_DIR:dmcmd4vsp>
)