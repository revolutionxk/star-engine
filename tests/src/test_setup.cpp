#include <catch2/catch_test_macros.hpp>
#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>

#include "star/core/logger.hpp"

class LoggerBootstrap final : public Catch::EventListenerBase {
  public:
    using Catch::EventListenerBase::EventListenerBase;

    void testRunStarting(const Catch::TestRunInfo&) override {
        star::Logger::initialize(star::LogLevel::Off, star::LogLevel::Off, "logs/test.log");
    }
};

CATCH_REGISTER_LISTENER(LoggerBootstrap)
