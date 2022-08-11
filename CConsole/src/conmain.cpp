/*
    ###############################################
    conmain.cpp
    CConsole demo program
    Made by PR00F88
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ################################################
*/

#include "CConsole.h"

#include <stdlib.h>

#include <atomic>              // requires cpp11
#include <condition_variable>  // requires cpp11
#include <mutex>               // requires cpp11
#include <thread>              // requires cpp11

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include "../../../PFL/PFL/PFL.h"


#define CON_TITLE "CConsole demo program"


#pragma warning(disable:4100)  /* unreferenced formal parameter */


static void TestDefaultColors(CConsole& con)
{
    con.OLn("%s", __func__);
    con.L();

    con.OLn("Simple text.");
    con.OLn("%s", "This is a string.");
    con.OLn("Signed integral value: %d", -5);
    con.OLn("Unsigned integral value: %u", 5);
    con.OLn("Floating point value: %f", 5.30215f);
    con.OLn("Boolean value: %b", false);
    con.OLn("");
}

static void TestErrorMode(CConsole& con)
{
    con.OLn("%s", __func__);
    con.L();

    con.OLn("Switching to error-mode now...");
    con.EOn();

    con.OLn("Simple text.");
    con.OLn("%s", "This is a string.");
    con.OLn("Signed integral value: %d", -5);
    con.OLn("Unsigned integral value: %u", 5);
    con.OLn("Floating point value: %f", 5.30215f);
    con.OLn("Boolean value: %b", false);
    con.OLn("");

    con.OLn("Switching back to normal mode now ...");
    con.EOff();

    con.OLn("Simple text.");
    con.OLn("%s", "This is a string.");
    con.OLn("Signed integral value: %d", -5);
    con.OLn("Unsigned integral value: %u", 5);
    con.OLn("Floating point value: %f", 5.30215f);
    con.OLn("Boolean value: %b", false);
    con.OLn("");
}

static void TestCustomColors(CConsole& con)
{
    con.OLn("%s", __func__);
    con.L();

    con.SetFGColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, "999999");
    con.SetIntsColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "FFFF00");
    con.SetStringsColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY, "FFFFFF");
    con.SetFloatsColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "FFFF00");
    con.SetBoolsColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "00FFFF");
    con.OLn("Colors changed!");

    con.OLn("Simple text.");
    con.OLn("%s", "This is a string.");
    con.OLn("Signed integral value: %d", -5);
    con.OLn("Unsigned integral value: %u", 5);
    con.OLn("Floating point value: %f", 5.30215f);
    con.OLn("Boolean value: %b", false);
    con.OLn("");

    con.OLn("Switching to error-mode now...");
    con.EOn();

    con.OLn("Simple text.");
    con.OLn("%s", "This is a string.");
    con.OLn("Signed integral value: %d", -5);
    con.OLn("Unsigned integral value: %u", 5);
    con.OLn("Floating point value: %f", 5.30215f);
    con.OLn("Boolean value: %b", false);
    con.OLn("");

    con.OLn("Switching back to normal mode now ...");
    con.EOff();

    con.OLn("Simple text.");
    con.OLn("%s", "This is a string.");
    con.OLn("Signed integral value: %d", -5);
    con.OLn("Unsigned integral value: %u", 5);
    con.OLn("Floating point value: %f", 5.30215f);
    con.OLn("Boolean value: %b", false);
    con.OLn("");
}

static void TestOperatorStreamOut(CConsole& con)
{
    con.OLn("%s", __func__);
    con.L();

    con << "Using operator<<, this is a string, and this is a boolean: " << false << ", this is an integer: " << 16 << CConsole::FormatSignal::NL;
    con << "This is already a new line";
    con << " , and this is still the same line" << CConsole::FormatSignal::NL;
    con << "This is a new line with a float: " << 4.67f << CConsole::FormatSignal::NL;
    con << CConsole::FormatSignal::E << "This is error mode " << CConsole::FormatSignal::S << "but this is success mode." << CConsole::FormatSignal::NL;
    con << CConsole::FormatSignal::N << "This is normal mode again." << CConsole::FormatSignal::NL;
    con << CConsole::FormatSignal::NL;
}

