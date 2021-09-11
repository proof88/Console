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

    con.OLn("Ez sima szoveg.");
    con.OLn("%s", "Ez egy string.");
    con.OLn("Ez egesz elojeles szam: %d", 5);
    con.OLn("Ez egesz elojel nelkuli szam: %u", 5);
    con.OLn("Ez valos szam: %f", 5.0f);
    con.OLn("Ez logikai: %b", false);
    con.OLn("");
    con.OLn("Es most error-mode...");
    con.EOn();
    con.OLn("Ez sima szoveg.");
    con.OLn("%s", "Ez egy string.");
    con.OLn("Ez egesz elojeles szam: %d", 5);
    con.OLn("Ez egesz elojel nelkuli szam: %u", 5);
    con.OLn("Ez valos szam: %f", 5.0f);
    con.OLn("Ez logikai: %b", false);
    con.OLn("");
    con.EOff();
    con.OLn("Es most normal mode vissza...");
    con.OLn("Ez sima szoveg.");
    con.OLn("%s", "Ez egy string.");
    con.OLn("Ez egesz elojeles szam: %d", 5);
    con.OLn("Ez egesz elojel nelkuli szam: %u", 5);
    con.OLn("Ez valos szam: %f", 5.0f);
    con.OLn("Ez logikai: %b", false);
    con.OLn("");

    con.SetFGColor( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, "999999" );
    con.SetIntsColor( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "FFFF00" );
    con.SetStringsColor( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY, "FFFFFF" );
    con.SetFloatsColor( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "FFFF00" );
    con.SetBoolsColor( FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "00FFFF" );
    con.OLn("Szinek atallitva...");

    con.OLn("Ez sima szoveg.");
    con.OLn("%s", "Ez egy string.");
    con.OLn("Ez egesz elojeles szam: %d", 5);
    con.OLn("Ez egesz elojel nelkuli szam: %u", 5);
    con.OLn("Ez valos szam: %f", 5.0f);
    con.OLn("Ez logikai: %b", false);
    con.OLn("");
    con.OLn("Es most error-mode...");
    con.EOn();
    con.OLn("Ez sima szoveg.");
    con.OLn("%s", "Ez egy string.");
    con.OLn("Ez egesz elojeles szam: %d", 5);
    con.OLn("Ez egesz elojel nelkuli szam: %u", 5);
    con.OLn("Ez valos szam: %f", 5.0f);
    con.OLn("Ez logikai: %b", false);
    con.OLn("");
    con.EOff();
    con.OLn("Es most normal mode vissza...");
    con.OLn("Ez sima szoveg.");
    con.OLn("%s", "Ez egy string.");
    con.OLn("Ez egesz elojeles szam: %d", 5);
    con.OLn("Ez egesz elojel nelkuli szam: %u", 5);
    con.OLn("Ez valos szam: %f", 5.0f);
    con.OLn("Ez logikai: %b", false);
    con.OLn("");
    
    con << "Ez most operatoros string, ez meg boolean: " << false << ", ez meg szam: " << 16 << CConsole::FormatSignal::NL;
    con << "Ez meg egy uj sor mar.";
    con << " Ez ugyanaz a sor meg." << CConsole::FormatSignal::NL;
    con << "Itt meg float van: " << 4.67f << CConsole::FormatSignal::NL;
    con << CConsole::FormatSignal::E << "Ez most error mode, " << CConsole::FormatSignal::S << "de ez mar success." << CConsole::FormatSignal::NL;
    con << CConsole::FormatSignal::N << "Most ismet normal mode." << CConsole::FormatSignal::NL;

    system("pause");

    return 0;

} // WinMain()
