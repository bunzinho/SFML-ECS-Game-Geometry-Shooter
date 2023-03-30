#pragma once
#include <chrono>

class Time
{
	double t = 0.0;
	const double dt = 1.0/30.0;

	double current_time = time_in_seconds();
	double accumulator = 0.0;

public:
	double get_starting_physics_time()
	{
		return current_time;
	}
	double delta_time()
	{
		return dt;
	}

	bool is_tick_ready()
	{
		return accumulator >= dt;
	}

	void update_accumulator()
	{
		accumulator -= dt;
		t += dt;
	}

	void update_delta_time()
	{
		double new_time = time_in_seconds();
		double frame_time = new_time - current_time;
		current_time = new_time;
		accumulator += frame_time;
	}

	double time_in_seconds()
	{
		typedef std::chrono::high_resolution_clock hires_clock;
		using namespace std::chrono;
		return 0.001 * duration_cast<milliseconds>(hires_clock::now().time_since_epoch()).count();
	}

	double alpha()
	{
		return accumulator / dt;
	}
};
