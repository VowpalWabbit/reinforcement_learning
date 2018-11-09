#include "experiment_controller.h"
#include <iostream>

void throw_if_conflicting(const boost::program_options::variables_map& vm, const std::string first, const std::string& second) {
  if (vm.count(first) && !vm[first].defaulted() &&
    vm.count(second) && !vm[second].defaulted())
  {
    throw std::logic_error(std::string("Conflicting options '") +
      first + "' and '" + second + "'.");
  }
}
void experiment_controller::restart()
{
  _iteration = 0;
  _is_running = true;
  on_restart();
}

void experiment_controller::iterate() {
  ++_iteration;
  on_iterate();
}

bool experiment_controller::is_running() const {
  return _is_running;
}

size_t experiment_controller::get_iteration() const {
  return _iteration;
}

void experiment_controller::stop() {
  _is_running = false;
}

void experiment_controller::progress_bar() {
  if (_iteration % 100 == 0) std::cout << "\r" << _iteration << " sent";
}

iterations_experiment_controller::iterations_experiment_controller(size_t num_iterations)
  : _num_iterations(num_iterations)
{ }

void iterations_experiment_controller::on_iterate()
{
  _is_running = _iteration < _num_iterations;
}

duration_experiment_controller::duration_experiment_controller(size_t duration)
  : _duration(duration)
{ }

duration_experiment_controller::~duration_experiment_controller()
{
  if (_timer_thread) _timer_thread->join();
}

void duration_experiment_controller::timer() {
  std::this_thread::sleep_for(std::chrono::milliseconds(_duration));
  stop();
}

void duration_experiment_controller::on_restart()
{
  if (_timer_thread) _timer_thread->join();
  _timer_thread.reset(new std::thread(&duration_experiment_controller::timer, this));
}

experiment_controller* experiment_controller_factory::create(const boost::program_options::variables_map& vm) {
  throw_if_conflicting(vm, "examples", "duration");
  if (vm.count("duration")) {
    return new duration_experiment_controller(vm["duration"].as<size_t>());
  }
  return new iterations_experiment_controller(vm["examples"].as<size_t>());
}