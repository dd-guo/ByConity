option (ENABLE_JAVA_EXTENSIONS "Enable java-extensions" OFF)

if (NOT ENABLE_JAVA_EXTENSIONS)
    return()
endif()

set(JAVA_HOME $ENV{JAVA_HOME})
if (NOT JAVA_HOME)
    message("JAVA_HOME is not set")
else()
    message("JAVA_HOME: ${JAVA_HOME}")
    add_library(jvm SHARED IMPORTED)
    file(GLOB_RECURSE LIB_JVM ${JAVA_HOME}/jre/lib/*/server/libjvm.so)
    message("LIB_JVM: ${LIB_JVM}")
    set_target_properties(jvm PROPERTIES IMPORTED_LOCATION ${LIB_JVM})
    include_directories(${JAVA_HOME}/include)
    include_directories(${JAVA_HOME}/include/linux)
    set (USE_JNI 1)
endif()

message (STATUS "Using jni=${USE_JNI}")

# optional nanoarrow library
if (EXISTS "${ClickHouse_SOURCE_DIR}/contrib/arrow-nanoarrow/CMakeLists.txt")
    cmake_minimum_required(VERSION 3.11)
    include(FetchContent)
    set(NANOARROW_NAMESPACE "NanoArrow")
    FetchContent_Declare(
        nanoarrow_content
        SOURCE_DIR ${ClickHouse_SOURCE_DIR}/contrib/arrow-nanoarrow)
    FetchContent_MakeAvailable(nanoarrow_content)
    set (USE_NANOARROW 1)
else()
    message(WARNING "submodule contrib/arrow-nanoarrow is missing. to fix try run: \n git submodule update --init --recursive")
    set (USE_NANOARROW 0)
endif()
message (STATUS "Using nanoarrow=${USE_NANOARROW}")

# depends on protobuf
if (USE_JNI AND USE_NANOARROW AND USE_PROTOBUF)
    set (USE_JAVA_EXTENSIONS 1)
endif ()
message (STATUS "Using java extensions=${USE_JAVA_EXTENSIONS}")
