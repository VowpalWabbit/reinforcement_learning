%module(directors="1") rl_client

%{
#define SWIG_FILE_WITH_INIT
#include "py_api.h"
#include "../../include/constants.h"
#include "../../include/err_constants.h"
#include "configuration.h"
#include <map>

static std::map<int, PyObject*> rl_exception_types;

// Initialized in init
static PyObject* rl_exception;
static PyObject* rl_exception_type_dictionary;

// Creates a new Python exception type that inherits from the base rl_exception type and saves it to the rl_exception_types map.
// This map is used when an error code is returned by the C++ API to convert it to the proper Python exception.
#define ERROR_CODE_DEFINITION(code, name, message) \
{ \
  PyObject* new_exception_type = PyErr_NewException("_rl_client." # name "_exception", rl_exception, NULL); \
  Py_INCREF(new_exception_type); \
  PyModule_AddObject(m, "name ## _exception", new_exception_type); \
  PyDict_SetItemString(rl_exception_type_dictionary, #name "_exception", new_exception_type); \
  rl_exception_types[code] = new_exception_type; \
}
%}

%init %{
  // Create a Python dictionary to surface exceptioin types in and add to the module.
  rl_exception_type_dictionary = PyDict_New();
  Py_INCREF(rl_exception_type_dictionary);
  PyModule_AddObject(m, "rl_exception_type_dictionary", rl_exception_type_dictionary);

  // Create a base exception type that inherits from Exception
  rl_exception = PyErr_NewException("_rl_client.rl_exception", NULL, NULL);
  Py_INCREF(rl_exception);
  PyModule_AddObject(m, "rl_exception", rl_exception);

  PyDict_SetItemString(rl_exception_type_dictionary, "rl_exception", rl_exception);

  // Uses ERROR_CODE_DEFINITION defined above to initialize an exception type for every defined error.
  #include "../../include/errors_data.h"
%}

%include <exception.i>
%include <std_string.i>
%include <std_vector.i>

%template(vectori) std::vector<int>;
%template(vectorf) std::vector<float>;

%feature("director") reinforcement_learning::python::error_callback;

%exception {
  try {
    $action
  } catch(reinforcement_learning::python::rl_exception_internal& e) {
    PyObject* exception_object = rl_exception;
    // Lookup the exception type based off the error code.
    if(rl_exception_types.find(e.error_code()) != rl_exception_types.end()) {
      exception_object = rl_exception_types[e.error_code()];
    }
    PyErr_SetString(exception_object, const_cast<char*>(e.what()));
    SWIG_fail;
  }
}

%include "py_api.h"
%include "../../include/constants.h"
%include "../../include/configuration.h"

%feature("pythonprepend") reinforcement_learning::python::live_model::live_model %{
    # If the overloaded version with a callback is called stash a reference to it so that the object is not deleted prematurely.
    if len(args) == 2:
        self.callback = args[1]
%}

namespace reinforcement_learning {
  namespace python {

    class live_model {
    public:
      live_model(const reinforcement_learning::utility::configuration config, error_callback& callback);
      live_model(const reinforcement_learning::utility::configuration config);

      void init();

      %rename(choose_rank_impl) choose_rank;
      reinforcement_learning::python::ranking_response choose_rank(const char* event_id, const char* context_json, bool deferred);
      // event_id is auto-generated.
      reinforcement_learning::python::ranking_response choose_rank(const char* context_json, bool deferred);

      void report_action_taken(const char* event_id);

      void report_outcome(const char* event_id, const char* outcome);
      void report_outcome(const char* event_id, float outcome);

      void refresh_model();

      %pythoncode %{
        def choose_rank(self, context, event_id = None, deferred = False):
            if event_id == None:
                ranking_response = self.choose_rank_impl(context, deferred)
                return ranking_response.model_id, ranking_response.chosen_action_id, list(zip(ranking_response.action_ids, ranking_response.probabilities)), ranking_response.event_id

            ranking_response = self.choose_rank_impl(event_id, context, deferred)
            return ranking_response.model_id, ranking_response.chosen_action_id, list(zip(ranking_response.action_ids, ranking_response.probabilities))
      %}
    };
  }
}