static void TestModuleLoggingSet(CConsole& con)
{
    con.OLn("%s", __func__);
    con.L();

    con.SetLoggingState(CON_TITLE, false);
    con.OLn("You are not supposed to see this, line %d!", __LINE__);
    con << "You are not supposed to see this either, line " << __LINE__ << "!" << CConsole::FormatSignal::NL;

    con.SetLoggingState("4LLM0DUL3S", true);
    con.OLn("You should see this due to all debugs are turned on now, line %d", __LINE__);
    con << "You should see this too due to all debugs are turned on now, line " << __LINE__ << CConsole::FormatSignal::NL;
    con.SetLoggingState("4LLM0DUL3S", false);

    con.OLn("You are not supposed to see this at line %d!", __LINE__);
    con.EOLn("You should see this due to being in error mode, line %d", __LINE__);
    // notice that we pass NL before passing N, otherwise in reverse order no newline will be added, it will be also ignored since module logging is not enabled
    con << CConsole::FormatSignal::E << "You should also see this due to error mode, line " << __LINE__ << CConsole::FormatSignal::NL << CConsole::FormatSignal::N;

    con.OLn("You are not supposed to see this at line %d!", __LINE__);
    con.SOLn("You should not see this success log, line %d", __LINE__);
    con << CConsole::FormatSignal::S << "You should not see this success log, line " << __LINE__ << CConsole::FormatSignal::NL << CConsole::FormatSignal::N;

    con.SetLoggingState(CON_TITLE, true);
    con.SOLn("You supposed to see this success log, line %d", __LINE__);
    con << CConsole::FormatSignal::S << "You supposed to see this success log, line " << __LINE__ << CConsole::FormatSignal::NL << CConsole::FormatSignal::N;
    con.OLn("");
}

static std::mutex mtx;
static std::condition_variable cv;
static std::atomic<int> numThreadsWaiting = 0;
static std::atomic<int> nThreadCntr = 0;
static std::atomic<int> nErrorsOutCount = 0;
static std::atomic<int> nSuccessOutCount = 0;

