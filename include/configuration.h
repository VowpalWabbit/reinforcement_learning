/**
 * @brief configuration definition.  configuration is a (name,value) pair based collection used to configure the API
 *
 * @file configuration.h
 * @author Rajan Chari et el
 * @date 2018-07-18
 */
#pragma once
#include <string>
#include <unordered_map>

namespace reinforcement_learning
{
namespace utility
{
class configuration;
}
}  // namespace reinforcement_learning

std::ostream& operator<<(std::ostream& os, const reinforcement_learning::utility::configuration&);

namespace reinforcement_learning
{
namespace utility
{
/**
 * @brief Configuration class to initialize the API
 * Represents a collection of (name,value) pairs used to configure the API
 */
class configuration
{
public:
  configuration() = default;
  ~configuration() = default;
  //! Copy constructor
  configuration(const configuration&) = default;
  //! Assignment operator
  configuration& operator=(const configuration&) = default;
  //! Move constructor
  configuration& operator=(configuration&&) = default;
  //! Move assignment operator
  configuration(configuration&&) = default;

  //! Sets the value for a given name.  It overrides any existing values for that name
  void set(const char* name, const char* value);
  //! Gets the value for a given name.  If the value does not exist, it returns defval
  const char* get(const char* name, const char* defval) const;
  //! Gets the value as an integer.  If the value does not exist or if there is an error, it returns defval
  int get_int(const char* name, int defval) const;
  //! Gets the value as a boolean.  If the value does not exist or if there is an error, it returns defval
  bool get_bool(const char* name, bool defval) const;
  //! Gets the value as a float.  If the value does not exist or if there is an error, it returns defval
  float get_float(const char* name, float defval) const;
  //! friend Left shift operator
  friend std::ostream& ::operator<<(std::ostream& os, const reinforcement_learning::utility::configuration&);

  //! Same as get_bool, but use global value as fallback before defval. Sections names are dot delimited. IE:
  //! `section.property`
  bool get_bool(const char* section, const char* name, bool defval) const;

private:
  using map_type = std::unordered_map<std::string, std::string>;  //! Collection type that holds the (name,value) pairs
  map_type _pmap;                                                 //! Collection that holds the (name,value) pairs
};
}  // namespace utility
}  // namespace reinforcement_learning
