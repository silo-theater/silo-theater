qt_add_library(silo_ui STATIC)

qt_add_qml_module(silo_ui
  URI Silo.Ui
  VERSION 1.0
  RESOURCE_PREFIX /qt/qml
  OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/qml/Silo/Ui
  QML_FILES
    Main.qml
)