static void threadFunc(CConsole& con, CConsole::FormatSignal fs)
{
    // in case of consecutive Initialize() calls, the parameters don't really matter
    con.Initialize("", false);

    // invoking following 2 functions is not necessary for this test, however I'm explicitly calling them
    // to remember myself about known issue A) explained in CConsole.h.
    con.RestoreDefaultColors();
    con.SaveColors();

    std::string sThreadName;
    switch (fs)
    {
    case CConsole::FormatSignal::S:
        con.SOn();
        con.SetIndent(8);
        sThreadName = "successThread";
        break;
    case CConsole::FormatSignal::E:
        con.EOn();
        con.SetIndent(12);
        sThreadName = "errorThread";
        break;
    default:
        con.NOn();
        con.SetIndent(4);
        sThreadName = "normalThread";
    }
    
    con.OLn("%s is starting now", sThreadName.c_str());

    // now we wait for all threads to execute above statements
    // ##############################################################################################################################
    std::unique_lock<std::mutex> lk(mtx);  // lock must be locked by current thread, so lock is thread-specific, but mutex is shared!
    
    // basically numThreadsWaiting and nThreadCntr are used for same purpose, their roles are replaced after each test

    numThreadsWaiting++;
    if (numThreadsWaiting == 3)
    {
        con.ResetErrorOutsCount();
        con.ResetSuccessOutsCount();
        nErrorsOutCount = 0;
        nSuccessOutCount = 0;
        nThreadCntr = 0;
    }
    // first 2 threads will actually wait here, the last thread setting numThreadsWaiting to 3 won't have to wait, it will continue
    cv.wait(lk, [&] {return numThreadsWaiting == 3; });
    cv.notify_all();
    lk.unlock();  // wait() unlocks the lock, but after continuing it will be locked again, so we explicitly unlock it here:
    // if we don't explicitly unlock it, the other 2 threads will wait until a thread exits.
    // we dont need the lock/mutex anymore, because every Console stuff we invoke below are expected to be thread-safe anyway!

    // first test: text output, indentation and mode (colors) are unique for each thread, they dont mess with each other
    // ##############################################################################################################################
    for (int i = 1; i <= 10; i++)
    {
        if (i % 5 == 0)
        {
            const int newIndentation = PFL::random(0, 20);
            con.SetIndent(newIndentation);
            con.OLn("%s has set new indentation: %d", sThreadName.c_str(), newIndentation);
        }

        con.OLn("%s logNo %d: some log blah blah blah 123 123", sThreadName.c_str(), i);
    }

    nThreadCntr++;

    // preparing for next test, we are waiting here for all threads to finish above test
    // ##############################################################################################################################
    lk.lock();

    // reset original indentations
    switch (fs)
    {
    case CConsole::FormatSignal::S: con.SetIndent(8); break;
    case CConsole::FormatSignal::E: con.SetIndent(12); break;
    default: con.SetIndent(4);
    }
    if (nThreadCntr == 3)
    {
        // last finishing thread will run this code
        nErrorsOutCount = con.getErrorOutsCount();
        nSuccessOutCount = con.getSuccessOutsCount();

        con.NOn(); // temporal normal mode
        con.OLn("");
        con.OLn("All threads have reset their indentation and ready for next test!");
        con.OLn("");
        con.OLn("Number of error outs during previous test: %d, this is %s!", nErrorsOutCount.load(), (nErrorsOutCount.load() == 12 ? "GOOD" : "BAD"));
        con.OLn("Number of success outs during previous test: %d, this is %s!", nSuccessOutCount.load(), (nSuccessOutCount.load() == 12 ? "GOOD" : "BAD"));
        con.OLn("");
        con.OLn("Next test starting");
        con.OLn("");

        // set required colors again
        switch (fs)
        {
        case CConsole::FormatSignal::S: con.SOn(); break;
        case CConsole::FormatSignal::E: con.EOn(); break;
        default:
            ; // nothing to do
        }

        con.ResetErrorOutsCount();
        con.ResetSuccessOutsCount();
        nErrorsOutCount = 0;
        nSuccessOutCount = 0;
        nThreadCntr = 0;
    }

    // SetLoggingState is needed because later we will use CConsole::getConsoleInstance() with module name
    con.SetLoggingState(sThreadName.c_str(), true);
    con.SetErrorsAlwaysOn(false);
    
    // first 2 threads will actually wait here, the last thread setting nThreadCntr to 0 won't have to wait, it will continue
    numThreadsWaiting = 0;
    cv.wait(lk, [&] {return nThreadCntr == 0; });
    cv.notify_all();
    lk.unlock();  // explanation of why we are doing this is written above before previous test

    // next test: using getConsoleInstance() with different module names, we expect threads are not messing with each other
    // ##############################################################################################################################
    int nVisibleLogs = 0;
    int nHiddenLogs = 0;
    for (int i = 1; i <= 10; i++)
    {
        if (i % 2 == 0)
        {
            const bool bModuleEnabled = (PFL::random(0, 1) == 1);
            if (con.getLoggingState(sThreadName.c_str()) != bModuleEnabled)
            {
                // here we use empty string for getConsoleInstance() because this way log will always appear regardless of module filter state
                CConsole::getConsoleInstance("").OLn("%s: has %s logging for its module name with %d visible and %d hidden logs so far",
                    sThreadName.c_str(), 
                    bModuleEnabled ? "enabled" : "disabled",
                    nVisibleLogs,
                    nHiddenLogs);
                // note that using empty string will actually clear the logger module name for current thread, so make sure
                // we always use the logger module name with getConsoleInstance() whenever we are logging something, to set the name again!
                con.SetLoggingState(sThreadName.c_str(), bModuleEnabled);
            }
        }
        // and here we log with module name, which might be enabled or disabled
        CConsole::getConsoleInstance(sThreadName.c_str()).OLn("%s logNo %d: should this be a visible log, shouldn't be?", sThreadName.c_str(), i);
        if (con.getLoggingState(sThreadName.c_str()))
        {
            nVisibleLogs++;
        }
        else
        {
            nHiddenLogs++;
        }
    }

    con.SetLoggingState(sThreadName.c_str(), true);
    con.NOn(); // temporal normal mode
    con.OLn("%s FINISHED, number of visible logs: %d, number of hidden logs: %d, total is %d, should be %d, that is %s!",
        sThreadName.c_str(), nVisibleLogs, nHiddenLogs, nVisibleLogs + nHiddenLogs, 10, (10 == nVisibleLogs + nHiddenLogs ? "GOOD" : "BAD"));
    
    numThreadsWaiting++;
    
    // preparing for next test, we are waiting here for all threads to finish above test
    // ##############################################################################################################################
    lk.lock();
    if (numThreadsWaiting == 3)
    {
        // last finishing thread will run this code
        con.OLn("");
        con.OLn("All threads have re-enabled their logging for their module names and are ready for next test!");
        con.OLn("Next test starting");
        con.OLn("");
        numThreadsWaiting = 0;
    }

    // set required colors again
    switch (fs)
    {
    case CConsole::FormatSignal::S: con.SOn(); break;
    case CConsole::FormatSignal::E: con.EOn(); break;
    default:
        ; // nothing to do
    }
    
    // first 2 threads will actually wait here, the last thread setting numThreadsWaiting to 0 won't have to wait, it will continue
    nThreadCntr = 0;
    cv.wait(lk, [&] {return numThreadsWaiting == 0; });
    cv.notify_all();
    lk.unlock();  // explanation of why we are doing this is written above at first test

    // next test could come here
    // ##############################################################################################################################

    con.OLn("%s is exiting now", sThreadName.c_str());
    con.NOn();
    con.Deinitialize();
}

