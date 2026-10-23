/*
 *  Copyright (C) 2026 Silo-theater contributors
 *  SPDX-License-Identifier: AGPL-3.0-or-later
 *  See LICENSE for more information.
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlExtensionPlugin>

Q_IMPORT_QML_PLUGIN(Silo_UiPlugin)

int main(int argc, char* argv[])
{
  QGuiApplication app(argc, argv);
  app.setApplicationName(QStringLiteral("silo-shell"));

  QQmlApplicationEngine engine;
  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
      []() { QCoreApplication::exit(1); }, Qt::QueuedConnection);
  engine.load(QUrl(QStringLiteral("qrc:/qt/qml/Silo/Ui/Main.qml")));

  return app.exec();
}
