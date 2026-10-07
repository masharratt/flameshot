target_sources(flameshot PRIVATE
    actionlist.h
    actionrunner.h
    capturefirstrequest.h
    flameshot.h
    hotkeyutils.h
    flameshotdaemon.h
    qguiappcurrentscreen.h
)

target_sources(flameshot PRIVATE
    actionlist.cpp
    actionrunner.cpp
    capturefirstrequest.cpp
    capturerequest.cpp
    flameshot.cpp
    flameshotdaemon.cpp
    hotkeyutils.cpp
    qguiappcurrentscreen.cpp
)

if (UNIX AND NOT APPLE)
target_sources(flameshot PRIVATE
    flameshotdbusadapter.h
    flameshotdbusadapter.cpp
)
endif ()

if (USE_KDSINGLEAPPLICATION)
if (NOT WIN32)
target_sources(flameshot PRIVATE
    signaldaemon.h
    signaldaemon.cpp
)
endif ()
endif ()

if (WIN32)
    target_sources(flameshot PRIVATE globalshortcutfilter.h globalshortcutfilter.cpp)
endif ()
