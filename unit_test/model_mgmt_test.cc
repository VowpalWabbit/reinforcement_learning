#include "model_mgmt_test.h"

m::model_data get_model_data();

void register_local_file_factory();
const char * const DUMMY_DATA_TRANSPORT = "DUMMY_DATA_TRANSPORT";
const char * const CFG_PARAM = "model.local.file";

BOOST_AUTO_TEST_CASE(data_transport_user_extention)
{
  register_local_file_factory();
  const u::configuration cc;

  m::i_data_transport* data_transport;
  auto scode = r::data_transport_factory.create(&data_transport, DUMMY_DATA_TRANSPORT, cc);
  BOOST_CHECK_EQUAL(scode, r::error_code::success);
  m::model_data md;
  scode = data_transport->get_data(md);
  BOOST_CHECK_EQUAL(scode, r::error_code::success);
  md.free();
  delete data_transport;
}

BOOST_AUTO_TEST_CASE(vw_model_factory)
{
  register_local_file_factory();

  u::configuration model_cc;
  model_cc.set(r::name::VW_CMDLINE, "--lda 5");
  m::i_model* vw;
  const auto scode = r::model_factory.create(&vw, r::value::VW, model_cc);
  BOOST_CHECK_EQUAL(scode, r::error_code::success);
  delete vw;
}

m::model_data get_model_data()
{
  const u::configuration cc;
  m::i_data_transport* data_transport;
  r::data_transport_factory.create(&data_transport, DUMMY_DATA_TRANSPORT, cc);
  std::unique_ptr<m::i_data_transport> pdt(data_transport);
  m::model_data md;
  const auto scode = pdt->get_data(md);
  BOOST_CHECK_EQUAL(scode, r::error_code::success);
  return md;
}

class dummy_data_transport : public m::i_data_transport {
  int get_data(m::model_data& data, r::api_status* status) override {
    data.alloc(10);
    return r::error_code::success;
  }
};

int dummy_data_tranport_create( m::i_data_transport** retval,
                                    const u::configuration& config,
                                    r::i_trace* trace,
                                    r::api_status* status) {
  *retval = new dummy_data_transport();
  return r::error_code::success;
}

void register_local_file_factory() {
  r::data_transport_factory.register_type(DUMMY_DATA_TRANSPORT, dummy_data_tranport_create);
}


BOOST_AUTO_TEST_CASE(vw_model_type)
{
  register_local_file_factory();

  u::configuration model_cc;
  m::i_model* vw;

  model_cc.set(r::name::VW_CMDLINE, "--cb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A");
  BOOST_CHECK_EQUAL(r::error_code::success, r::model_factory.create(&vw, r::value::VW, model_cc));
  BOOST_CHECK_EQUAL((int)m::model_type_t::CB, (int)vw->model_type());
  delete vw;

  model_cc.set(r::name::VW_CMDLINE, "--ccb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A");
  model_cc.set(r::name::MODEL_VW_INITIAL_COMMAND_LINE, "--ccb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A");
  BOOST_CHECK_EQUAL(r::error_code::success, r::model_factory.create(&vw, r::value::VW, model_cc));
  BOOST_CHECK_EQUAL((int)m::model_type_t::CCB, (int)vw->model_type());
  delete vw;

  model_cc.set(r::name::VW_CMDLINE, "--slates --ccb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A");
  model_cc.set(r::name::MODEL_VW_INITIAL_COMMAND_LINE, "--slates --ccb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A");
  BOOST_CHECK_EQUAL(r::error_code::success, r::model_factory.create(&vw, r::value::VW, model_cc));
  BOOST_CHECK_EQUAL((int)m::model_type_t::SLATES, (int)vw->model_type());
  delete vw;
}
