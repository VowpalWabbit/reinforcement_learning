#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#  define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>

#include "utility/versioned_object_pool.h"

using namespace reinforcement_learning;
using namespace reinforcement_learning::utility;
using namespace std;

class my_object
{
public:
  int _id;

  my_object(int id) : _id(id) {}
};

class my_object_factory
{
public:
  int _count;

  my_object_factory() : _count(0) {}

  my_object* operator()() { return new my_object(_count++); }
};

BOOST_AUTO_TEST_CASE(object_pool_nothing)
{
  // pool owns a copy of factory
  my_object_factory factory;
  versioned_object_pool<my_object> pool(factory);

  // get pointer to pool's copy of factory
  auto pool_factory = pool.get_factory_function().target<my_object_factory>();
  BOOST_CHECK_NE(pool_factory, nullptr);
  BOOST_CHECK_EQUAL(pool_factory->_count, 0);
}

BOOST_AUTO_TEST_CASE(object_pool_get_same_object)
{
  // pool owns a copy of factory
  my_object_factory factory;
  versioned_object_pool<my_object> pool(factory);

  {
    auto obj = pool.get_or_create();
    BOOST_CHECK_EQUAL(obj->_id, 0);
  }

  // let's make sure we get the same object back
  {
    auto obj = pool.get_or_create();
    BOOST_CHECK_EQUAL(obj->_id, 0);
  }

  // pool's copy of factory should have incremented count
  auto pool_factory = pool.get_factory_function().target<my_object_factory>();
  BOOST_CHECK_NE(pool_factory, nullptr);
  BOOST_CHECK_EQUAL(pool_factory->_count, 1);

  // original factory should be un-modified
  BOOST_CHECK_EQUAL(factory._count, 0);
}

BOOST_AUTO_TEST_CASE(object_pool_get_2_objects)
{
  my_object_factory factory;
  versioned_object_pool<my_object> pool(factory);

  {
    auto obj1 = pool.get_or_create();
    BOOST_CHECK_EQUAL(obj1->_id, 0);

    auto obj2 = pool.get_or_create();
    BOOST_CHECK_EQUAL(obj2->_id, 1);
  }

  auto pool_factory = pool.get_factory_function().target<my_object_factory>();
  BOOST_CHECK_NE(pool_factory, nullptr);
  BOOST_CHECK_EQUAL(pool_factory->_count, 2);
}

BOOST_AUTO_TEST_CASE(object_pool_update_factory)
{
  my_object_factory factory;
  versioned_object_pool<my_object> pool(factory);

  auto obj1 = pool.get_or_create();
  BOOST_CHECK_EQUAL(obj1->_id, 0);

  // update factory -> this resets the counter
  pool.update_factory(my_object_factory());

  auto obj2 = pool.get_or_create();
  BOOST_CHECK_EQUAL(obj2->_id, 0);  // _id is 0 as this is created from the new factory
}

BOOST_AUTO_TEST_CASE(object_pool_init_size_test)
{
  my_object_factory factory;
  versioned_object_pool<my_object> pool(factory, 2);

  // pool should be initialized with 2 objects
  auto pool_factory = pool.get_factory_function().target<my_object_factory>();
  BOOST_CHECK_NE(pool_factory, nullptr);
  BOOST_CHECK_EQUAL(pool_factory->_count, 2);

  // updating factory preserves pool count
  my_object_factory new_factory;
  pool.update_factory(new_factory);
  auto pool_factory_new = pool.get_factory_function().target<my_object_factory>();
  BOOST_CHECK_NE(pool_factory_new, nullptr);
  BOOST_CHECK_EQUAL(pool_factory_new->_count, 2);

  {
    // get 2 objects
    auto obj1 = pool.get_or_create();
    BOOST_CHECK_EQUAL(obj1->_id, 1);
    BOOST_CHECK_EQUAL(pool_factory_new->_count, 2);

    auto obj2 = pool.get_or_create();
    BOOST_CHECK_EQUAL(obj2->_id, 0);
    BOOST_CHECK_EQUAL(pool_factory_new->_count, 2);
    // obj1 and obj2 returned to pool
  }

  // get 2 objects
  auto obj1 = pool.get_or_create();
  BOOST_CHECK_EQUAL(obj1->_id, 1);
  BOOST_CHECK_EQUAL(pool_factory_new->_count, 2);

  auto obj2 = pool.get_or_create();
  BOOST_CHECK_EQUAL(obj2->_id, 0);
  BOOST_CHECK_EQUAL(pool_factory_new->_count, 2);

  // get 3rd object
  // this should result in a new factory creation
  auto obj3 = pool.get_or_create();
  BOOST_CHECK_EQUAL(obj3->_id, 2);
  BOOST_CHECK_EQUAL(pool_factory_new->_count, 3);
}