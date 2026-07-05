#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "star/application/command_line_args.hpp"

using namespace star::application;

TEST_CASE("CommandLineArgs - empty by default", "[application][args]") {
    CommandLineArgs args;
    REQUIRE(args.empty());
    REQUIRE(args.size() == 0u);
    REQUIRE(args.get_positional().empty());
    REQUIRE(args.get_executable_path().empty());
}

TEST_CASE("CommandLineArgs - executable path is extracted from argv[0]", "[application][args]") {
    const char* argv[] = {"my_engine"};
    const CommandLineArgs args(1, argv);
    REQUIRE(args.get_executable_path() == "my_engine");
    REQUIRE(args.empty());
}

TEST_CASE("CommandLineArgs - double-dash flags are recognized", "[application][args]") {
    const char* argv[] = {"app", "--verbose", "--debug"};
    CommandLineArgs args(3, argv);
    REQUIRE(args.has_flag("verbose"));
    REQUIRE(args.has_flag("debug"));
    REQUIRE_FALSE(args.has_flag("release"));
}

TEST_CASE("CommandLineArgs - single-dash flags are recognized", "[application][args]") {
    const char* argv[] = {"app", "-v", "-d"};
    const CommandLineArgs args(3, argv);
    REQUIRE(args.has_flag("v"));
    REQUIRE(args.has_flag("d"));
}

TEST_CASE("CommandLineArgs - querying a missing flag returns false", "[application][args]") {
    const char* argv[] = {"app"};
    const CommandLineArgs args(1, argv);
    REQUIRE_FALSE(args.has_flag("nonexistent"));
}

TEST_CASE("CommandLineArgs - key=value pairs are parsed", "[application][args]") {
    const char* argv[] = {"app", "--width=1920", "--height=1080"};
    const CommandLineArgs args(3, argv);
    REQUIRE(args.get_value("width") == "1920");
    REQUIRE(args.get_value("height") == "1080");
}

TEST_CASE("CommandLineArgs - key followed by value (space-separated)", "[application][args]") {
    const char* argv[] = {"app", "--scene", "main_scene"};
    const CommandLineArgs args(3, argv);
    REQUIRE(args.get_value("scene") == "main_scene");
}

TEST_CASE("CommandLineArgs - missing key returns default value", "[application][args]") {
    const char* argv[] = {"app"};
    const CommandLineArgs args(1, argv);
    REQUIRE(args.get_value("missing", "default") == "default");
}

TEST_CASE("CommandLineArgs - get_int parses integer value", "[application][args]") {
    const char* argv[] = {"app", "--samples=8"};
    const CommandLineArgs args(2, argv);
    REQUIRE(args.get_int("samples") == 8);
}

TEST_CASE("CommandLineArgs - get_int returns default for missing key", "[application][args]") {
    const char* argv[] = {"app"};
    const CommandLineArgs args(1, argv);
    REQUIRE(args.get_int("missing", 42) == 42);
}

TEST_CASE("CommandLineArgs - get_float parses float value", "[application][args]") {
    const char* argv[] = {"app", "--scale=2.5"};
    const CommandLineArgs args(2, argv);
    REQUIRE(args.get_float("scale") == Catch::Approx(2.5f));
}

TEST_CASE("CommandLineArgs - get_float returns default for missing key", "[application][args]") {
    const char* argv[] = {"app"};
    const CommandLineArgs args(1, argv);
    REQUIRE(args.get_float("missing", 1.0f) == Catch::Approx(1.0f));
}

TEST_CASE("CommandLineArgs - get_bool recognizes true values", "[application][args]") {
    SECTION("true") {
        const char* argv[] = {"app", "--flag=true"};
        const CommandLineArgs args(2, argv);
        REQUIRE(args.get_bool("flag"));
    }
    SECTION("1") {
        const char* argv[] = {"app", "--flag=1"};
        const CommandLineArgs args(2, argv);
        REQUIRE(args.get_bool("flag"));
    }
    SECTION("yes") {
        const char* argv[] = {"app", "--flag=yes"};
        const CommandLineArgs args(2, argv);
        REQUIRE(args.get_bool("flag"));
    }
    SECTION("on") {
        const char* argv[] = {"app", "--flag=on"};
        const CommandLineArgs args(2, argv);
        REQUIRE(args.get_bool("flag"));
    }
}

