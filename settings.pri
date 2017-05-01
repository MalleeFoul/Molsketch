MSK_SOURCE_BASE = $$PWD
QT += widgets printsupport svg
CONFIG += silent c++14
lessThan(QT_MAJOR_VERSION,5): QMAKE_CXXFLAGS += -std=c++11

qtVersionSuffix=""
equals(QT_MAJOR_VERSION, 5) {
    qtVersionSuffix= -qt5
}
DEFINES += QMAKEBUILD \
        "QTVERSIONSUFFIX=\"\\\"$$qtVersionSuffix\\\"\""

buildVars = $$cat(buildvariables)
for(line, $$list($$split(buildVars, $$escape_expand(\\n)))) {
  name = $$section(line, ;, 1, 1)
  !exists($$OUT_PWD/../buildvars/$$name) : error("Could not find file with value of $$name")
  $$name = $$cat($$OUT_PWD/../buildvars/$$name)
  !equals($$name, false) : DEFINES += "$${name}=\"\\\"$$eval($$name)\\\"\""
}

VERSION = $$cat(version)
CONFIG(static) : DEFINES += QT_STATIC_BUILD
