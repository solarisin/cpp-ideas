#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>
#include <fmt/format.h>
#include <loguru/loguru.hpp>
#include <string>

class testRunListener : public Catch::EventListenerBase {
public:
    using Catch::EventListenerBase::EventListenerBase;

    void testRunStarting(Catch::TestRunInfo const&) override {
        {
            LOG_F(0, "testRunStarting");
        }
    }
};


int main( int argc, char* argv[] ) {
    // Check if this is just test discovery (--list-tests) or actual test execution
    bool isDiscovery = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]).find("--list") != std::string::npos) {
            isDiscovery = true;
            break;
        }
    }
    int result;
    if (!isDiscovery) {
        loguru::init(argc, argv);
        loguru::add_file("test-run.log", loguru::Append, loguru::Verbosity_MAX);
        LOG_SCOPE_F(0, "TEST RUN LOG");
        result = Catch::Session().run( argc, argv );
    } else {
        result = Catch::Session().run( argc, argv );
    }
    
    return result;
}


CATCH_REGISTER_LISTENER(testRunListener)