TEST_CASE("CommandLineArgs - get_bool recognizes false values", "[application][args]") {
    SECTION("false") {
        const char* argv[] = {"app", "--flag=false"};
        const CommandLineArgs args(2, argv);
        REQUIRE_FALSE(args.get_bool("flag"));
    }
    SECTION("0") {
        const char* argv[] = {"app", "--flag=0"};
        const CommandLineArgs args(2, argv);
        REQUIRE_FALSE(args.get_bool("flag"));
    }
    SECTION("no") {
        const char* argv[] = {"app", "--flag=no"};
        const CommandLineArgs args(2, argv);
        REQUIRE_FALSE(args.get_bool("flag"));
    }
    SECTION("off") {
        const char* argv[] = {"app", "--flag=off"};
        const CommandLineArgs args(2, argv);
        REQUIRE_FALSE(args.get_bool("flag"));
    }
}

TEST_CASE("CommandLineArgs - get_bool returns default for missing key", "[application][args]") {
    const char* argv[] = {"app"};
    const CommandLineArgs args(1, argv);
    REQUIRE(args.get_bool("missing", true) == true);
    REQUIRE(args.get_bool("missing", false) == false);
}

TEST_CASE("CommandLineArgs - positional arguments are collected", "[application][args]") {
    const char* argv[] = {"app", "file1.txt", "file2.txt"};
    const CommandLineArgs args(3, argv);
    const auto& pos = args.get_positional();
    REQUIRE(pos.size() == 2u);
    REQUIRE(pos[0] == "file1.txt");
    REQUIRE(pos[1] == "file2.txt");
}

TEST_CASE("CommandLineArgs - all() contains all non-exe args", "[application][args]") {
    const char* argv[] = {"app", "--verbose", "scene.json"};
    const CommandLineArgs args(3, argv);
    REQUIRE(args.get_all().size() == 2u);
    REQUIRE(args.size() == 2u);
}

TEST_CASE("CommandLineArgs - mixed flags, key-values, and positionals", "[application][args]") {
    const char* argv[] = {"app", "--verbose", "--width=1920", "scene.json"};
    const CommandLineArgs args(4, argv);

    REQUIRE(args.has_flag("verbose"));
    REQUIRE(args.get_value("width") == "1920");
    REQUIRE(args.get_positional().size() == 1u);
    REQUIRE(args.get_positional()[0] == "scene.json");
}

TEST_CASE("CommandLineArgs - negative numeric values are treated as values, not options", "[application][args]") {
    SECTION("negative integer") {
        const char* argv[] = {"app", "--speed", "-5"};
        const CommandLineArgs args(3, argv);
        REQUIRE(args.get_value("speed") == "-5");
        REQUIRE(args.get_int("speed") == -5);
        REQUIRE(args.get_positional().empty());
    }
    SECTION("negative float") {
        const char* argv[] = {"app", "--offset", "-2.5"};
        const CommandLineArgs args(3, argv);
        REQUIRE(args.get_float("offset") == Catch::Approx(-2.5f));
    }
}

TEST_CASE("CommandLineArgs - a following option is not swallowed as a value", "[application][args]") {
    const char* argv[] = {"app", "--mode", "--fast"};
    const CommandLineArgs args(3, argv);
    REQUIRE(args.has_flag("mode"));
    REQUIRE(args.has_flag("fast"));
    REQUIRE(args.get_value("mode", "none") == "none");
}

TEST_CASE("CommandLineArgs - parse clears previous state", "[application][args]") {
    const char* argv1[] = {"app", "--a"};
    const char* argv2[] = {"app", "--b"};

    CommandLineArgs args(2, argv1);
    REQUIRE(args.has_flag("a"));

    args.parse(2, argv2);
    REQUIRE_FALSE(args.has_flag("a"));
    REQUIRE(args.has_flag("b"));
}
