# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
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
# ====================================================================================

cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PICO_BOARD pico CACHE STRING "Board type")

include(pico_sdk_import.cmake)
project(Main_Monitoramento_Temperatura_Incendio C CXX ASM)

pico_sdk_init()

add_executable(${PROJECT_NAME}
    ${PROJECT_NAME}.c
        lib/ssd1306.c
      
)


# Generate PIO header
pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/ws2818b.pio)

pico_set_program_version(${PROJECT_NAME} "0.1")
pico_enable_stdio_uart(${PROJECT_NAME} 1)
pico_enable_stdio_usb(${PROJECT_NAME} 1)

# Link com as bibliotecas necessárias
target_link_libraries(${PROJECT_NAME} 
pico_stdlib 
hardware_pio # para matriz de leds
hardware_clocks # para matriz de leds
hardware_i2c # para comuniccao do display
hardware_adc # para o njoystick
hardware_pwm # para o leds RGB
hardware_gpio # PARA AS ENTRADAS GPIO
pico_bootsel_via_double_reset # PARA COLOCAR A PLACA NO MODO DE GRAVACAO
pico_bootrom # PARA COLOCAR A PLACA NO MODO DE GRAVACAO
)

# Adicione o diretório atual aos caminhos de inclusão
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR})

pico_add_extra_outputs(${PROJECT_NAME})