static void TestConcurrentLogging(CConsole& con)
{   
    /*
     * Unfortunately, Valgrind is not available for Windows.
     * However, there are multiple tools for checking thread behavior.
     * There is an official extension for VS2022, Concurrency Visualizer:
     * https://marketplace.visualstudio.com/items?itemName=Diagnostics.DiagnosticsConcurrencyVisualizer2022#overview
     * 
     * Intel Inspector can pinpoint data race, etc:
     * https://www.intel.com/content/www/us/en/developer/articles/tool/oneapi-standalone-components.html#inspector
     * 
     * And also in Windows SDK, there is the Application Verifier:
     * https://docs.microsoft.com/en-us/windows-hardware/drivers/devtest/application-verifier#installing-appverifier
     */
    con.OLn("%s", __func__);
    con.L();
    con.OLn("You should see 3 different threads logging their own logs with their own color and own indentation.");
    con.OLn("Each thread should NOT have impact on other thread's color (formatsignal) or indentation.");
    con.OLn("");

    std::thread successThread = std::thread{ threadFunc, std::ref(con), CConsole::FormatSignal::S };
    std::thread errorThread = std::thread{ threadFunc, std::ref(con), CConsole::FormatSignal::E };
    std::thread normalThread = std::thread{ threadFunc, std::ref(con), CConsole::FormatSignal::N };

    // thread synchronization for starting their work is in threadFunc(), we are just waiting for them here to finish their job

    if (successThread.get_id() != std::thread().get_id())
    {
        if (successThread.joinable())
        {
            successThread.join();
            con.OLn("Main Thread: successThread joined");
        }
    }
    if (errorThread.get_id() != std::thread().get_id())
    {
        if (errorThread.joinable())
        {
            errorThread.join();
            con.OLn("Main Thread: errorThread joined");
        }
    }
    if (normalThread.get_id() != std::thread().get_id())
    {
        if (normalThread.joinable())
        {
            normalThread.join();
            con.OLn("Main Thread: normalThread joined");
        }
    }
}

int WINAPI WinMain(const HINSTANCE hInstance, const HINSTANCE hPrevInstance, const LPSTR lpCmdLine, const int nCmdShow)
{
    CConsole& con = CConsole::getConsoleInstance(CON_TITLE);

    con.Initialize(CON_TITLE, true);
    con.SetLoggingState(CON_TITLE, true); // if we dont explicitly enable logging for CON_TITLE module, only errors will be visible
    con.OLn("%s", CON_TITLE);
    con.L();
    con.OLn("");

    TestDefaultColors(con);
    TestErrorMode(con);
    TestCustomColors(con);
    TestOperatorStreamOut(con);
    TestModuleLoggingSet(con);
    TestConcurrentLogging(con);

    system("pause");

    con.Deinitialize();

    return 0;

} // WinMain()
