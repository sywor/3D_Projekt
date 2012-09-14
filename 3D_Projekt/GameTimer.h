#pragma once

class GameTimer
{
public:

	GameTimer();
	float getGameTime() const;
	float getDeltaTime() const;

	void reset(); // Call before message loop.
	void start(); // Call when unpaused.
	void stop();  // Call when paused.
	void tick();  // Call every frame.

private:

	double secondsPerCount;
	double deltaTime;

	__int64 baseTime;
	__int64 pausedTime;
	__int64 stopTime;
	__int64 prevTime;
	__int64 currTime;

	bool stopped;
};