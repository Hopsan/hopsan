#include "BuiltinTests.h"

#include "HcomTest.hpp"

#include "global.h"
#include "ModelHandler.h"
#include <QObject>

int runBuiltInTests() {

    HComTest hcomtest{};
    int rc = QTest::qExec(&hcomtest);
    return rc;
}
