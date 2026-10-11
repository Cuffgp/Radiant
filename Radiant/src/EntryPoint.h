#pragma once

#include "VulkanDevice.h"

extern Radiant::Application* Radiant::CreateApplication();

int main(int argc, char** argv)
{
	Radiant::VulkanDevice::Init();

	auto app = Radiant::CreateApplication();
	app->Run();
	delete app;

	Radiant::VulkanDevice::Destroy();

}