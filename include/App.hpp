#pragma once

#include <string>
#include <chrono>

#include "NES/NES.hpp"
#include "NES/Controller.hpp"
#include "IO/GlfwApp.hpp"
#include "IO/SoundManager.hpp"

class App
{
public:
	App();

	int run();

private:
	enum class AppState
	{
		IDLE,
		ERROR,
		PLAYING
	} mAppState;
	
	void processIdleState(GlfwApp& appWindow);
	void showErrorWindow(GlfwApp& appWindow);
	void playGame(GlfwApp& appWindow);
	void sendPictureToWindow(GlfwApp& appWindow, const picture_t& picture);
	void linkFifosToWindow(NES& nes, GlfwApp& window);

	Controller mController1;
	Controller mController2;
	SoundManager mSoundManager;

	std::chrono::steady_clock::time_point mTimePrevious;
	double mElapsedTimeOffset;
};
