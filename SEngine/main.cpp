#include "SimpleApp.h"

int main()
{
    Core::SimpleApp app(1024, 1024, 0);
    if (app.Initialize())
    {
        return app.Run();
    }
}
