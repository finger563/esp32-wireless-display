// header for code under test
#include "console_logger.hpp"

#include <thread>
#include <chrono>

// required for TEST_CASE & TEST_ASSERT* etc.
#include "unity.h"
// required for unity_wait_for_signal etc.
#include "test_utils.hpp"

TEST_CASE("Info logging", "[LOGGER]")
{
  phoenix::ConsoleLogger logger;
  logger.info("This is an info message");
}

TEST_CASE("Warn logging", "[LOGGER]")
{
  phoenix::ConsoleLogger logger;
  logger.warn("This is a warning message");
}

TEST_CASE("Error logging", "[LOGGER]")
{
  phoenix::ConsoleLogger logger;
  logger.error("This is an error message");
}

TEST_CASE("Info logging", "[LOGGER]")
{
  phoenix::ConsoleLogger logger;
  logger.info("This is a looooooooooooooooooooooooooooooooooooooong message");
  logger.warn("This is another looooooooooooooooooooooooooooooooooooooong message");
  logger.error("This is an even longer looooooooooooooooooooooooooooooooooooooo\n"
               "ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n"
               "ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n"
               "ooooong message");
}

TEST_CASE("Formatting", "[LOGGER]")
{
  phoenix::ConsoleLogger logger;
  logger.info("[{}]::{}{}::{}::The value of foobar is: {}", "INFO", "Thread#", 3, "My Component", 233.124151f);
}

TEST_CASE("Multi-threaded logging using a single logger", "[LOGGER]")
{
  phoenix::ConsoleLogger logger;

  auto t1 = std::thread([&logger] {
    auto i = 0;
    while(i < 40) {
      logger.info("Info message #{}", i);
      i += 1;
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
  });

  auto t2 = std::thread([&logger] {
    auto i = 0;
    while(i < 30) {
      logger.warn("Warn message #{}", i);
      i += 1;
      std::this_thread::sleep_for(std::chrono::milliseconds(125));
    }
  });

  auto t3 = std::thread([&logger] {
    auto i = 0;
    while(i < 20) {
      logger.error("Error message #{}", i);
      i += 1;
      std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
  });

  t1.join();
  t2.join();
  t3.join();
}

TEST_CASE("Multi-threaded logging using multiple loggers", "[LOGGER]")
{
  phoenix::ConsoleLogger logger_1;
  phoenix::ConsoleLogger logger_2;
  phoenix::ConsoleLogger logger_3;

  auto t1 = std::thread([&logger_1] {
    auto i = 0;
    while(i < 1000) {
      logger_1.info("Logger 1 info messages #{}", i);
      i += 1;
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
  });

  auto t2 = std::thread([&logger_2] {
    auto i = 0;
    while(i < 1000) {
      logger_2.warn("Logger 2 warn messages #{}", i);
      i += 1;
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
  });

  auto t3 = std::thread([&logger_3] {
    auto i = 0;
    while(i < 1000) {
      logger_3.error("Logger 3 error messages #{}", i);
      i += 1;
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
  });

  t1.join();
  t2.join();
  t3.join();
}