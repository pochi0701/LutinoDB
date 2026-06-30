# CMakeList.txt : MSQL の CMake プロジェクト。ソースを含めて、次を定義します:
# プロジェクト専用ロジックはこちらです。
#
cmake_minimum_required (VERSION 3.8)

# サポートされている場合は、MSVC コンパイラのホット リロードを有効にします。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("MSQL")
option(STARTUP_SQL_TEST "Enable startup SQL sample tests at launch" OFF)

# ソースをこのプロジェクトの実行可能ファイルに追加します。
add_executable (MSQL 
"ltn_db.cpp"
"ltn_String.cpp"
"node.hpp"
"ltn_tools.cpp"
"ltn_db.h"
"ltn_String.h"
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET MSQL PROPERTY CXX_STANDARD 20)
endif()

if (STARTUP_SQL_TEST)
  target_compile_definitions(MSQL PRIVATE STARTUP_SQL_TEST)
  add_custom_command(TARGET MSQL POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:MSQL>/database"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/database/startup_sql_tests.txt"
            "$<TARGET_FILE_DIR:MSQL>/database/startup_sql_tests.txt")
endif()

# TODO: テストを追加し、必要な場合は、ターゲットをインストールします。
