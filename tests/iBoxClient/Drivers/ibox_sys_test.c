#include "unity.h"
#include "unity_fixture.h"
#include "ibox_board.h"
#include "unity_config.h"

TEST_GROUP(ibox_sys);

TEST_SETUP(ibox_sys)
{
  //This is run before EACH TEST
  printf("ibox_sys testing....\r\n");
}

TEST_TEAR_DOWN(ibox_sys)
{
    printf("ibox_sys test end....\r\n");
}

TEST(ibox_sys, num_add_function_bianjie)
{
  //All of these should pass
  TEST_ASSERT_EQUAL(0, num_add_function(0,0));
  TEST_ASSERT_EQUAL(100, num_add_function(0,100));
  TEST_ASSERT_EQUAL(100, num_add_function(100,0));
  TEST_ASSERT_EQUAL(200, num_add_function(100,100));
  TEST_ASSERT_EQUAL(99, num_add_function(-1,100));
}

TEST(ibox_sys, num_add_function_yichu)
{
  // You should see this line fail in your test summary
  TEST_ASSERT_EQUAL(1, num_add_function(9999999,999999999));

}
