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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>


#define CON_TITLE "CConsole demo program"


#pragma warning(disable:4100)  /* unreferenced formal parameter */


int WINAPI WinMain(const HINSTANCE hInstance, const HINSTANCE hPrevInstance, const LPSTR lpCmdLine, const int nCmdShow)
{

    CConsole& con = CConsole::getConsoleInstance("");

    con.Initialize(CON_TITLE, true);
    con.OLn(CON_TITLE);
    con.L();
    con.OLn("");

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

    con.SetFGColor( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, "999999" );
    con.SetIntsColor( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "FFFF00" );
    con.SetStringsColor( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY, "FFFFFF" );
    con.SetFloatsColor( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "FFFF00" );
    con.SetBoolsColor( FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "00FFFF" );
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
    
    con << "Using operator<<, this is a string, and this is a boolean: " << false << ", this is an integer: " << 16 << CConsole::FormatSignal::NL;
    con << "This is already a new line";
    con << " , and this is still the same line" << CConsole::FormatSignal::NL;
    con << "This is a new line with a float: " << 4.67f << CConsole::FormatSignal::NL;
    con << CConsole::FormatSignal::E << "This is error mode " << CConsole::FormatSignal::S << "but this is success mode." << CConsole::FormatSignal::NL;
    con << CConsole::FormatSignal::N << "This is normal mode again." << CConsole::FormatSignal::NL;

    system("pause");

    return 0;

} // WinMain()
