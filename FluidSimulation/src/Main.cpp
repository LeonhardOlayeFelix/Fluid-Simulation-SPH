#include "app/Application.h";


void processInput(GLFWwindow* window);


int main()
{
    ApplicationSpec appSpec;
    appSpec.Title = "Fluid Simulation";
    appSpec.WinSpec.Width = 1500;
    appSpec.WinSpec.Height = 900;

    Application app(appSpec);

    app.Run();
}

