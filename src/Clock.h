#pragma once
#include <chrono>

class Clock
{
	typedef std::chrono::high_resolution_clock hires_clock;

private:

	hires_clock::time_point initial_time = hires_clock::now();
	double t = 0.0;
	const double dt = 1.0/30;
	double frame_time = 0.0;
	double time_scale = 1.0;

	bool paused = false;
	double current_time = time_in_seconds();
	double accumulator = 0.0;

public:

	double get_frame_time() const;

	void set_paused(bool pause);

	bool is_paused() const;

	double get_game_time() const;

	double get_starting_physics_time() const;

	double delta_time() const;

	bool is_tick_ready() const;

	void update_accumulator();

	void update_time();

	void update_delta_time();

	double time_in_seconds() const;

	double alpha() const;

	double add_time_scale(double);
};
