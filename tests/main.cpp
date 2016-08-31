#include <QTest>
#include "coordinatestest.h"
#include "drawingtest.h"
#include "renderingtest.h"
#include "valencetest.h"
#include "xmltest.h"
#include "moleculetest.h"

typedef QPair<QString, int> stringIntPair ;

stringIntPair runTest(QObject *testobject)
{
  return qMakePair(QString(testobject->metaObject()->className()),
                   QTest::qExec(testobject, QStringList())) ;
}

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  QVector<stringIntPair> results ;
//  results << runTest(new coordinatesTest());
  results << runTest(new DrawingTest);
  results << runTest(new ValenceTest);
  results << runTest(new XmlTest);
//  results << runTest(new RenderingTest);
  results << runTest(new MoleculeTest);
  qDebug() << "=========== All tests done. ===========" ;
  foreach(const stringIntPair& result, results)
    qDebug() << (result.second ? "!!" : "  ")
             << result.second
             << result.first ;

  return 0 ;
}
