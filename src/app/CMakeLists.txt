qt_add_executable(silo-shell main.cpp)

target_link_libraries(silo-shell PRIVATE Qt6::Gui Qt6::Qml Qt6::Quick silo_uiplugin)

install(TARGETS silo-shell RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
