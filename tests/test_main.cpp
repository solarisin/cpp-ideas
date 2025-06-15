#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>
#include <fmt/format.h>
#include <loguru/loguru.hpp>

class testRunListener : public Catch::EventListenerBase {
public:
    using Catch::EventListenerBase::EventListenerBase;

    void testRunStarting(Catch::TestRunInfo const& ri) override {
        {
            LOG_F(0, "testRunStarting");
        }
    }
};


int main( int argc, char* argv[] ) {
    loguru::init(argc, argv);
    loguru::add_file("test-run.log", loguru::Append, loguru::Verbosity_MAX);
    LOG_SCOPE_F(0, "TEST RUN LOG");
    int result = Catch::Session().run( argc, argv );

    return result;
}


CATCH_REGISTER_LISTENER(testRunListener)