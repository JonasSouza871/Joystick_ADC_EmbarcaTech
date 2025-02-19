if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()

cmake_minimum_required(VERSION 3.13)

# Configurações básicas
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Configuração do SDK
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()

set(PICO_BOARD pico_w CACHE STRING "Board type")
include(pico_sdk_import.cmake)

project(joystick C CXX ASM)
pico_sdk_init()

# Adiciona os arquivos fontes
add_executable(joystick
    main.c
    Display_Bibliotecas/ssd1306.c
)

# Configuração da comunicação serial
pico_enable_stdio_uart(joystick 1)
pico_enable_stdio_usb(joystick 1) # Ativa USB

# Vincula as bibliotecas
target_link_libraries(joystick PRIVATE
    pico_stdlib
    hardware_adc
    hardware_pwm
    hardware_i2c
    hardware_pio
)

pico_add_extra_outputs(joystick)