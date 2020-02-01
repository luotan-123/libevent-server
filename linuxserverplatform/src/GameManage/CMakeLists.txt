
include_directories(
    ${CMAKE_SOURCE_DIR}/
    ${CMAKE_SOURCE_DIR}/common/
    ${CMAKE_SOURCE_DIR}/src/
)

link_directories(
    ${CMAKE_SOURCE_DIR}/
    ${CMAKE_SOURCE_DIR}/common/
    ${CMAKE_SOURCE_DIR}/src/
)

set(SERVER_SRC
    main.cpp
    server.cpp
    worker.cpp
    handlers.cpp
)

ADD_EXECUTABLE(http-server ${SERVER_SRC})
target_link_libraries(http-server
    event
    pthread
)

