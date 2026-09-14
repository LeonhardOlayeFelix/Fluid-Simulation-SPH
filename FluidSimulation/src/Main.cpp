#include "app/Application.h";


void processInput(GLFWwindow* window);


int main()
{
    ApplicationSpec appSpec;
    appSpec.Title = "Fluid Simulation";
    appSpec.WinSpec.Width = 800;
    appSpec.WinSpec.Height = 600;

    Application app(appSpec);

    app.Run();
}

