#pragma once
#include "data_callback_fn.h"
namespace reinforcement_learning {
  class error_callback_fn;
}

namespace reinforcement_learning { namespace model_management {
  class model_downloader {
  public:
    model_downloader(i_data_transport* ptrans, data_callback_fn* pdata_cb, i_trace* trace);
    model_downloader(model_downloader&& temp) noexcept;
    model_downloader& operator=(model_downloader&& temp) noexcept;

    int run_iteration(api_status* status) const;
  private:
    // Lifetime of pointers managed by user of this class
    i_data_transport* _ptrans = nullptr;
    data_callback_fn* _pdata_cb = nullptr;
    i_trace* _trace;
  };
}}
