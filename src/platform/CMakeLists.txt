target_sources(flameshot PRIVATE
    windowlist.h
    windowpick.h
    windowpick.cpp
    screenrecorder.h
    gifencoder.h
    videothumbnail.h
)

if (APPLE)
    target_sources(flameshot PRIVATE
        macos/windowlist_mac.cpp
        macos/screenrecorder.mm
        macos/gifencoder.mm
        macos/videothumbnail.mm
    )
    set_source_files_properties(
        macos/screenrecorder.mm
        macos/gifencoder.mm
        macos/videothumbnail.mm
        PROPERTIES COMPILE_FLAGS "-fobjc-arc")
    # ScreenCaptureKit is linked weakly so the app still starts on macOS
    # versions that lack it; recording is hidden there.
    target_link_libraries(flameshot
        "-framework CoreGraphics"
        "-framework CoreFoundation"
        "-framework AppKit"
        "-framework AVFoundation"
        "-framework CoreMedia"
        "-framework CoreVideo"
        "-framework ImageIO"
        "-framework UniformTypeIdentifiers"
        "-weak_framework ScreenCaptureKit"
    )
else ()
    target_sources(flameshot PRIVATE windowlist_stub.cpp recording_stub.cpp)
endif ()
