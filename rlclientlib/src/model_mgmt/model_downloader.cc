#include "model_downloader.h"
#include "api_status.h"

namespace reinforcement_learning { namespace model_management {
  model_downloader::model_downloader(i_data_transport* ptrans, data_callback_fn* pdata_cb)
    : _ptrans(ptrans), _pdata_cb(pdata_cb){}

  model_downloader::model_downloader(model_downloader&& temp) noexcept {
    _ptrans = temp._ptrans;
    temp._ptrans = nullptr;
    _pdata_cb = temp._pdata_cb;
    temp._pdata_cb = nullptr;
  }

  model_downloader& model_downloader::operator=(model_downloader&& temp) noexcept {
    if (&temp != this) {
      _ptrans = temp._ptrans;
      temp._ptrans = nullptr;
      _pdata_cb = temp._pdata_cb;
      temp._pdata_cb = nullptr;
    }
    return *this;
  }

  int model_downloader::run_iteration(api_status* status) const {
    model_data md;
    RETURN_IF_FAIL(_ptrans->get_data(md, status));

    // If the data size is zero, it's not a valid model
    if (md.data_sz() <= 0) {
      return error_code::success;
    }

    const auto scode = _pdata_cb->report_data(md, status);
    return scode;
  }
}}
