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

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

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

static void threadFunc(CConsole& con, CConsole::FormatSignal fs, std::atomic<int>& numThreadsWaiting)
{
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

    std::unique_lock<std::mutex> lk(mtx);
    numThreadsWaiting++;
    cv.notify_all();
    // first 2 threads will actually wait here, the last thread setting numThreadsWaiting to 3 won't have to wait
    cv.wait(lk, [&] {return numThreadsWaiting == 3; });

    // TODO: per-module filter setting should be also tested, e.g. getConsoleInstance() accepts module name and stores it globally, it should be per-thread!
    for (int i = 1; i <= 10; i++)
    {
        /*
        switch (fs)
        {
        case CConsole::FormatSignal::S: con.SOn(); break;
        case CConsole::FormatSignal::E: con.EOn(); break;
        default: con.NOn();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        */
        if (i % 5 == 0)
        {
            const int newIndentation = PFL::random(0, 20);
            con.SetIndent(newIndentation);
            con.OLn("%s has set new indentation: %d", sThreadName.c_str(), newIndentation);
        }
        
        con.OLn("%s: some log blah blah blah 123 123", sThreadName.c_str());
    }

    con.OLn("%s is exiting now", sThreadName.c_str());
    con.NOn();
}

static void TestConcurrentLogging(CConsole& con)
{   
    con.OLn("%s", __func__);
    con.L();
    con.OLn("You should see 3 different threads logging their own logs with their own color and own indentation.");
    con.OLn("Each thread should NOT have impact on other thread's color (formatsignal) or indentation.");
    con.OLn("");

    std::atomic<int> numThreadsWaiting = 0;
    std::thread successThread = std::thread{ threadFunc, std::ref(con), CConsole::FormatSignal::S, std::ref(numThreadsWaiting) };
    std::thread errorThread = std::thread{ threadFunc, std::ref(con), CConsole::FormatSignal::E, std::ref(numThreadsWaiting) };
    std::thread normalThread = std::thread{ threadFunc, std::ref(con), CConsole::FormatSignal::N, std::ref(numThreadsWaiting) };

    if (successThread.get_id() != std::thread().get_id())
    {
        if (successThread.joinable())
        {
            successThread.join();
            con.OLn("%s: successThread joined", __func__);
        }
    }
    if (errorThread.get_id() != std::thread().get_id())
    {
        if (errorThread.joinable())
        {
            errorThread.join();
            con.OLn("%s: errorThread joined", __func__);
        }
    }
    if (normalThread.get_id() != std::thread().get_id())
    {
        if (normalThread.joinable())
        {
            normalThread.join();
            con.OLn("%s: normalThread joined", __func__);
        }
    }
}

int WINAPI WinMain(const HINSTANCE hInstance, const HINSTANCE hPrevInstance, const LPSTR lpCmdLine, const int nCmdShow)
{
    CConsole& con = CConsole::getConsoleInstance(CON_TITLE);

    con.Initialize(CON_TITLE, true);
    con.SetLoggingState(CON_TITLE, true); // if we dont explicitly enable logging for CON_TITLE module, only errors will be visible
    con.OLn(CON_TITLE);
    con.L();
    con.OLn("");

    TestDefaultColors(con);
    TestErrorMode(con);
    TestCustomColors(con);
    TestOperatorStreamOut(con);
    TestModuleLoggingSet(con);
    TestConcurrentLogging(con);

    system("pause");

    return 0;

} // WinMain()
