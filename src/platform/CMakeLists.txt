target_sources(flameshot PRIVATE
    windowlist.h
    windowpick.h
    windowpick.cpp
)

if (APPLE)
    target_sources(flameshot PRIVATE macos/windowlist_mac.cpp)
    target_link_libraries(flameshot
        "-framework CoreGraphics"
        "-framework CoreFoundation"
    )
else ()
    target_sources(flameshot PRIVATE windowlist_stub.cpp)
endif ()
