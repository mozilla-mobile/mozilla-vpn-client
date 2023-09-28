
#include <QTest>

class TestClient : public QObject {
  Q_OBJECT
 private slots:
  void toUpper();
};
QTEST_MAIN(TestClient)
#include "testclient.moc"

void TestClient::toUpper() {
  QString str = "Hello";
  QVERIFY(str.toUpper() == "HELLO");
}
