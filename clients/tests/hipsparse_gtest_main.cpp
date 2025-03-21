/* ************************************************************************
 * Copyright (c) 2018-2019 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * ************************************************************************ */

#include "utility.hpp"
#ifdef GOOGLE_TEST
#include <gtest/gtest.h>
#endif
#include <hip/hip_runtime_api.h>
#include <stdexcept>

using namespace testing;

class ConfigurableEventListener : public TestEventListener
{
    TestEventListener* eventListener;

public:
    bool showTestCases; // Show the names of each test case.
    bool showTestNames; // Show the names of each test.
    bool showSuccesses; // Show each success.
    bool showInlineFailures; // Show each failure as it occurs.
    bool showEnvironment; // Show the setup of the global environment.

    explicit ConfigurableEventListener(TestEventListener* theEventListener)
        : eventListener(theEventListener)
        , showTestCases(true)
        , showTestNames(true)
        , showSuccesses(true)
        , showInlineFailures(true)
        , showEnvironment(true)
    {
    }

    ~ConfigurableEventListener() override
    {
        delete eventListener;
    }

    void OnTestProgramStart(const UnitTest& unit_test) override
    {
        eventListener->OnTestProgramStart(unit_test);
    }

    void OnTestIterationStart(const UnitTest& unit_test, int iteration) override
    {
        eventListener->OnTestIterationStart(unit_test, iteration);
    }

    void OnEnvironmentsSetUpStart(const UnitTest& unit_test) override
    {
        if(showEnvironment)
        {
            eventListener->OnEnvironmentsSetUpStart(unit_test);
        }
    }

    void OnEnvironmentsSetUpEnd(const UnitTest& unit_test) override
    {
        if(showEnvironment)
        {
            eventListener->OnEnvironmentsSetUpEnd(unit_test);
        }
    }

    void OnTestCaseStart(const TestCase& test_case) override
    {
        if(showTestCases)
        {
            eventListener->OnTestCaseStart(test_case);
        }
    }

    void OnTestStart(const TestInfo& test_info) override
    {
        if(showTestNames)
        {
            eventListener->OnTestStart(test_info);
        }
    }

    void OnTestPartResult(const TestPartResult& result) override
    {
        eventListener->OnTestPartResult(result);
    }

    void OnTestEnd(const TestInfo& test_info) override
    {
        if(test_info.result()->Failed() ? showInlineFailures : showSuccesses)
        {
            eventListener->OnTestEnd(test_info);
        }
    }

    void OnTestCaseEnd(const TestCase& test_case) override
    {
        if(showTestCases)
        {
            eventListener->OnTestCaseEnd(test_case);
        }
    }

    void OnEnvironmentsTearDownStart(const UnitTest& unit_test) override
    {
        if(showEnvironment)
        {
            eventListener->OnEnvironmentsTearDownStart(unit_test);
        }
    }

    void OnEnvironmentsTearDownEnd(const UnitTest& unit_test) override
    {
        if(showEnvironment)
        {
            eventListener->OnEnvironmentsTearDownEnd(unit_test);
        }
    }

    void OnTestIterationEnd(const UnitTest& unit_test, int iteration) override
    {
        eventListener->OnTestIterationEnd(unit_test, iteration);
    }

    void OnTestProgramEnd(const UnitTest& unit_test) override
    {
        eventListener->OnTestProgramEnd(unit_test);
    }
};

/* =====================================================================
      Main function:
=================================================================== */

int main(int argc, char** argv)
{
    // Get device id from command line
    int device_id = 0;

    for(int i = 1; i < argc; ++i)
    {
        if(strcmp(argv[i], "--device") == 0 && argc > i + 1)
        {
            device_id = atoi(argv[i + 1]);
        }
    }

    // Device Query
    int device_count = query_device_property();

    if(device_count <= device_id)
    {
        fprintf(stderr, "Error: invalid device ID. There may not be such device ID. Will exit\n");
        return -1;
    }
    else
    {
        set_device(device_id);
    }

    // Print version
    char version[256];
    query_version(version);

    printf("hipSPARSE version: %s\n", version);

    // Initialize google test
    InitGoogleTest(&argc, argv);

    // Remove the default listener
    auto& listeners       = UnitTest::GetInstance()->listeners();
    auto  default_printer = listeners.Release(listeners.default_result_printer());

    // Add our listener, by default everything is on (the same as using the default listener)
    // Here turning everything off so only the 3 lines for the result are visible
    // (plus any failures at the end), like:

    // [==========] Running 149 tests from 53 test cases.
    // [==========] 149 tests from 53 test cases ran. (1 ms total)
    // [  PASSED  ] 149 tests.
    //
    auto listener       = new ConfigurableEventListener(default_printer);
    auto gtest_listener = getenv("GTEST_LISTENER");

    if(gtest_listener && !strcmp(gtest_listener, "NO_PASS_LINE_IN_LOG"))
    {
        listener->showTestNames = listener->showSuccesses = listener->showInlineFailures = false;
    }

    listeners.Append(listener);

    // Run all tests
    int ret = RUN_ALL_TESTS();

    // Reset HIP device
    hipDeviceReset();

    return ret;
}
