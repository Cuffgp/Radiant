#include <Radiant.h>

#include "EntryPoint.h"

class SandboxApp : public Radiant::Application
{
public:
	SandboxApp()
	{

	}

	~SandboxApp()
	{

	}
private:

};

Radiant::Application* Radiant::CreateApplication()
{
	return new SandboxApp();
}