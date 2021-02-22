#include "vw_example_pool.h"

namespace reinforcement_learning {

vw_example_pool::vw_example_pool() : _vw(nullptr) {}

vw_example_pool::vw_example_pool(vw *vw) : _vw(vw) {}
vw_example_pool::~vw_example_pool() {

  // cleanup examples
  for (auto &&ex : _example_pool) {
    VW::dealloc_example(_vw->example_parser->lbl_parser.delete_label, *ex);
    ::free_it(ex);
  }

  // cleanup VW instance
  reset_source(*_vw, _vw->num_bits);

  VW::finish(*_vw);
}

vw *vw_example_pool::get_vw() const { return _vw; }

void vw_example_pool::set_vw(vw *vw) {
  if (_vw != nullptr) {
    // cleanup VW instance
    reset_source(*_vw, _vw->num_bits);

    VW::finish(*_vw);
  }
  _vw = vw;
}

example *vw_example_pool::get_or_create_example() {
  // alloc new element if we don't have any left
  if (_example_pool.size() == 0) {
    auto ex = VW::alloc_examples(0, 1);
    _vw->example_parser->lbl_parser.default_label(&ex->l);

    return ex;
  }

  // get last element
  example *ex = _example_pool.back();
  _example_pool.pop_back();

  VW::empty_example(*_vw, *ex);
  _vw->example_parser->lbl_parser.default_label(&ex->l);

  return ex;
}

void vw_example_pool::return_example(example *ex) {
  _example_pool.emplace_back(ex);
}

} // namespace reinforcement_learning