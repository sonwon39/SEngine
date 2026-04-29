#include "SimpleApp.h"

int main()
{
	Core::SimpleApp app(1280, 720, 0);
	if (app.Initialize())
	{
		return app.Run();
	}

}