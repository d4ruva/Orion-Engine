#include "OApplication.h"
#include "OLog.h"

int main() {
    Orion::OLog::Init();

    Orion::OApplication app;
    app.Run();

    return 0;
}
