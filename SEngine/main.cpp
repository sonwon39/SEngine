#include "SimpleApp.h"

int main()
{
	Core::SimpleApp app(720, 720, 0);
	if (app.Initialize())
	{
		return app.Run();
	}

}
