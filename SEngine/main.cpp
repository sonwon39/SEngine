#include "SimpleApp.h"

int main()
{
	Core::SimpleApp app(512, 512, 0);
	if (app.Initialize())
	{
		return app.Run();
	}

}
