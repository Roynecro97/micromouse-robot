idf_component_register(
    SRCS
        main.cpp
        motor.cpp
        periodic_caller.cpp
        pid_controller.cpp
    INCLUDE_DIRS
        .
)
target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu++23)
