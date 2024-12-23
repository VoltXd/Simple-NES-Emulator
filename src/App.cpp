#include "App.hpp"

#include <cstdlib>
#include <iostream>
#include <chrono>
#include "NES/NES.hpp"
#include "NES/Cartridge.hpp"
#include "NES/Toolbox.hpp"
#include "IO/GlfwApp.hpp"

#include "IO/SoundManager.hpp"
#include <cmath>

App::App()
{
}

int App::run()
{
	mAppState = AppState::IDLE;
	
	// Init window & sound
	GlfwApp appWindow(mController);
	testAndExitWithMessage(mSoundManager.initialise() == EXIT_FAILURE, "Cannot initialise sound manager...");

	// Infinite loop
	while (!appWindow.shouldWindowClose())
	{
		switch (mAppState)
		{
			case AppState::IDLE:
				processIdleState(appWindow);
				break;

			case AppState::PLAYING:
				playGame(appWindow);
				break;
		}
	}

	return EXIT_SUCCESS;
}

void App::processIdleState(GlfwApp& appWindow)
{
	constexpr picture_t BLANK_SCREEN = { 0 };

	while (!appWindow.shouldWindowClose() && !appWindow.isRomOpened())
		appWindow.draw(BLANK_SCREEN);
	
	mAppState = AppState::PLAYING;
}

void App::playGame(GlfwApp& appWindow)
{
	using std::chrono::steady_clock;
	
	// *************** NES Emulation *************** //
	NES nes(mController, appWindow.getRomName());
	appWindow.clearIsRomOpened();
	linkFifosToWindow(nes, appWindow);

    mTimePrevious = steady_clock::now();

	while (!appWindow.shouldWindowClose() && !appWindow.isRomOpened())
	{
		// Pause
		if (appWindow.isPaused())
		{
			appWindow.draw(nes.getPicture());
			mTimePrevious = steady_clock::now();

			continue;
		}

		// Emulation
		nes.runOneCpuInstruction();

		// Video
		if (nes.isImageReady())
		{
			nes.clearIsImageReady();

			sendPictureToWindow(appWindow, nes.getPicture());
		}

		// Sound
		if (nes.isSoundBufferReady())
		{
			nes.clearIsSoundBufferReady();

			// Push sample to sound stream
			mSoundManager.streamSound(*nes.getSoundBufferPtr());
		}
	}
}

void App::sendPictureToWindow(GlfwApp &appWindow, const picture_t &picture)
{
	using std::chrono::steady_clock;
	
	steady_clock::time_point timeNow;
	double elapsedTime, elapsedTimeOffset = 0;

	// Wait before rendering -> 60 FPS
	do
	{
		timeNow = steady_clock::now();
		elapsedTime = std::chrono::duration<double>(timeNow - mTimePrevious).count();
	} while (elapsedTime + elapsedTimeOffset < FRAME_PERIOD_NTSC);
	mTimePrevious = timeNow;
	elapsedTimeOffset = elapsedTime + elapsedTimeOffset - FRAME_PERIOD_NTSC;

	// Render
	appWindow.draw(picture);
}

void App::linkFifosToWindow(NES &nes, GlfwApp &window)
{
	window.setSoundFIFOPtr(nes.getSoundFIFOPtr());
	window.setP1FIFOPtr(nes.getP1FIFOPtr());
	window.setP2FIFOPtr(nes.getP2FIFOPtr());
	window.setTriangleFIFOPtr(nes.getTriangleFIFOPtr());
	window.setNoiseFIFOPtr(nes.getNoiseFIFOPtr());
	window.setDmcFIFOPtr(nes.getDmcFIFOPtr());
}
