#pragma once

#include <boost/program_options.hpp>

#include <mutex>
#include <thread>

class experiment_controller {
public:
  virtual ~experiment_controller() {}
  void restart();
  void iterate();

  void progress_bar();
  size_t get_iteration() const;

  bool is_running() const;

protected:
  virtual void on_restart() = 0;
  virtual void on_iterate() = 0;

  void stop();

protected:
  bool _is_running{ true };
  size_t _iteration{ 0 };
};

class iterations_experiment_controller : public experiment_controller {
public:
  iterations_experiment_controller(size_t num_iterations);

  virtual void on_restart() override {}
  virtual void on_iterate() override;

private:
  size_t _num_iterations{ 0 };
};

class duration_experiment_controller : public experiment_controller {
public:
  duration_experiment_controller(size_t duration);
  virtual ~duration_experiment_controller() override;

  virtual void on_restart() override;
  virtual void on_iterate() override {}

private:
  void timer();

private:
  size_t _duration{ 0 };
  std::unique_ptr<std::thread> _timer_thread;
};

class experiment_controller_factory {
public:
  static experiment_controller* create(const boost::program_options::variables_map& vm);
};
