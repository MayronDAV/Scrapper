#include "Core/Base.h"
#include "Core/Search.h"
#include "Core/Application.h"



int main()
{
    auto app = new SCPY::Application();
    app->Run();
    delete app;
}