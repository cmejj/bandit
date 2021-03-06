#include <specs/specs.h>
namespace bd = bandit::detail;

go_bandit([]() {
  describe("reporter::dots", [&]() {
    std::stringstream stm;
    std::unique_ptr<reporter::dots> reporter;
    failure_formatter::posix formatter;
    colorizer::off colorizer;

    before_each([&]() {
      stm.str(std::string());
      reporter.reset(new reporter::dots(stm, formatter, colorizer));
    });

    auto output = [&]() { return stm.str(); };

    describe("an empty test run", [&]() {
      before_each([&]() {
        reporter->test_run_starting();
        reporter->test_run_complete();
      });

      it("reports that no tests were run", [&]() {
        AssertThat(output(), Equals("\nCould not find any tests.\n"));
      });

      it("is considered successful", [&]() {
        AssertThat(reporter->did_we_pass(), IsTrue());
      });
    });

    describe("a successful test run", [&]() {
      before_each([&]() {
        reporter->test_run_starting();
        reporter->context_starting("my context");
        reporter->it_starting("my test");
        reporter->it_succeeded("my test");
        reporter->context_ended("my context");
        reporter->test_run_complete();
      });

      it("reports a successful test run", [&]() {
        AssertThat(output(), Contains("Success!"));
        AssertThat(output(), EndsWith("Test run complete. 1 tests run. 1 succeeded.\n"));
      });

      it("displays a dot for the successful test", [&]() {
        AssertThat(output(), StartsWith("."));
      });

      it("reports a successful test run", [&]() {
        AssertThat(reporter->did_we_pass(), Equals(true));
      });
    });

    describe("a failing test run", [&]() {
      before_each([&]() {
        reporter->test_run_starting();
        reporter->context_starting("my context");
        reporter->it_starting("my test");

        bd::assertion_exception exception("assertion failed!", "some_file", 123);
        reporter->it_failed("my test", exception);

        reporter->context_ended("my context");
        reporter->test_run_complete();
      });

      it("reports a failing test run in summary", [&]() {
        AssertThat(output(), EndsWith("Test run complete. 1 tests run. 0 succeeded. 1 failed.\n"));
      });

      it("reports the failed assertion", [&]() {
        AssertThat(output(), Contains("my context my test:\nsome_file:123: assertion failed!"));
      });

      it("only reports assertion failure once", [&]() {
        AssertThat(output(), Has().Exactly(1).EndingWith("assertion failed!"));
      });

      it("reports an 'F' for the failed assertion", [&]() {
        AssertThat(output(), StartsWith("F"));
      });

      it("reports a failed test run", [&]() {
        AssertThat(reporter->did_we_pass(), Equals(false));
      });
    });

    describe("a test run with a non assertion_exception thrown", [&]() {
      before_each([&]() {
        reporter->test_run_starting();
        reporter->context_starting("my context");
        reporter->it_starting("my test");

        reporter->it_unknown_error("my test");

        reporter->context_ended("my context");
        reporter->test_run_complete();
      });

      it("reports an 'E' for the failed test", [&]() {
        AssertThat(output(), StartsWith("E"));
      });

      it("reports the failed test", [&]() {
        AssertThat(output(), Contains("my context my test:\nUnknown exception"));
      });
    });

    describe("a failing test run with nested contexts", [&]() {
      before_each([&]() {
        reporter->test_run_starting();
        reporter->context_starting("my context");
        reporter->context_starting("a nested context");
        reporter->it_starting("my test");

        bd::assertion_exception exception("assertion failed!", "some_file", 123);
        reporter->it_failed("my test", exception);

        reporter->context_ended("a nested context");
        reporter->context_ended("my context");
        reporter->test_run_complete();
      });

      it("reports a failing test run in summary", [&]() {
        AssertThat(output(), EndsWith("Test run complete. 1 tests run. 0 succeeded. 1 failed.\n"));
      });

      it("reports the failed assertion", [&]() {
        AssertThat(output(), Contains("my context a nested context my test:\nsome_file:123: assertion failed!"));
      });

      it("reports an 'F' for the failed assertion", [&]() {
        AssertThat(output(), StartsWith("F"));
      });

      it("reports a failed test run", [&]() {
        AssertThat(reporter->did_we_pass(), Equals(false));
      });
    });

    describe("a context with test run errors", [&]() {
      before_each([&]() {
        reporter->test_run_starting();
        reporter->context_starting("my context");

        bd::test_run_error error("we dun goofed!");
        reporter->test_run_error("my context", error);

        reporter->context_ended("my context");
        reporter->test_run_complete();
      });

      it("reports that the context has failed", [&]() {
        AssertThat(output(), Contains("Failed to run \"my context\": error \"we dun goofed!\""));
      });

      it("reports test run errors in summary", [&]() {
        AssertThat(output(), EndsWith("Test run complete. 0 tests run. 0 succeeded. 1 test run errors.\n"));
      });

      it("reports a failed test run", [&]() {
        AssertThat(reporter->did_we_pass(), Equals(false));
      });
    });

    describe("a context with a skipped test", [&]() {
      before_each([&]() {
        reporter->test_run_starting();
        reporter->context_starting("my context");

        reporter->it_starting("my test");
        reporter->it_succeeded("my test");
        reporter->it_skip("my skipped test");

        reporter->context_ended("my context");
        reporter->test_run_complete();
      });

      it("displays a dot for the successful test, and a 'S' for the skipped test", [&]() {
        AssertThat(output(), StartsWith(".S"));
      });

      it("reports that there is one skipped test in the summary", [&]() {
        AssertThat(output(), EndsWith("Test run complete. 1 tests run. 1 succeeded. 1 skipped.\n"));
      });
    });

    reporter.reset(); // necessary so that reporter dtor is called before colorizer dtor
  });
});
