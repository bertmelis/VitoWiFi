set(COMPONENT_SRCDIRS
    "src" "src/Datapoint" "src/GWG" "src/VS1" "src/VS2" "src/Interface"
)

set(COMPONENT_ADD_INCLUDEDIRS
    "src" "src/Datapoint" "src/GWG" "src/VS1" "src/VS2" "src/Interface"
)

set(COMPONENT_REQUIRES
    "arduino-esp32"
)

register_component()

target_compile_definitions(${COMPONENT_TARGET} PUBLIC -DESP32)
target_compile_options(${COMPONENT_TARGET} PRIVATE -fno-rtti)
