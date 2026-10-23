qt_add_executable(tst_ui tst_ui.cpp)
target_link_libraries(tst_ui PRIVATE Qt6::Qml Qt6::Quick Qt6::Test silo_uiplugin)

add_test(NAME tst_ui COMMAND tst_ui)
set_tests_properties(tst_ui PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
