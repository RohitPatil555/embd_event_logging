# src/CMakeLists.txt
add_library(embdEventLog STATIC)

target_include_directories(embdEventLog PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../tmpl>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../genHdr>
    $<INSTALL_INTERFACE:include>
)

# Link with sanitizers if needed (they are added globally already)
if(SANITIZERS)
    target_link_options(embdEventLog PRIVATE ${SANITIZERS})
endif()

set(EMBD_EVENT_LOG_SORUCES
        eventCollector.cpp
        eventPacket.cpp
)

target_sources(embdEventLog PRIVATE ${EMBD_EVENT_LOG_SORUCES})

# Export the library for other projects
export(TARGETS embdEventLog FILE EmbdEventLogConfig.cmake)
