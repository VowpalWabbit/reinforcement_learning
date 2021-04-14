#include "lru_dedup_cache.h"
#include "test_common.h"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(test_lru_add_new_examples_to_cache) {
  auto vw = VW::initialize("--cb_explore_adf --binary_parser --quiet", nullptr,
                           false, nullptr, nullptr);

  v_array<example *> examples;
  examples.push_back(&VW::get_unused_example(vw));
  // create a dummy examples to be cached
  examples[0]->indices.push_back('A');
  examples[0]->feature_space['A'].indicies.push_back(1);
  examples[0]->feature_space['A'].values.push_back(0.2f);
  uint64_t dedup_id_0 = 0;

  examples.push_back(&VW::get_unused_example(vw));
  examples[1]->indices.push_back('B');
  examples[1]->feature_space['B'].indicies.push_back(2);
  examples[1]->feature_space['B'].values.push_back(0.5f);
  uint64_t dedup_id_1 = 1;

  lru_dedup_cache dedup_cache;

  BOOST_CHECK_EQUAL(dedup_cache.exists(dedup_id_0), false);
  dedup_cache.add(dedup_id_0, examples[0]);
  BOOST_CHECK_EQUAL(dedup_cache.exists(dedup_id_0), true);

  // check deleting after this example does nothing
  dedup_cache.clear_after(dedup_id_0);
  BOOST_CHECK_EQUAL(dedup_cache.exists(dedup_id_0), true);

  // add second another example
  BOOST_CHECK_EQUAL(dedup_cache.exists(dedup_id_1), false);
  dedup_cache.add(dedup_id_1, examples[1]);
  BOOST_CHECK_EQUAL(dedup_cache.exists(dedup_id_1), true);
  // first example still there
  BOOST_CHECK_EQUAL(dedup_cache.exists(dedup_id_0), true);

  // check examples are OK
  BOOST_CHECK_EQUAL(dedup_cache.dedup_examples.size(), 2);
  auto *ex_0 = dedup_cache.dedup_examples[dedup_id_0];
  BOOST_CHECK_EQUAL(ex_0->indices.size(), 1);
  BOOST_CHECK_EQUAL(ex_0->indices[0], 'A');
  BOOST_CHECK_EQUAL(ex_0->feature_space['A'].indicies.size(), 1);
  BOOST_CHECK_EQUAL(ex_0->feature_space['A'].indicies[0], 1);
  BOOST_CHECK_EQUAL(ex_0->feature_space['A'].values.size(), 1);
  BOOST_CHECK_EQUAL(ex_0->feature_space['A'].values[0], 0.2f);

  auto *ex_1 = dedup_cache.dedup_examples[dedup_id_1];
  BOOST_CHECK_EQUAL(ex_1->indices.size(), 1);
  BOOST_CHECK_EQUAL(ex_1->indices[0], 'B');
  BOOST_CHECK_EQUAL(ex_1->feature_space['B'].indicies.size(), 1);
  BOOST_CHECK_EQUAL(ex_1->feature_space['B'].indicies[0], 2);
  BOOST_CHECK_EQUAL(ex_1->feature_space['B'].values.size(), 1);
  BOOST_CHECK_EQUAL(ex_1->feature_space['B'].values[0], 0.5f);
  // done checking examples

  // remove anything after dedup_id_1 should remove dedup_id_0
  dedup_cache.clear_after(dedup_id_1);
  BOOST_CHECK_EQUAL(dedup_cache.exists(dedup_id_0), false);
  BOOST_CHECK_EQUAL(dedup_cache.exists(dedup_id_1), true);

  clear_examples(examples, vw);
  VW::finish(*vw);
}

BOOST_AUTO_TEST_CASE(test_lru_update) {
  auto vw = VW::initialize("--cb_explore_adf --binary_parser --quiet", nullptr,
                           false, nullptr, nullptr);

  v_array<example *> examples;
  examples.push_back(&VW::get_unused_example(vw));

  // create a dummy examples to be cached
  examples[0]->indices.push_back('A');
  examples[0]->feature_space['A'].indicies.push_back(1);
  examples[0]->feature_space['A'].values.push_back(0.2f);
  uint64_t dedup_id_0 = 0;

  examples.push_back(&VW::get_unused_example(vw));
  examples[1]->indices.push_back('B');
  examples[1]->feature_space['B'].indicies.push_back(2);
  examples[1]->feature_space['B'].values.push_back(0.5f);
  uint64_t dedup_id_1 = 1;

  lru_dedup_cache dedup_cache;

  dedup_cache.add(dedup_id_0, examples[0]);
  dedup_cache.add(dedup_id_1, examples[1]);

  // add dedup_0 again should bump it up
  dedup_cache.update(dedup_id_0);
  // order is dedup_id_0, dedup_id_1, clearning after dedup_id_1 should not do
  // anything
  dedup_cache.clear_after(dedup_id_1);

  BOOST_CHECK_EQUAL(dedup_cache.exists(dedup_id_0), true);
  BOOST_CHECK_EQUAL(dedup_cache.exists(dedup_id_1), true);

  // check examples are OK
  BOOST_CHECK_EQUAL(dedup_cache.dedup_examples.size(), 2);
  auto *ex_0 = dedup_cache.dedup_examples[dedup_id_0];
  BOOST_CHECK_EQUAL(ex_0->indices.size(), 1);
  BOOST_CHECK_EQUAL(ex_0->indices[0], 'A');
  BOOST_CHECK_EQUAL(ex_0->feature_space['A'].indicies.size(), 1);
  BOOST_CHECK_EQUAL(ex_0->feature_space['A'].indicies[0], 1);
  BOOST_CHECK_EQUAL(ex_0->feature_space['A'].values.size(), 1);
  BOOST_CHECK_EQUAL(ex_0->feature_space['A'].values[0], 0.2f);

  auto *ex_1 = dedup_cache.dedup_examples[dedup_id_1];
  BOOST_CHECK_EQUAL(ex_1->indices.size(), 1);
  BOOST_CHECK_EQUAL(ex_1->indices[0], 'B');
  BOOST_CHECK_EQUAL(ex_1->feature_space['B'].indicies.size(), 1);
  BOOST_CHECK_EQUAL(ex_1->feature_space['B'].indicies[0], 2);
  BOOST_CHECK_EQUAL(ex_1->feature_space['B'].values.size(), 1);
  BOOST_CHECK_EQUAL(ex_1->feature_space['B'].values[0], 0.5f);
  // done checking examples

  // clearing after dedup_id_0 should remove dedup_id_1
  dedup_cache.clear_after(dedup_id_0);

  BOOST_CHECK_EQUAL(dedup_cache.exists(dedup_id_0), true);
  BOOST_CHECK_EQUAL(dedup_cache.exists(dedup_id_1), false);

  clear_examples(examples, vw);
  VW::finish(*vw);
}