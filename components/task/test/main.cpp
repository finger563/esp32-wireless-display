// required for TEST_CASE & TEST_ASSERT* etc.
#include "unity.h"
// required for unity_wait_for_signal etc.
#include "test_utils.hpp"
// header for code under test
#include "task.hpp"

#include <functional>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

int basic_callback_executions = 0;
void basic_callback(void) {
  printf("[%d] basic callback\n", basic_callback_executions++);
  // delay in the task (otherwise it will consume the processor)
  std::this_thread::sleep_for( 500ms );
}

class Foo {
public:
  int bar = 0;
  static int foobar;
  static void static_function() {
    printf("[%d] Foo::static_function callback\n", foobar++);
    // delay in the task (otherwise it will consume the processor)
    std::this_thread::sleep_for( 500ms );
  }
  void member_function() {
    printf("[%d] Foo::member_function callback\n", bar++);
    // delay in the task (otherwise it will consume the processor)
    std::this_thread::sleep_for( 500ms );
  }
};
int Foo::foobar = 0;

TEST_CASE("basic callback test", "[TASK]")
{
  int starting_executions = basic_callback_executions;
  auto task = phoenix::Task::make_unique();
  auto task_config =
    phoenix::Task::Config{.name = "single task",
                          .stack_size_bytes = 4096,
                          .priority = 5,
                          .on_task_callback = basic_callback};
  // start the task
  TEST_ASSERT(task->start(task_config));
  // delay for a while
  std::this_thread::sleep_for(1s);
  // assert the task was started
  TEST_ASSERT(task->started());
  // assert the task actually ran
  TEST_ASSERT_GREATER_THAN(starting_executions, basic_callback_executions);
};

TEST_CASE("default stack size test", "[TASK]")
{
  int starting_executions = basic_callback_executions;
  auto task = phoenix::Task::make_unique();
  auto task_config =
    phoenix::Task::Config{.name = "single task",
                          .priority = 5,
                          .on_task_callback = basic_callback};
  // start the task
  TEST_ASSERT(task->start(task_config));
  // delay for a while
  std::this_thread::sleep_for(1s);
  // assert the task was started
  TEST_ASSERT(task->started());
  // assert the task actually ran
  TEST_ASSERT_GREATER_THAN(starting_executions, basic_callback_executions);
};

TEST_CASE("start and stop repeat-call test", "[TASK]")
{
  int starting_executions = basic_callback_executions;
  auto task = phoenix::Task::make_unique();
  auto task_config =
    phoenix::Task::Config{.name = "single task",
                          .priority = 5,
                          .on_task_callback = basic_callback};
  // stop the task - shouldn't be able to stop it because it hasn't
  // been started!
  TEST_ASSERT(!task->stop());
  // start the task
  TEST_ASSERT(task->start(task_config));
  // start the task again - shouldn't be able to start it again since
  // it's already running
  TEST_ASSERT(!task->start(task_config));
  // delay for a while
  std::this_thread::sleep_for(1s);
  // assert the task was started
  TEST_ASSERT(task->started());
  // assert the task actually ran
  TEST_ASSERT_GREATER_THAN(starting_executions, basic_callback_executions);
  // stop the task
  TEST_ASSERT(task->stop());
  // stop the task again - shouldn't be able to stop it again
  TEST_ASSERT(!task->stop());
};

TEST_CASE("class callback test", "[TASK]")
{
  Foo foo;
  auto member_callback = std::bind(&Foo::member_function, &foo);
  auto static_callback = &Foo::static_function;
  {
    int starting_executions = foo.bar;
    auto task = phoenix::Task::make_unique();
    auto task_config =
      phoenix::Task::Config{.name = "single task",
                            .priority = 5,
                            .on_task_callback = member_callback};
    // start the task
    TEST_ASSERT(task->start(task_config));
    // delay for a while
    std::this_thread::sleep_for(1s);
    // assert the task actually ran
    TEST_ASSERT_GREATER_THAN(starting_executions, foo.bar);
  }
  {
    int starting_executions = Foo::foobar;
    auto task = phoenix::Task::make_unique();
    auto task_config =
      phoenix::Task::Config{.name = "single task",
                            .priority = 5,
                            .on_task_callback = static_callback};
    // start the task
    TEST_ASSERT(task->start(task_config));
    // delay for a while
    std::this_thread::sleep_for(1s);
    // assert the task actually ran
    TEST_ASSERT_GREATER_THAN(starting_executions, Foo::foobar);
  }
};

TEST_CASE("multi-task test", "[TASK]")
{
  Foo foo1, foo2;
  auto task1_callback = std::bind(&Foo::member_function, &foo1);
  auto task2_callback = std::bind(&Foo::member_function, &foo2);

  int starting_executions1 = foo1.bar;
  int starting_executions2 = foo2.bar;

  auto task1 = phoenix::Task::make_unique();
  auto task1_config =
    phoenix::Task::Config{.name = "task 1",
                          .priority = 5,
                          .on_task_callback = task1_callback};
  auto task2 = phoenix::Task::make_unique();
  auto task2_config =
    phoenix::Task::Config{.name = "task 2",
                          .priority = 5,
                          .on_task_callback = task2_callback};
  // start the tasks
  TEST_ASSERT(task1->start(task1_config));
  TEST_ASSERT(task2->start(task2_config));
  // delay for a while
  std::this_thread::sleep_for(1s);
  // assert the tasks actually ran
  TEST_ASSERT_GREATER_THAN(starting_executions1, foo1.bar);
  TEST_ASSERT_GREATER_THAN(starting_executions2, foo2.bar);
};
