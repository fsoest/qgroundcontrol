find_package(Qt6 REQUIRED COMPONENTS Core Test)

qt_add_library(FollowMeTest
    STATIC
        FollowMeTest.cc
        FollowMeTest.h
)

target_link_libraries(FollowMeTest
    PRIVATE
        Qt6::Test
        QGC
    PUBLIC
        qgcunittest
)

target_include_directories(FollowMeTest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
