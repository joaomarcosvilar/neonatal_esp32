idf_component_register(
    SRCS
        "main.c"
        "hardware/files/fs_manager.c"
        "hardware/temperature/temperature.c"
        "hardware/temperature/ds18x20.c"
        "library/onewire.c"

    INCLUDE_DIRS
        "."
        "hardware"
        "hardware/temperature"
        "hardware/files"
        "library"
)
