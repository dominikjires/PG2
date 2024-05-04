#include "app.hpp"

App app;

int main()
{
    if (app.Init()) {
        return app.Run();
    }
}
