#include <QQmlApplicationEngine>
#include <QQmlExtensionPlugin>
#include <QQuickWindow>
#include <QTest>

Q_IMPORT_QML_PLUGIN(Silo_UiPlugin)

class TestUi : public QObject
{
  Q_OBJECT

private slots:
  void mainWindowLoads()
  {
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/qt/qml/Silo/Ui/Main.qml")));
    QCOMPARE(engine.rootObjects().size(), 1);
    QVERIFY(qobject_cast<QQuickWindow*>(engine.rootObjects().first()));
  }
};

QTEST_MAIN(TestUi)
#include "tst_ui.moc"
