/*
    ###################################################################################
    CConsole.cpp
    Class to handle console window.
    Made by PR00F88
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "CConsole.h"

#include <stdio.h> 
#include <stdlib.h>

#include <ctime>
#include <fstream>
#include <iostream>
#include <mutex>
#include <set>
#include <string>

#include "../../../PFL/PFL/PFL.h"

// WINAPI header include just for the FOREGROUND_XXX and WORD macros and console API functions
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

// unused warnings
// todo: revise these warnings when C++11 compiler is set, so that std::iota, etc can be used instead of itoa
#pragma warning(disable:4996)  /* "may be unsafe" and "deprecated" */

/**
    If CCONSOLE_IS_ENABLED macro is defined, console window can be created and printouts will be visible, otherwise not.
    Comment this macro when you definitely dont want any logging functionality in your program, so that even if your code
    invokes the logging functions of this CConsole class, the log strings can be optimized out by the compiler since
    it will see there is no functionality in the invoked functions. This can be useful if you really wanna optimize for
    generated binary file size.
*/
#ifndef CCONSOLE_IS_ENABLED
#define CCONSOLE_IS_ENABLED
#endif

#ifndef CCONSOLE_IS_ENABLED
#pragma warning(disable:4100)  /* "unreferenced formal parameter", obviously we will see a lot of if our macro is undefined */
#endif

using namespace std;

static std::mutex mainMutex;  // did not want to put this into CConsoleImpl because then IsInitialized() could not be protected by this mutex when impl is not yet existing

/*
   CConsole::CConsoleImpl
   ###########################################################################
   Not thread-safe, but not public anyway.
   Only CConsole uses it, and CConsole is thread-safe.
*/

class CConsole::CConsoleImpl
{
public:
    void SetLoggingState(const char* loggerModule, bool state);  /**< Sets logging on or off for the given logger module. */
    void SetErrorsAlwaysOn(bool state);                          /**< Sets errors always appear irrespective of logging state of current logger module. */

    int  getIndent() const;       /**< Gets the current indentation. */
    void SetIndent(int value);    /**< Sets the current indentation. */
    void Indent();                /**< Increases indentation. */
    void IndentBy(int value);     /**< Increases indentation by the given value. */
    void Outdent();               /**< Decreases indentation. */
    void OutdentBy(int value);    /**< Decreases indentation by the given value. */

    void LoadColors();            /**< Loads previously saved colors. */
    void SaveColors();            /**< Saves current colors. */
    void RestoreDefaultColors();  /**< Restores default colors. */

    WORD        getFGColor() const;            /**< Gets foreground color. */
    const char* getFGColorHtml() const;        /**< Gets html foreground color. */
    void        SetFGColor(
        WORD clr, const char* html = NULL);   /**< Sets foreground color. */
    WORD        getBGColor() const;            /**< Gets background color. */
    void        SetBGColor(WORD clr);          /**< Sets background color. */

    WORD        getIntsColor() const;          /**< Gets ints color. */
    const char* getIntsColorHtml() const;      /**< Gets ints html color. */
    void        SetIntsColor(
        WORD clr, const char* html = NULL);   /**< Sets ints color. */

    WORD        getStringsColor() const;       /**< Gets strings color. */
    const char* getStringsColorHtml() const;   /**< Gets strings html color. */
    void        SetStringsColor(
        WORD clr, const char* html = NULL);   /**< Sets strings color. */

    WORD        getFloatsColor() const;        /**< Gets floats color. */
    const char* getFloatsColorHtml() const;    /**< Gets floats html color. */
    void        SetFloatsColor(
        WORD clr, const char* html = NULL);   /**< Sets floats color. */

    WORD        getBoolsColor() const;         /**< Gets bools color. */
    const char* getBoolsColorHtml() const;     /**< Gets bools html color. */
    void        SetBoolsColor(
        WORD clr, const char* html = NULL);   /**< Sets bools color. */

    void O(const char* text, va_list list);       /**< Prints text to console. */
    void OLn(const char* text, va_list list);     /**< Prints text to console and adds a new line. */
    void OLn(const char* text, ...);              /**< Prints text to console and adds a new line. */
    void OI();                           /**< Indent(). */
    void OIO(const char* text, va_list list);     /**< OI() + O(text). */
    void OIOLn(const char* text, va_list list);   /**< OI() + OLn(text). */
    void OLnOI(const char* text, va_list list);   /**< OLn(text) + OI(). */
    void OIb(int value);                 /**< IndentBy(). */
    void OO();                           /**< Outdent(). */
    void OOO(const char* text, va_list list);     /**< OO() + O(text). */
    void OOOLn(const char* text, va_list list);   /**< OO() + OLn(text). */
    void OLnOO(const char* text, va_list list);   /**< OLn(text) + OO(). */
    void OOb(int value);                 /**< OutdentBy(). */
    void OIOLnOO(const char* text, va_list list); /**< OI() + OLn(text) + OO(). */
    void L(int n = 20);                  /**< Prints line to console and adds a new line. */

    void NOn();                          /**< Normal-mode on. */
    void EOn();                          /**< Error-mode on. */
    void EOff();                         /**< Error-mode  off. */
    void SOn();                          /**< Success-mode on. */
    void SOff();                         /**< Success-mode off. */

    void SO(const char* text, va_list list);      /**< SOn() + O(text) + SOff(). */
    void SOLn(const char* text, va_list list);    /**< SOn() + OLn(text) + SOff(). */
    void SOLn(const char* text, ...);             /**< SOn() + OLn(text) + SOff(). */
    void EO(const char* text, va_list list);      /**< EOn() + O(text) + EOff(). */
    void EOLn(const char* text, va_list list);    /**< EOn() + OLn(text) + EOff(). */
    void EOLn(const char* text, ...);             /**< EOn() + OLn(text) + EOff(). */

    void OISO(const char* text, va_list list);    /**< OI() + SO(text). */
    void OISOLn(const char* text, va_list list);  /**< OI() + SOLn(text). */
    void OOSO(const char* text, va_list list);    /**< OO() + SO(text). */
    void OOSOLn(const char* text, va_list list);  /**< OO() + SOLn(text). */
    void OIEO(const char* text, va_list list);    /**< OI() + EO(text). */
    void OIEOLn(const char* text, va_list list);  /**< OI() + EOLn(text). */
    void OOEO(const char* text, va_list list);    /**< OO() + EO(text). */
    void OOEOLn(const char* text, va_list list);  /**< OO() + EOLn(text). */

    void SOOI(const char* text, va_list list);    /**< SO(text) + OI(). */
    void SOLnOI(const char* text, va_list list);  /**< SOLn(text) + OI(). */
    void SOOO(const char* text, va_list list);    /**< SO(text) + OO(). */
    void SOLnOO(const char* text, va_list list);  /**< SOLn(text) + OO(). */
    void EOOI(const char* text, va_list list);    /**< EO(text) + OI(). */
    void EOLnOI(const char* text, va_list list);  /**< EOLn(text) + OI(). */
    void EOOO(const char* text, va_list list);    /**< EO(text) + OO(). */
    void EOLnOO(const char* text, va_list list);  /**< EOLn(text) + OO(). */

    void OISOOO(const char* text, va_list list);    /**< OI() + SO(text) + OO(). */
    void OISOLnOO(const char* text, va_list list);  /**< OI() + SOLn(text) + OO(). */
    void OIEOOO(const char* text, va_list list);    /**< OI() + EO(text) + OO(). */
    void OIEOLnOO(const char* text, va_list list);  /**< OI() + EOLn(text) + OO(). */

    int getErrorOutsCount() const;      /**< Gets total count of printouts-with-newline during error-mode. */
    int getSuccessOutsCount() const;    /**< Gets total count of printouts-with-newline during success-mode. */

    CConsole::CConsoleImpl& operator<<(const char* text);
    CConsole::CConsoleImpl& operator<<(const bool& b);
    CConsole::CConsoleImpl& operator<<(const int& n);
    CConsole::CConsoleImpl& operator<<(const float& f);
    CConsole::CConsoleImpl& operator<<(const CConsole::FormatSignal& fs);

protected:

private:
    static const int CCONSOLE_INDENTATION_CHANGE = 2;           /**< Positive, amount of indent/outdent change by Indent()/Outdent(). */
    static const int CCONSOLE_DEF_CLR_FG = 
        FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE;    /**< Foreground color. */

    static const int HTML_CLR_S = 7*sizeof(char);   /**< Size to store 1 HTML-color. */

    static CConsoleImpl consoleImplInstance;

    static int      nErrorOutCount;         /**< Total OLn() during error mode. */
    static int      nSuccessOutCount;       /**< Total OLn() during success mode. */

    // ---------------------------------------------------------------------------

    bool bInited;                              /**< False by default, Initialize() sets it to true, Deinitialize() sets it to false. */
    int  nRefCount;                            /**< 0 by default, Initialize() increases it by 1, Deinitialize() decreases it by 1. */
    int  nMode;                                /**< Current mode: 0 if normal, 1 is error, 2 is success (EOn()/EOff()/SOn()/SOff()/RestoreDefaultColors() set this). */
    int  nIndentValue;                         /**< Current indentation. */ 
    bool bFirstWriteTextCallAfterWriteTextLn;  /**< True if we are at the 1st no-new-line-print after a new-line-print. */
    
    HANDLE hConsole;                    /**< Console output handle. */
    
    WORD clrFG, clrBG;                  /**< Current foreground and background colors. */
    char clrFGhtml[HTML_CLR_S];         /**< Current foreground html color. */
    WORD clrInts,
         clrFloats,
         clrStrings,
         clrBools;                      /**< Current colors for ints, strings, floats and bools. */
    char clrIntsHtml[HTML_CLR_S],
         clrFloatsHtml[HTML_CLR_S],
         clrStringsHtml[HTML_CLR_S],
         clrBoolsHtml[HTML_CLR_S];      /**< Current html colors for ints, strings, floats and bools. */
    WORD dLastFGColor;                  /**< Saved foreground color. */
    char dLastFGColorHtml[HTML_CLR_S];  /**< Saved html foreground color. */
    WORD dLastIntsColor,
         dLastFloatsColor,
         dLastStringsColor,
         dLastBoolsColor;                   /**< Saved colors for ints, strings, floats and bools. */
    char dLastIntsColorHtml[HTML_CLR_S],
         dLastFloatsColorHtml[HTML_CLR_S],
         dLastStringsColorHtml[HTML_CLR_S],
         dLastBoolsColorHtml[HTML_CLR_S];   /**< Saved html colors for ints, strings, floats and bools. */

    char  vmi[80];                   /**< Temp, always used for the current printout. */
    DWORD wrt;                       /**< Temp, always used for the current printout. */
    WORD  oldClrFG;                  /**< Temp, always used for the current printout. */
    char  oldClrFGhtml[HTML_CLR_S];  /**< Temp, always used for the current printout. */

    std::ofstream fLog;
    bool bAllowLogFile;

    std::string loggerName;                /**< Name of the current logger module that last invoked getConsoleInstance(). */
    std::set<std::string> enabledModules;  /**< Contains logger module names for which logging is enabled. */
    bool        bErrorsAlwaysOn;           /**< Should module error logs always appear or not. */

    // ---------------------------------------------------------------------------

    CConsoleImpl();

    CConsoleImpl(const CConsoleImpl&);
    CConsoleImpl& operator=(const CConsoleImpl&);

    virtual ~CConsoleImpl();

    bool canWeWriteBasedOnFilterSettings() const;

    void ImmediateWriteString(const char* text);    /**< Directly writes formatted string value to the console. */
    void ImmediateWriteBool(bool b);                /**< Directly writes formatted boolean value to the console. */
    void ImmediateWriteInt(int n);                  /**< Directly writes formatted signed integer value to the console. */
    void ImmediateWriteUInt(unsigned int n);        /**< Directly writes formatted unsigned integer value to the console. */
    void ImmediateWriteFloat(float f);              /**< Directly writes formatted floating-point value to the console. */
    
    void WriteText(const char* text);             /**< Directly writes unformatted text to the console. */
    void WriteFormattedTextEx(
        const char* fmt, va_list list);           /**< Writes text to the console. */

    void WriteFormattedTextExCaller(
        const char* fmt, va_list list, bool nl);  /**< Writes text to the console. */

    friend class CConsole;

}; // class CConsoleImpl



// ############################### PUBLIC ################################


/**
    Sets logging on or off for the given logger module.
    By default logging is NOT enabled for any logger modules.
    Initially logging can be done only with empty loggerModule name.
    For specific modules that invoke getConsoleInstance() with their module name, logging
    state must be enabled in order to make their logs actually appear.

    @param loggerModule Name of the logger who wants to change its logging state.
                        If this is "4LLM0DUL3S", the given state turns full verbose logging on or off, regardless of any other logging state.
    @param state True to enable logging of the loggerModule, false to disable.
*/
void CConsole::CConsoleImpl::SetLoggingState(const char* loggerModule, bool state)
{
    if ( !bInited )
        return;

    const size_t sizeOfLoggerModuleNameBuffer = sizeof(char) * (strlen(loggerModule) + 1);
    char* const newNameLoggerModule = (char* const)malloc(sizeOfLoggerModuleNameBuffer);
    if (newNameLoggerModule == nullptr)
        return;

    strncpy_s(newNameLoggerModule, sizeOfLoggerModuleNameBuffer, loggerModule, sizeOfLoggerModuleNameBuffer);
    PFL::strClr(newNameLoggerModule);
    if (strlen(newNameLoggerModule) == 0)
    {
        free(newNameLoggerModule);
        return;
    }
    free(newNameLoggerModule);

    if (state)
    {
        enabledModules.insert(loggerModule);
    }
    else
    {
        enabledModules.erase(loggerModule);
    }
} // SetLoggingState 


/**
    Sets errors always appear irrespective of logging state of current logger module.
    Default value is true.

    @param state True will make module error logs appear even if module logging state is false for the current module.
                 False will let module errors logs be controlled purely by module logging states.
*/
void CConsole::CConsoleImpl::SetErrorsAlwaysOn(bool state)
{
    if ( !bInited )
        return;

    bErrorsAlwaysOn = state;
} // SetErrorsAlwaysOn()


/**
    Gets the current indentation.
*/
int CConsole::CConsoleImpl::getIndent() const
{
    if ( !bInited )
        return 0;

    return nIndentValue;
} // getIndent()


/**
    Sets the current indentation.
*/
void CConsole::CConsoleImpl::SetIndent(int value)
{
    if ( !bInited )
        return;

    nIndentValue = value;
    if (nIndentValue < 0)
        nIndentValue = 0;
} // SetIndent()


/**
    Increases indentation.
*/
void CConsole::CConsoleImpl::Indent()
{
    if ( !bInited )
        return;

    nIndentValue += CConsoleImpl::CCONSOLE_INDENTATION_CHANGE;
} // Indent()


/**
    Increases indentation by the given value.
*/
void CConsole::CConsoleImpl::IndentBy(int value)
{
    if ( !bInited )
        return;

    nIndentValue += value;
    if (nIndentValue < 0)
        nIndentValue = 0;
} // IndentBy()


/**
    Decreases indentation.
*/
void CConsole::CConsoleImpl::Outdent()
{
    if ( !bInited )
        return;

    nIndentValue -= CConsoleImpl::CCONSOLE_INDENTATION_CHANGE;
    if (nIndentValue < 0)
        nIndentValue = 0;
} // Outdent()


/**
    Decreases indentation by the given value.
*/
void CConsole::CConsoleImpl::OutdentBy(int value)
{
    if ( !bInited )
        return;

    nIndentValue -= value;
    if (nIndentValue < 0)
        nIndentValue = 0;
} // OutdentBy()


/**
    Loads previously saved colors.
*/
void CConsole::CConsoleImpl::LoadColors()
{
    if ( !bInited )
        return;

    SetFGColor(dLastFGColor, dLastBoolsColorHtml);
    SetStringsColor(dLastStringsColor, dLastStringsColorHtml);
    SetFloatsColor(dLastFloatsColor, dLastFloatsColorHtml);
    SetIntsColor(dLastIntsColor, dLastIntsColorHtml);
    SetBoolsColor(dLastBoolsColor, dLastBoolsColorHtml);
} // LoadColors()


/**
    Saves current colors.
*/
void CConsole::CConsoleImpl::SaveColors()
{
#ifdef CCONSOLE_IS_ENABLED
    if (!bInited)
        return;

    dLastFGColor = clrFG;
    dLastStringsColor = clrStrings;
    dLastFloatsColor = clrFloats;
    dLastIntsColor = clrInts;
    dLastBoolsColor = clrBools;
    strcpy(dLastFGColorHtml, clrFGhtml);
    strcpy(dLastStringsColorHtml, clrStringsHtml);
    strcpy(dLastFloatsColorHtml, clrFloatsHtml);
    strcpy(dLastIntsColorHtml, clrIntsHtml);
    strcpy(dLastBoolsColorHtml, clrBoolsHtml);
#endif
}


/**
    Restores default colors.
*/
void CConsole::CConsoleImpl::RestoreDefaultColors()
{
#ifdef CCONSOLE_IS_ENABLED
    if (!bInited)
        return;

    nMode = 0;
    clrFG = CConsoleImpl::CCONSOLE_DEF_CLR_FG;
    clrBG = 0;
    clrInts = clrFG;
    clrFloats = clrFG;
    clrStrings = clrFG;
    clrBools = clrFG;
    strcpy(clrFGhtml, "999999");
    strcpy(clrIntsHtml, clrFGhtml);
    strcpy(clrFloatsHtml, clrFGhtml);
    strcpy(clrStringsHtml, clrFGhtml);
    strcpy(clrBoolsHtml, clrFGhtml);
#endif
}


/**
    Gets foreground color.
*/
WORD CConsole::CConsoleImpl::getFGColor() const
{
    if ( !bInited )
        return 0;

    return clrFG;
} // getFGColor()


/**
    Gets html foreground color.
*/
const char* CConsole::CConsoleImpl::getFGColorHtml() const
{
    if ( !bInited )
        return "#DDBEEF";

    return clrFGhtml;
} // getFGColorHtml()


/**
    Sets foreground color.
*/
void CConsole::CConsoleImpl::SetFGColor(WORD clr, const char* html)
{
    if ( !bInited )
        return;

    clrFG = clr;
    SetConsoleTextAttribute(hConsole, clrFG | clrBG);
    if (html)
        strcpy_s(clrFGhtml, CConsoleImpl::HTML_CLR_S, html);
} // SetFGColor()


/**
    Gets background color.
*/
WORD CConsole::CConsoleImpl::getBGColor() const
{
    if ( !bInited )
        return 0;

    return clrBG;
} // getBGColor()


/**
    Sets background color.
*/
void CConsole::CConsoleImpl::SetBGColor(WORD clr)
{
    if ( !bInited )
        return;

    clrBG = clr;
    SetConsoleTextAttribute(hConsole, clrFG | clrBG);
} // SetBGColor()


/**
    Gets ints color.
*/
WORD CConsole::CConsoleImpl::getIntsColor() const
{
    if ( !bInited )
        return 0;

    return clrInts;
} // getIntsColor()


/**
    Gets ints html color.
*/
const char* CConsole::CConsoleImpl::getIntsColorHtml() const
{
    if ( !bInited )
        return "#DDBEEF";

    return clrIntsHtml;
} // getIntsColorHtml()


/**
    Sets ints color.
*/
void CConsole::CConsoleImpl::SetIntsColor(WORD clr, const char* html)
{
    if ( !bInited )
        return;

    clrInts = clr;
    if (html)
        strcpy_s(clrIntsHtml, CConsoleImpl::HTML_CLR_S, html);
} // SetIntsColor()


/**
    Gets strings color.
*/
WORD CConsole::CConsoleImpl::getStringsColor() const
{
    if ( !bInited )
        return 0;

    return clrStrings;
} // getStringsColor()


/**
    Gets strings html color.
*/
const char* CConsole::CConsoleImpl::getStringsColorHtml() const
{
    if ( !bInited )
        return "#DDBEEF";

    return clrStringsHtml;
} // getStringsColorHtml()


/**
    Sets strings color.
*/
void CConsole::CConsoleImpl::SetStringsColor(WORD clr, const char* html)
{
    if ( !bInited )
        return;

    clrStrings = clr;
    if (html)
        strcpy_s(clrStringsHtml, CConsoleImpl::HTML_CLR_S, html);
} // SetStringsColor()


/**
    Gets floats color.
*/
WORD CConsole::CConsoleImpl::getFloatsColor() const
{
    if ( !bInited )
        return 0;

    return clrFloats;
} // getFloatsColor()


/**
    Gets floats html color.
*/
const char* CConsole::CConsoleImpl::getFloatsColorHtml() const
{
    if ( !bInited )
        return "#DDBEEF";

    return clrFloatsHtml;
} // getFloatsColorHtml()


/**
    Sets floats color.
*/
void CConsole::CConsoleImpl::SetFloatsColor(WORD clr, const char* html)
{
    if ( !bInited )
        return;

    clrFloats = clr;
    if (html)
        strcpy_s(clrFloatsHtml, CConsoleImpl::HTML_CLR_S, html);
} // SetFloatsColor()


/**
    Gets bools color.
*/
WORD CConsole::CConsoleImpl::getBoolsColor() const
{
    if ( !bInited )
        return 0;

    return clrBools;
} // getBoolsColor()


/**
    Gets bools html color.
*/
const char* CConsole::CConsoleImpl::getBoolsColorHtml() const
{
    if ( !bInited )
        return "#DDBEEF";

    return clrBoolsHtml;
} // getBoolsColorHtml()


/**
    Sets bools color.
*/
void CConsole::CConsoleImpl::SetBoolsColor(WORD clr, const char* html)
{
    if ( !bInited )
        return;

    clrBools = clr;
    if (html)
        strcpy_s(clrBoolsHtml, CConsoleImpl::HTML_CLR_S, html);
} // SetBoolsColor()


/**
    Prints text to console.
*/
void CConsole::CConsoleImpl::O(const char* text, va_list list)
{
    if ( !bInited )
        return;

    WriteFormattedTextExCaller(text, list, false);
} // O()


/**
    Prints text to console and adds a new line.
*/
void CConsole::CConsoleImpl::OLn(const char* text, va_list list)
{
    if ( !bInited )
        return;

    WriteFormattedTextExCaller(text, list, true);
} // OLn()


/**
    Prints text to console and adds a new line.
*/
void CConsole::CConsoleImpl::OLn(const char* text, ...)
{
    if (!bInited)
        return;

    va_list list;
    va_start(list, text);
    OLn(text, list);
    va_end(list);
} // OLn()


/**
    Indent().
*/
void CConsole::CConsoleImpl::OI()
{
    if ( !bInited )
        return;

    Indent();
} // OI()


/**
    OI() + O(text).
*/
void CConsole::CConsoleImpl::OIO(const char* text, va_list list)
{
    if ( !bInited )
        return;

    OI();
    WriteFormattedTextExCaller(text, list, false);
} // OI()


/**
    OI() + OLn(text).
*/
void CConsole::CConsoleImpl::OIOLn(const char* text, va_list list)
{
    if ( !bInited )
        return;

    OI();
    WriteFormattedTextExCaller(text, list, true);
} // OIOLn()


/**
    OLn(text) + OI().
*/
void CConsole::CConsoleImpl::OLnOI(const char* text, va_list list)
{
    if ( !bInited )
        return;

    WriteFormattedTextExCaller(text, list, true);
    OI();
} // OLnOI()


/**
    IndentBy().
*/
void CConsole::CConsoleImpl::OIb(int value)
{
    if ( !bInited )
        return;

    IndentBy(value);
} // OIb()


/**
    Outdent().
*/
void CConsole::CConsoleImpl::OO()
{
    if ( !bInited )
        return;

    Outdent();
} // OO()


/**
    OO() + O(text).
*/
void CConsole::CConsoleImpl::OOO(const char* text, va_list list)
{
    if ( !bInited )
        return;

    OO();
    WriteFormattedTextExCaller(text, list, false);
} // OO()


/**
    OO() + OLn(text).
*/
void CConsole::CConsoleImpl::OOOLn(const char* text, va_list list)
{
    if ( !bInited )
        return;

    OO();
    WriteFormattedTextExCaller(text, list, true);
} // OOLn()


/**
    OLn(text) + OO().
*/
void CConsole::CConsoleImpl::OLnOO(const char* text, va_list list)
{
    if ( !bInited )
        return;

    WriteFormattedTextExCaller(text, list, true);
    OO();
} // OLnOI()


/**
    OutdentBy().
*/
void CConsole::CConsoleImpl::OOb(int value)
{
    if ( !bInited )
        return;

    OutdentBy(value);
} // OOb()


/**
    OI() + OLn(text) + OO().
*/
void CConsole::CConsoleImpl::OIOLnOO(const char* text, va_list list)
{
    if ( !bInited )
        return;

    OI();
    WriteFormattedTextExCaller(text, list, true);
    OO();
}


/**
    Prints line to console and adds a new line.
    @param n How many times the "-=" chars should be repeated.
*/
void CConsole::CConsoleImpl::L(int n)
{
    if ( !bInited )
        return;

    for (int i = 0; i < n; i++)
        WriteText("-=");
    WriteText("\n\r");
} // L()


/**
    Normal-mode on.
*/
void CConsole::CConsoleImpl::NOn()
{
    if ( !bInited )
        return;

    nMode = 0;
    LoadColors();
} // NOn()


/**
    Error-mode on.
*/
void CConsole::CConsoleImpl::EOn()
{
    if ( !bInited )
        return;

    if (nMode == 1)
        return;
    else if (nMode == 2)
        SOff();

    nMode = 1;
    SaveColors();
    SetFGColor(FOREGROUND_RED | FOREGROUND_INTENSITY, "FF0000");
    SetStringsColor(FOREGROUND_RED | FOREGROUND_GREEN, "DDDD00");
    SetFloatsColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "FFFF00");
    SetIntsColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "FFFF00");
    SetBoolsColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "FFFF00");
} // EOn()


/**
    Error-mode  off.
*/
void CConsole::CConsoleImpl::EOff()
{
    if ( !bInited )
        return;

    NOn();
} // EOff()


/**
    Success-mode on.
*/
void CConsole::CConsoleImpl::SOn()
{
    if ( !bInited )
        return;

    if (nMode == 2)
        return;
    else if (nMode == 1)
        EOff();

    nMode = 2;
    SaveColors();
    SetFGColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY, "00FF00");
    SetStringsColor(FOREGROUND_GREEN, "00DD00");
    SetFloatsColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "FFFF00");
    SetIntsColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "FFFF00");
    SetBoolsColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "FFFF00");
} // SOn()


/**
    Success-mode off.
*/
void CConsole::CConsoleImpl::SOff()
{
    if ( !bInited )
        return;

    NOn();
} // SOff()


/**
    SOn() + O(text) + SOff().
*/
void CConsole::CConsoleImpl::SO(const char* text, va_list list)
{
    if ( !bInited )
        return;

    SOn();
    WriteFormattedTextExCaller(text, list, false);
    SOff();
} // SO()


/**
    SOn() + OLn(text) + SOff().
*/
void CConsole::CConsoleImpl::SOLn(const char* text, va_list list)
{
    if ( !bInited )
        return;

    SOn();
    WriteFormattedTextExCaller(text, list, true);
    SOff();
} // SOln()


/**
    SOn() + OLn(text) + SOff().
*/
void CConsole::CConsoleImpl::SOLn(const char* text, ...)
{
    if (!bInited)
        return;

    va_list list;
    va_start(list, text);
    SOLn(text, list);
    va_end(list);
} // SOLn()


/**
    EOn() + O(text) + EOff().
*/
void CConsole::CConsoleImpl::EO(const char* text, va_list list)
{
    if ( !bInited )
        return;

    EOn();
    WriteFormattedTextExCaller(text, list, false);
    EOff();
} // EO()


/**
    EOn() + OLn(text) + EOff().
*/
void CConsole::CConsoleImpl::EOLn(const char* text, va_list list)
{
    if ( !bInited )
        return;

    EOn();
    WriteFormattedTextExCaller(text, list, true);
    EOff();
} // EOLn()


/**
    EOn() + OLn(text) + EOff().
*/
void CConsole::CConsoleImpl::EOLn(const char* text, ...)
{
    if (!bInited)
        return;

    va_list list;
    va_start(list, text);
    EOLn(text, list);
    va_end(list);
} // SOLn()


/**
    OI() + SO(text).
*/
void CConsole::CConsoleImpl::OISO(const char* text, va_list list)
{
    if ( !bInited )
        return;

    OI();
    SOn();
    WriteFormattedTextExCaller(text, list, false);
    SOff();
} // OISO()


/**
    OI() + SOLn(text).
*/
void CConsole::CConsoleImpl::OISOLn(const char* text, va_list list)
{
    if ( !bInited )
        return;

    OI();
    SOn();
    WriteFormattedTextExCaller(text, list, true);
    SOff();
} // OISOLn()


/**
    OO() + SO(text).
*/
void CConsole::CConsoleImpl::OOSO(const char* text, va_list list)
{
    if ( !bInited )
        return;

    OO();
    SOn();
    WriteFormattedTextExCaller(text, list, false);
    SOff();
} // OOSO()


/**
    OO() + SOLn(text).
*/
void CConsole::CConsoleImpl::OOSOLn(const char* text, va_list list)
{
    if ( !bInited )
        return;

    OO();
    SOn();
    WriteFormattedTextExCaller(text, list, true);
    SOff();
} // OOSOLn()


/**
    OI() + EO(text).
*/
void CConsole::CConsoleImpl::OIEO(const char* text, va_list list)
{
    if ( !bInited )
        return;

    OI();
    EOn();
    WriteFormattedTextExCaller(text, list, false);
    EOff();
} // OIEO()


/**
    OI() + EOLn(text).
*/
void CConsole::CConsoleImpl::OIEOLn(const char* text, va_list list)
{
    if ( !bInited )
        return;

    OI();
    EOn();
    WriteFormattedTextExCaller(text, list, true);
    EOff();
} // OIEOLn()


/**
    OO() + EO(text).
*/
void CConsole::CConsoleImpl::OOEO(const char* text, va_list list)
{
    if ( !bInited )
        return;

    OO();
    EOn();
    WriteFormattedTextExCaller(text, list, false);
    EOff();
} // OOEO()


/**
    OO() + EOLn(text).
*/
void CConsole::CConsoleImpl::OOEOLn(const char* text, va_list list)
{
    if ( !bInited )
        return;

    OO();
    EOn();
    WriteFormattedTextExCaller(text, list, true);
    EOff();
} // OOEOLn()


/**
    SO(text) + OI().
*/
void CConsole::CConsoleImpl::SOOI(const char* text, va_list list)
{
    if ( !bInited )
        return;

    SOn();
    WriteFormattedTextExCaller(text, list, false);
    SOff();
    OI();
} // SOOI()


/**
    SOLn(text) + OI().
*/
void CConsole::CConsoleImpl::SOLnOI(const char* text, va_list list)
{
    if ( !bInited )
        return;

    SOn();
    WriteFormattedTextExCaller(text, list, true);
    SOff();
    OI();
} // SOLnOI()


/**
    SO(text) + OO().
*/
void CConsole::CConsoleImpl::SOOO(const char* text, va_list list)
{
    if ( !bInited )
        return;

    SOn();
    WriteFormattedTextExCaller(text, list, false);
    SOff();
    OO();
} // SOOO()


/**
    SOLn(text) + OO().
*/
void CConsole::CConsoleImpl::SOLnOO(const char* text, va_list list)
{
    if ( !bInited )
        return;

    SOn();
    WriteFormattedTextExCaller(text, list, true);
    SOff();
    OO();
} // SOLnOO()


/**
    EO(text) + OI().
*/
void CConsole::CConsoleImpl::EOOI(const char* text, va_list list)
{
    if ( !bInited )
        return;

    EOn();
    WriteFormattedTextExCaller(text, list, false);
    EOff();
    OI();
} // EOOI()


/**
    EOLn(text) + OI().
*/
void CConsole::CConsoleImpl::EOLnOI(const char* text, va_list list)
{
    if ( !bInited )
        return;

    EOn();
    WriteFormattedTextExCaller(text, list, true);
    EOff();
    OI();
} // EOLnOI()


/**
    EO(text) + OO().
*/
void CConsole::CConsoleImpl::EOOO(const char* text, va_list list)
{
    if ( !bInited )
        return;

    EOn();
    WriteFormattedTextExCaller(text, list, false);
    EOff();
    OO();
} // EOOO()


/**
    EOLn(text) + OO().
*/
void CConsole::CConsoleImpl::EOLnOO(const char* text, va_list list)
{
    if ( !bInited )
        return;

    EOn();
    WriteFormattedTextExCaller(text, list, true);
    EOff();
    OO();
} // EOLnOO()


/**
    OI() + SO(text) + OO().
*/
void CConsole::CConsoleImpl::OISOOO(const char* text, va_list list)
{
    if ( !bInited )
        return;

    OI();
    SOn();
    WriteFormattedTextExCaller(text, list, false);
    SOff();
    OO();
} // OISOOO


/**
    OI() + SOLn(text) + OO().
*/
void CConsole::CConsoleImpl::OISOLnOO(const char* text, va_list list)
{
    if ( !bInited )
        return;

    OI();
    SOn();
    WriteFormattedTextExCaller(text, list, true);
    SOff();
    OO();
} // OISOLnOO


/**
    OI() + EO(text) + OO().
*/
void CConsole::CConsoleImpl::OIEOOO(const char* text, va_list list)
{
    if ( !bInited )
        return;

    OI();
    EOn();
    WriteFormattedTextExCaller(text, list, false);
    EOff();
    OO();
} // OIEOOO


/**
    OI() + EOLn(text) + OO().
*/
void CConsole::CConsoleImpl::OIEOLnOO(const char* text, va_list list)
{
    if ( !bInited )
        return;

    OI();
    EOn();
    WriteFormattedTextExCaller(text, list, true);
    EOff();
    OO();
} // OIEOLnOO


/**
    Gets total count of printouts-with-newline during error-mode.
*/
int CConsole::CConsoleImpl::getErrorOutsCount() const
{
    if ( !bInited )
        return 0;

    return nErrorOutCount;
} // getErrorOutsCount()


/**
    Gets total count of printouts-with-newline during success-mode.
*/
int CConsole::CConsoleImpl::getSuccessOutsCount() const
{
    if ( !bInited )
        return 0;

    return nSuccessOutCount;
} // getSuccessOutsCount()


CConsole::CConsoleImpl& CConsole::CConsoleImpl::operator<<(const char* text)
{
    if ( !bInited )
        return *this;

    if (bFirstWriteTextCallAfterWriteTextLn)
        for (int i = 0; i < nIndentValue; i++)
            WriteText(" ");
    ImmediateWriteString(text);
    return *this;
} // operator<<()

CConsole::CConsoleImpl& CConsole::CConsoleImpl::operator<<(const bool& b)
{
    if ( !bInited )
        return *this;

    if (bFirstWriteTextCallAfterWriteTextLn)
        for (int i = 0; i < nIndentValue; i++)
            WriteText(" ");
    ImmediateWriteBool(b);
    return *this;
} // operator<<()

CConsole::CConsoleImpl& CConsole::CConsoleImpl::operator<<(const int& n)
{
    if ( !bInited )
        return *this;

    if (bFirstWriteTextCallAfterWriteTextLn)
        for (int i = 0; i < nIndentValue; i++)
            WriteText(" ");
    ImmediateWriteInt(n);
    return *this;
} // operator<<()

CConsole::CConsoleImpl& CConsole::CConsoleImpl::operator<<(const float& f)
{
    if ( !bInited )
        return *this;

    if (bFirstWriteTextCallAfterWriteTextLn)
        for (int i = 0; i < nIndentValue; i++)
            WriteText(" ");
    ImmediateWriteFloat(f);
    return *this;
} // operator<<()

CConsole::CConsoleImpl& CConsole::CConsoleImpl::operator<<(const CConsole::FormatSignal& fs)
{
    if ( !bInited )
        return *this;

    switch (fs)
    {
    case NL: WriteText("\n\r"); break;
    case  S: SOn(); break;
    case  E: EOn(); break;
    default: NOn(); break;
    }
    return *this;
} // operator<<()


// ############################## PROTECTED ##############################


// ############################### PRIVATE ###############################


CConsole::CConsoleImpl CConsole::CConsoleImpl::consoleImplInstance;
int CConsole::CConsoleImpl::nErrorOutCount = 0;     /**< Total OLn() during error mode. */
int CConsole::CConsoleImpl::nSuccessOutCount = 0;   /**< Total OLn() during success mode. */


CConsole::CConsoleImpl::CConsoleImpl()
{
    hConsole = NULL;
    nRefCount = 0;
    bInited = false;
    bErrorsAlwaysOn = true;
    nIndentValue = 0;
    bFirstWriteTextCallAfterWriteTextLn = true;
    memset(clrFGhtml, 0, HTML_CLR_S);
    memset(clrStringsHtml, 0, HTML_CLR_S);
    memset(clrIntsHtml, 0, HTML_CLR_S);
    memset(clrFloatsHtml, 0, HTML_CLR_S);
    memset(clrBoolsHtml, 0, HTML_CLR_S);
    memset(dLastFGColorHtml, 0, HTML_CLR_S);
    memset(dLastStringsColorHtml, 0, HTML_CLR_S);
    memset(dLastIntsColorHtml, 0, HTML_CLR_S);
    memset(dLastFloatsColorHtml, 0, HTML_CLR_S);
    memset(dLastBoolsColorHtml, 0, HTML_CLR_S);
    RestoreDefaultColors();
    SaveColors();
} // CConsoleImpl(...)


CConsole::CConsoleImpl::CConsoleImpl(const CConsoleImpl&)
{

}


CConsole::CConsoleImpl& CConsole::CConsoleImpl::operator=(const CConsoleImpl&)
{
    return *this;
}


CConsole::CConsoleImpl::~CConsoleImpl()
{
#ifdef CCONSOLE_IS_ENABLED
    FreeConsole();
    if ( bAllowLogFile )
    {
        fLog << "</font>" << endl;
        fLog << "</body>" << endl;
        fLog << "</html>" << endl;
        fLog.close();
    }
#endif
} // ~CConsoleImpl()


bool CConsole::CConsoleImpl::canWeWriteBasedOnFilterSettings() const
{
    if ( loggerName.empty() )
    {
        return true;
    }

    // magic module name for turning on all logging
    auto it = enabledModules.find("4LLM0DUL3S");
    if ( it != enabledModules.end() )
    {
        return true;
    }

    it = enabledModules.find(loggerName);
    if ( it != enabledModules.end() )
    {
        return true;
    }

    if ( bErrorsAlwaysOn && (nMode == 1) )
    {
        return true;
    }

    return false;
} // canWeWriteBasedOnFilterSettings()


/**
    Directly writes formatted string value to the console.
    Used by WriteFormattedTextEx() and operator<<()s.
*/
void CConsole::CConsoleImpl::ImmediateWriteString(const char* text)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !canWeWriteBasedOnFilterSettings() )
        return;

    oldClrFG = clrFG;
    //oldClrFGhtml = clrFGhtml;
    if ( text != NULL )
    {
        SetFGColor(clrStrings);                             
        WriteConsoleA(hConsole, text, strlen(text), &wrt, 0);
        if ( bAllowLogFile )
            fLog << "<font color=\"#" << clrStringsHtml << "\">" << text << "</font>";
    }
    else
    {
        SetFGColor(oldClrFG);
        WriteConsoleA(hConsole, "NULL", 4, &wrt, 0);
        if ( bAllowLogFile )
            fLog << "<font color=\"#" << clrStringsHtml << "\">NULL</font>";
    }
    SetFGColor(oldClrFG);
#endif
} // ImmediateWriteString()


/**
    Directly writes formatted boolean value to the console.
    Used by WriteFormattedTextEx() and operator<<()s.
*/
void CConsole::CConsoleImpl::ImmediateWriteBool(bool l)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !canWeWriteBasedOnFilterSettings() )
        return;

    oldClrFG = clrFG;
    SetFGColor(clrBools);
    WriteConsoleA(hConsole, l ? "true" : "false", l ? 4 : 5, &wrt, 0);
    if ( bAllowLogFile )
        fLog << "<font color=\"#" << clrBoolsHtml << "\">" << (l ? "true" : "false") << "</font>";
    SetFGColor(oldClrFG);
#endif
} // ImmediateWriteBool()


/**
    Directly writes formatted signed integer value to the console.
    Used by WriteFormattedTextEx() and operator<<()s.
*/
void CConsole::CConsoleImpl::ImmediateWriteInt(int n)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !canWeWriteBasedOnFilterSettings() )
        return;

    oldClrFG = clrFG;
    SetFGColor(clrInts);
    itoa(n,vmi,10);
    WriteConsoleA(hConsole, vmi, strlen(vmi), &wrt, 0);
    if ( bAllowLogFile )
        fLog << "<font color=\"#" << clrIntsHtml << "\">" << vmi << "</font>";
    SetFGColor(oldClrFG);
#endif
} // ImmediateWriteInt()


/**
    Directly writes formatted unsigned integer value to the console.
    Used by WriteFormattedTextEx() and operator<<()s.
*/
void CConsole::CConsoleImpl::ImmediateWriteUInt(unsigned int n)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !canWeWriteBasedOnFilterSettings() )
        return;

    oldClrFG = clrFG;
    SetFGColor(clrInts);
    sprintf(vmi, "%u", n);
    WriteConsoleA(hConsole, vmi, strlen(vmi), &wrt, 0);
    if ( bAllowLogFile )
        fLog << "<font color=\"#" << clrIntsHtml << "\">" << vmi << "</font>";
    SetFGColor(oldClrFG);
#endif
} // ImmediateWriteUInt()


/**
    Directly writes formatted floating-point value to the console.
    Used by WriteFormattedTextEx() and operator<<()s.
*/
void CConsole::CConsoleImpl::ImmediateWriteFloat(float f)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !canWeWriteBasedOnFilterSettings() )
        return;

    oldClrFG = clrFG;
    sprintf(vmi, "%0.4f", f);
    const size_t nOriginalLen = strlen(vmi);
    size_t newlen = strlen(vmi);
    for (size_t blah = strlen(vmi); (vmi[blah] == '0') || (vmi[blah] == 0); blah--)
    {
        newlen--;
    }
    if (newlen < nOriginalLen)
    {
        newlen++;
    }
    vmi[newlen] = '\0';

    SetFGColor(clrFloats);
    WriteConsoleA(hConsole, vmi, newlen, &wrt, 0);
    if (bAllowLogFile)
    {
        fLog << "<font color=\"#" << clrFloatsHtml << "\">" << vmi << "</font>";
    }
    SetFGColor(oldClrFG);
#endif
} // ImmediateWriteFloat()


/**
    Directly writes unformatted text to the console.
    Used by WriteFormattedTextEx(), WriteFormattedTextExCaller(), L() and operator<<()s.
*/
void CConsole::CConsoleImpl::WriteText(const char* text)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !canWeWriteBasedOnFilterSettings() )
        return;

    oldClrFG = clrFG;
    clrFG = clrStrings;
    WriteConsoleA(hConsole, text, strlen(text), &wrt, 0);
    if ( bAllowLogFile )
    {
        if ( strcmp("\n\r", text) == 0 )
            fLog << "<br>" << endl;
        else
        {
            string tmpSpacesString;
            tmpSpacesString = "";
            unsigned int numspaces = PFL::numCharAppears(' ', text, strlen(text));
            if ( numspaces == strlen(text) )
            {
                for (unsigned int j = 0; j < numspaces; j++)
                    tmpSpacesString += "&nbsp;";
                fLog << tmpSpacesString.c_str();
            }
            else
                fLog << text;
        }
    }
    bFirstWriteTextCallAfterWriteTextLn = ( strstr(text, "\n") != NULL );
    clrFG = oldClrFG;
#endif
} // WriteText()


/**
    Writes text to the console.
    Shouldn't be called from outside but only from a higher-level function with variable-length argument list.
    @param fmt  The text to be printed to the console, may contain formatting chars.
    @param list The list of arguments passed from the calling higher-level function.
*/
void CConsole::CConsoleImpl::WriteFormattedTextEx(const char* fmt, va_list list)
{
#ifdef CCONSOLE_IS_ENABLED
    const char *p, *r;
    int   e;
    unsigned int ue;
    bool  l;
    float f;                                                                            

    if ( bFirstWriteTextCallAfterWriteTextLn )
        for (int i = 0; i < nIndentValue; i++)
            WriteText(" ");
    
    oldClrFG = clrFG;
    if ( bAllowLogFile )
        if ( nMode != 0 )
            fLog << "<font color=\"#" << clrFGhtml << "\">";

    if ( strstr(fmt, "%") == NULL )
    {
        SetFGColor(oldClrFG);
        WriteText(fmt);
    }
    else
    {
        for (p = fmt; *p; ++p)
        {
            if ( *p != '%' )
            {
                SetFGColor(oldClrFG);
                WriteConsoleA(hConsole, p, sizeof(char), &wrt, 0);
                if ( bAllowLogFile )
                    fLog << *p;
            }
            else
            {
                switch ( *++p )
                {
                case 's':
                    {
                        r = va_arg(list, char*);
                        ImmediateWriteString(r);
                        continue;
                    }
                case 'i':
                case 'd':
                    {
                        e = va_arg(list, int);
                        ImmediateWriteInt(e);
                        continue;
                    }
                case 'u':
                    {
                        ue = va_arg(list, unsigned int);
                        ImmediateWriteUInt(ue);
                        continue;
                    }
                case 'b':
                    {
                        l = va_arg(list, bool);
                        ImmediateWriteBool(l);
                        continue;
                    }
                case 'f':
                    {                                                        
                        f = (float) va_arg(list, double);
                        ImmediateWriteFloat(f);
                        continue;
                    }
                default:
                    {
                        SetFGColor(oldClrFG);
                        WriteConsoleA(hConsole, p, sizeof(char), &wrt, 0);
                        if ( bAllowLogFile )
                            fLog << *p;
                    }
                } // switch
            } // else
        } // for p
    } // else
    bFirstWriteTextCallAfterWriteTextLn = ( strstr(fmt, "\n") != NULL );
    SetFGColor(oldClrFG);
    if ( bAllowLogFile )
        if ( nMode != 0 )
            fLog << "</font>";
#endif
} // WriteFormattedTextEx()


/**
    Writes text to the console.
    Shouldn't be called from outside but only from a higher-level function with variable-length argument list.
    @param fmt  The text to be printed to the console, may contain formatting chars.
    @param list The list of arguments passed from the calling higher-level function.
    @param nl   Whether to print newline after the text or not. This also activates success/error counting.
*/
void CConsole::CConsoleImpl::WriteFormattedTextExCaller(const char* fmt, va_list list, bool nl)
{           
    if ( !canWeWriteBasedOnFilterSettings() )
        return;

    WriteFormattedTextEx(fmt, list);
    if ( nl )
    {
        WriteText("\n\r");
        if ( nMode == 1 )
            nErrorOutCount++;
        else if ( nMode == 2 )
            nSuccessOutCount++;
    }
} // WriteFormattedTextExCaller()


/*
   CConsole
   ###########################################################################
*/


// ############################### PUBLIC ################################


/**
    Gets the singleton instance.
    @param logger Name of the logger who wants to use the singleton instance.
           It is recommended to use the same name for same entity, because this logger name
           is the basis of per-module filtering.
    @return The singleton instance pre-set for the logger specified.
*/
CConsole& CConsole::getConsoleInstance(const char* loggerModule)
{
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( consoleInstance.consoleImpl && loggerModule ) {
        consoleInstance.consoleImpl->loggerName = loggerModule;
    }
    return consoleInstance;
} // getConsoleInstance()


/**
    Sets logging on or off for the given logger module.
    By default logging is NOT enabled for any logger modules.
    Initially logging can be done only with empty loggerModule name.
    For specific modules that invoke getConsoleInstance() with their module name, logging
    state must be enabled in order to make their logs actually appear.

    @param loggerModule Name of the logger who wants to change its logging state.
                        If this is "4LLM0DUL3S", the given state turns full verbose logging on or off, regardless of any other logging state.
    @param state True to enable logging of the loggerModule, false to disable.
*/
void CConsole::SetLoggingState(const char* loggerModule, bool state)
{
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    consoleImpl->SetLoggingState(loggerModule, state);
} // SetLoggingState 


/**
    Sets errors always appear irrespective of logging state of current logger module.
    Default value is true.

    @param state True will make module error logs appear even if module logging state is false for the current module.
                 False will let module errors logs be controlled purely by module logging states.
*/
void CConsole::SetErrorsAlwaysOn(bool state)
{
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    consoleImpl->SetErrorsAlwaysOn(state);
} // SetErrorsAlwaysOn()


/**
    This creates actually the console window if not created yet.
    An internal reference count is also increased by 1. Reference count explanation is described at Deinitialize().
*/
void CConsole::Initialize(const char* title, bool createLogFile)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !consoleImpl )
    {
        try
        {
            consoleImpl = new CConsoleImpl();
        }
        catch (const std::bad_alloc&)
        {
            return;
        }
    }

    // we need to initialize only once, but refcount always needs to be incremented
    consoleImpl->nRefCount++;

    if ( !(consoleImpl->bInited) )
    {
        if ( !AllocConsole() )
        {
            return;
        }

        // hack to let logs of this initialize function pass thru 
        const std::string prevLoggerName = consoleImpl->loggerName;
        consoleImpl->loggerName = "";
        
        consoleImpl->bInited = true;
        consoleImpl->nErrorOutCount = 0;
        consoleImpl->nSuccessOutCount = 0;
        SetConsoleTitleA( title );
        if ( NULL != (consoleImpl->hConsole = GetStdHandle( STD_OUTPUT_HANDLE )) )
        {
            COORD crd;
            crd.X = 80;
            crd.Y = 10000;
            SetConsoleScreenBufferSize(consoleImpl->hConsole, crd);
        }
        consoleImpl->RestoreDefaultColors();
        consoleImpl->bAllowLogFile = createLogFile;
        if ( createLogFile )
        {
            const auto time = std::time(nullptr);
            char fLogFilename[100];
            if ( 0 == std::strftime(fLogFilename, sizeof(fLogFilename), "log_%Y-%m-%d_%H-%M-%S.html", std::gmtime(&time)) )
            {
                consoleImpl->bAllowLogFile = false;
                consoleImpl->EOLn("ERROR: Couldn't generate file name!");
            }
            else
            {
                consoleImpl->fLog.open(fLogFilename);
                if ( consoleImpl->fLog.fail() )
                {
                    consoleImpl->bAllowLogFile = false;
                    consoleImpl->EOLn("ERROR: Couldn't open output html for writing!");
                }
                else
                {
                    consoleImpl->fLog << "<html>" << endl;
                    consoleImpl->fLog << "<head>" << endl;
                    consoleImpl->fLog << "<title>" << title << "</title>" << endl;
                    consoleImpl->fLog << "</head>" << endl;
                    consoleImpl->fLog << "<body bgcolor=\"#1D1D1D\" text=\"#DDDDDD\">" << endl;
                    consoleImpl->fLog << "<font face=\"Courier\" size=\"2\">" << endl;
                }
            }
        }

        consoleImpl->SOLn(" > CConsole has been initialized with title: %s, refcount: %d!", title, consoleImpl->nRefCount);

        // now we get rid of our hack
        consoleImpl->loggerName = prevLoggerName;

    }
    else
    {
        consoleImpl->SOLn(" > CConsole is already initialized, new refcount: %d!", consoleImpl->nRefCount);
    }
#endif
} // Initialize()


/**
    If reference count is positive, it is decreased by 1.
    If reference count reaches 0, console window gets deleted.
    With this simple reference counting, different parts/layers of a process can invoke Initialize() and Deinitialize()
    at their initializing and deinitializing functions, without the fear of 1 single Deinitialize() call might ruin
    the console functionality of other parts of the process.
*/
void CConsole::Deinitialize()
{
#ifdef CCONSOLE_IS_ENABLED   
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    consoleImpl->nRefCount--;
    //TODO consoleImpl->OLn("CConsole::Deinitialize() new refcount: %d", consoleImpl->nRefCount);
    if ( consoleImpl->nRefCount == 0 )
    {
        this->~CConsole();
    }
#endif
} // Deinitialize()


/**
    Tells if console window is already initialized.
    @return True if console window is initialized, false otherwise.
*/
bool CConsole::isInitialized() const
{
    std::lock_guard<std::mutex> lock(mainMutex);
    return consoleImpl && (consoleImpl->bInited);
}


/**
    Gets the current indentation.
*/
int CConsole::getIndent() const
{
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return 0;

    return consoleImpl->getIndent();
} // getIndent()


/**
    Sets the current indentation.
*/
void CConsole::SetIndent(int value)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    consoleImpl->SetIndent(value);
#endif
} // SetIndent()


/**
    Increases indentation.
*/
void CConsole::Indent()
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    consoleImpl->Indent();
#endif
} // Indent()


/**
    Increases indentation by the given value.
*/
void CConsole::IndentBy(int value)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    consoleImpl->IndentBy(value);
#endif
} // IndentBy()


/**
    Decreases indentation.
*/
void CConsole::Outdent()
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    consoleImpl->Outdent();
#endif
} // Outdent()


/**
    Decreases indentation by the given value.
*/
void CConsole::OutdentBy(int value)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    consoleImpl->OutdentBy(value);
#endif
} // OutdentBy()


/**
    Loads previously saved colors.
*/
void CConsole::LoadColors()
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    consoleImpl->SetFGColor(consoleImpl->dLastFGColor, consoleImpl->dLastBoolsColorHtml);
    consoleImpl->SetStringsColor(consoleImpl->dLastStringsColor, consoleImpl->dLastStringsColorHtml);
    consoleImpl->SetFloatsColor(consoleImpl->dLastFloatsColor, consoleImpl->dLastFloatsColorHtml);
    consoleImpl->SetIntsColor(consoleImpl->dLastIntsColor, consoleImpl->dLastIntsColorHtml);
    consoleImpl->SetBoolsColor(consoleImpl->dLastBoolsColor, consoleImpl->dLastBoolsColorHtml);
#endif
} // LoadColors()


/** 
    Saves current colors.
*/
void CConsole::SaveColors()
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    consoleImpl->SaveColors();
#endif
} // SaveColors()


/**
    Restores default colors.
*/
void CConsole::RestoreDefaultColors()
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    consoleImpl->RestoreDefaultColors();
#endif
} // RestoreDefaultColors()


/**
    Gets foreground color.
*/
WORD CConsole::getFGColor() const
{
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return 0;

    return consoleImpl->getFGColor();
} // getFGColor()


/**
    Gets html foreground color.
*/
const char* CConsole::getFGColorHtml() const
{
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return "#DDBEEF";

    return consoleImpl->getFGColorHtml();
} // getFGColorHtml()


/**
    Sets foreground color.
*/
void CConsole::SetFGColor(WORD clr, const char* html)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    consoleImpl->SetFGColor(clr, html);
#endif
} // SetFGColor()


/**
    Gets background color.
*/
WORD CConsole::getBGColor() const
{
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return 0;

    return consoleImpl->getBGColor();
} // getBGColor()


/**
    Sets background color.
*/
void CConsole::SetBGColor(WORD clr)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    consoleImpl->SetBGColor(clr);
#endif
} // SetBGColor()


/**
    Gets ints color.
*/
WORD CConsole::getIntsColor() const
{
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return 0;

    return consoleImpl->getIntsColor();
} // getIntsColor()


/**
    Gets ints html color.
*/
const char* CConsole::getIntsColorHtml() const
{
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return "#DDBEEF";

    return consoleImpl->getIntsColorHtml();
} // getIntsColorHtml()


/**
    Sets ints color.
*/
void CConsole::SetIntsColor(WORD clr, const char* html)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    consoleImpl->SetIntsColor(clr, html);
#endif
} // SetIntsColor()


/**
    Gets strings color.
*/
WORD CConsole::getStringsColor() const
{
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return 0;

    return consoleImpl->getStringsColor();
} // getStringsColor()


/**
    Gets strings html color.
*/
const char* CConsole::getStringsColorHtml() const
{
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return "#DDBEEF";

    return consoleImpl->getStringsColorHtml();
} // getStringsColorHtml()


/**
    Sets strings color.
*/
void CConsole::SetStringsColor(WORD clr, const char* html)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    consoleImpl->SetStringsColor(clr, html);
#endif
} // SetStringsColor()


/**
    Gets floats color.
*/
WORD CConsole::getFloatsColor() const
{
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return 0;

    return consoleImpl->getFloatsColor();
} // getFloatsColor()


/**
    Gets floats html color.
*/
const char* CConsole::getFloatsColorHtml() const
{
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return "#DDBEEF";

    return consoleImpl->getFloatsColorHtml();
} // getFloatsColorHtml()


/**
    Sets floats color.
*/
void CConsole::SetFloatsColor(WORD clr, const char* html)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    consoleImpl->SetFloatsColor(clr, html);
#endif
} // SetFloatsColor()


/**
    Gets bools color.
*/
WORD CConsole::getBoolsColor() const
{
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return 0;

    return consoleImpl->getBoolsColor();
} // getBoolsColor()


/**
    Gets bools html color.
*/
const char* CConsole::getBoolsColorHtml() const
{
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return "#DDBEEF";

    return consoleImpl->getBoolsColorHtml();
} // getBoolsColorHtml()


/**
    Sets bools color.
*/
void CConsole::SetBoolsColor(WORD clr, const char* html)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    consoleImpl->SetBoolsColor(clr, html);
#endif
} // SetBoolsColor()


/**
    Prints text to console.
*/
void CConsole::O(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    va_list list;
    va_start(list, text);
    consoleImpl->O(text, list);
    va_end(list);
#endif
} // O()


/**
    Prints text to console and adds a new line.
*/
void CConsole::OLn(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    va_list list;
    va_start(list, text);
    consoleImpl->OLn(text, list);
    va_end(list);
#endif
} // OLn()


/**
    Indent().
*/
void CConsole::OI()
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    consoleImpl->OI();
#endif
} // OI()


/**
    OI() + O(text).
*/
void CConsole::OIO(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    va_list list;
    consoleImpl->OI();
    va_start(list, text);
    consoleImpl->OIO(text, list);
    va_end(list);
#endif
} // OI()


/**
    OI() + OLn(text).
*/
void CConsole::OIOLn(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    va_list list;
    consoleImpl->OI();
    va_start(list, text);
    consoleImpl->OIOLn(text, list);
    va_end(list);
#endif
} // OIOLn()


/**
    OLn(text) + OI().
*/
void CConsole::OLnOI(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    va_list list;
    va_start(list, text);
    consoleImpl->OLnOI(text, list);
    va_end(list);
    consoleImpl->OI();
#endif
} // OLnOI()


/**
    IndentBy().
*/
void CConsole::OIb(int value)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    consoleImpl->OIb(value);
#endif
} // OIb()


/**
    Outdent().
*/
void CConsole::OO()
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    consoleImpl->OO();
#endif
} // OO()


/**
    OO() + O(text).
*/
void CConsole::OOO(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    va_list list;
    va_start(list, text);
    consoleImpl->OOO(text, list);
    va_end(list);
#endif
} // OO()


/**
    OO() + OLn(text).
*/
void CConsole::OOOLn(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    va_list list;
    va_start(list, text);
    consoleImpl->OOOLn(text, list);
    va_end(list);
#endif
} // OOLn()


/**
    OLn(text) + OO().
*/
void CConsole::OLnOO(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    va_list list;
    va_start(list, text);
    consoleImpl->OLnOO(text, list);
    va_end(list);
#endif
} // OLnOI()


/**
    OutdentBy().
*/
void CConsole::OOb(int value)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    consoleImpl->OOb(value);
#endif
} // OOb()


/**
    OI() + OLn(text) + OO().
*/
void CConsole::OIOLnOO(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    va_list list;
    va_start(list, text);
    consoleImpl->OIOLnOO(text, list);
    va_end(list);
#endif
}


/**
    Prints line to console and adds a new line.
    @param n How many times the "-=" chars should be repeated.
*/
void CConsole::L(int n)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    consoleImpl->L(n);
#endif
} // L()


/**
    Normal-mode on.
*/
void CConsole::NOn()
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    consoleImpl->NOn();
#endif
} // NOn()


/**
    Error-mode on.
*/
void CConsole::EOn()
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    consoleImpl->EOn();
#endif
} // EOn()


/**
    Error-mode  off.
*/
void CConsole::EOff()
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    consoleImpl->EOff();
#endif
} // EOff()


/**
    Success-mode on.
*/
void CConsole::SOn()
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    consoleImpl->SOn();
#endif
} // SOn()


/**
    Success-mode off. 
*/
void CConsole::SOff()
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    consoleImpl->SOff();
#endif
} // SOff()


/**
    SOn() + O(text) + SOff().
*/
void CConsole::SO(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    va_list list;
    va_start(list, text);
    consoleImpl->SO(text, list);
    va_end(list);
#endif
} // SO()


/**
    SOn() + OLn(text) + SOff().
*/
void CConsole::SOLn(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    va_list list;
    va_start(list, text);
    consoleImpl->SOLn(text, list);
    va_end(list);
#endif
} // SOln()


/**
    EOn() + O(text) + EOff().
*/
void CConsole::EO(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    va_list list;
    va_start(list, text);
    consoleImpl->EO(text, list);
    va_end(list);
#endif
} // EO()


/**
    EOn() + OLn(text) + EOff().
*/
void CConsole::EOLn(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    va_list list;
    va_start(list, text);
    consoleImpl->EOLn(text, list);
    va_end(list);
#endif
} // EOln()


/**
    OI() + SO(text).
*/
void CConsole::OISO(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    va_list list;
    va_start(list, text);
    consoleImpl->OISO(text, list);
    va_end(list);
#endif
} // OISO()


/**
    OI() + SOLn(text).
*/
void CConsole::OISOLn(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    va_list list;
    va_start(list, text);
    consoleImpl->OISOLn(text, list);
    va_end(list);
#endif
} // OISOLn()


/**
    OO() + SO(text).
*/
void CConsole::OOSO(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    va_list list;
    va_start(list, text);
    consoleImpl->OOSO(text, list);
    va_end(list);
#endif
} // OOSO()


/**
    OO() + SOLn(text).
*/
void CConsole::OOSOLn(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    va_list list;
    va_start(list, text);
    consoleImpl->OOSOLn(text, list);
    va_end(list);
#endif
} // OOSOLn()


/**
    OI() + EO(text).
*/
void CConsole::OIEO(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    va_list list;
    va_start(list, text);
    consoleImpl->OIEO(text, list);
    va_end(list);
#endif
} // OIEO()


/**
    OI() + EOLn(text).
*/
void CConsole::OIEOLn(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    va_list list;
    va_start(list, text);
    consoleImpl->OIEOLn(text, list);
    va_end(list);
#endif
} // OIEOLn()


/**
    OO() + EO(text).
*/
void CConsole::OOEO(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    va_list list;
    va_start(list, text);
    consoleImpl->OOEO(text, list);
    va_end(list);
#endif
} // OOEO()


/**
    OO() + EOLn(text).
*/
void CConsole::OOEOLn(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    va_list list;
    va_start(list, text);
    consoleImpl->OOEOLn(text, list);
    va_end(list);
#endif
} // OOEOLn()


/**
    SO(text) + OI().
*/
void CConsole::SOOI(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    va_list list;
    va_start(list, text);
    consoleImpl->SOOI(text, list);
    va_end(list);
#endif
} // SOOI()


/**
    SOLn(text) + OI().
*/
void CConsole::SOLnOI(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    va_list list;
    va_start(list, text);
    consoleImpl->SOLnOI(text, list);
    va_end(list);
#endif
} // SOLnOI()


/**
    SO(text) + OO().
*/
void CConsole::SOOO(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    va_list list;
    va_start(list, text);
    consoleImpl->SOOO(text, list);
    va_end(list);
#endif
} // SOOO()


/**
    SOLn(text) + OO().
*/
void CConsole::SOLnOO(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    va_list list;
    va_start(list, text);
    consoleImpl->SOLnOO(text, list);
    va_end(list);
#endif
} // SOLnOO()


/**
    EO(text) + OI().
*/
void CConsole::EOOI(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    va_list list;
    va_start(list, text);
    consoleImpl->EOOI(text, list);
    va_end(list);
#endif
} // EOOI()


/**
    EOLn(text) + OI().
*/
void CConsole::EOLnOI(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    va_list list;
    va_start(list, text);
    consoleImpl->EOLnOI(text, list);
    va_end(list);
#endif
} // EOLnOI()


/**
    EO(text) + OO().
*/
void CConsole::EOOO(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    va_list list;
    va_start(list, text);
    consoleImpl->EOOO(text, list);
    va_end(list);
#endif
} // EOOO()


/**
    EOLn(text) + OO().
*/
void CConsole::EOLnOO(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    va_list list;
    va_start(list, text);
    consoleImpl->EOLnOO(text, list);
    va_end(list);
#endif
} // EOLnOO()


/**
    OI() + SO(text) + OO().
*/
void CConsole::OISOOO(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    va_list list;
    va_start(list, text);
    consoleImpl->OISOOO(text, list);
    va_end(list);
#endif
} // OISOOO


/**
    OI() + SOLn(text) + OO().
*/
void CConsole::OISOLnOO(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    va_list list;
    va_start(list, text);
    consoleImpl->OISOLnOO(text, list);
    va_end(list);
#endif
} // OISOLnOO


/**
    OI() + EO(text) + OO().
*/
void CConsole::OIEOOO(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    va_list list;
    va_start(list, text);
    consoleImpl->OIEOOO(text, list);
    va_end(list);
#endif
} // OIEOOO


/**
    OI() + EOLn(text) + OO().
*/
void CConsole::OIEOLnOO(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    va_list list;
    va_start(list, text);
    consoleImpl->OIEOLnOO(text, list);
    va_end(list);
#endif
} // OIEOLnOO


/**
    Gets total count of printouts-with-newline during error-mode.
*/
int CConsole::getErrorOutsCount() const
{
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return 0;

    return consoleImpl->getErrorOutsCount();
} // getErrorOutsCount()


/**
    Gets total count of printouts-with-newline during success-mode.
*/
int CConsole::getSuccessOutsCount() const    
{
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return 0;

    return consoleImpl->getSuccessOutsCount();
} // getSuccessOutsCount()


CConsole& CConsole::operator<<(const char* text)
{
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return *this;

    *consoleImpl << text;
    return *this;
} // operator<<()

CConsole& CConsole::operator<<(const bool& b)
{
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return *this;

    *consoleImpl << b;
    return *this;
} // operator<<()

CConsole& CConsole::operator<<(const int& n)
{
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return *this;

    *consoleImpl << n;
    return *this;
} // operator<<()

CConsole& CConsole::operator<<(const float& f)
{
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return *this;

    *consoleImpl << f;
    return *this;
} // operator<<()

CConsole& CConsole::operator<<(const CConsole::FormatSignal& fs)
{
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return *this;

    *consoleImpl << fs;
    return *this;
} // operator<<()


// ############################## PROTECTED ##############################


// ############################### PRIVATE ###############################


CConsole CConsole::consoleInstance;



CConsole::CConsole()                                   
{
    consoleImpl = new CConsoleImpl();
}


CConsole::CConsole(const CConsole&)
{

}


CConsole& CConsole::operator= (const CConsole&)
{
    return *this;
}


CConsole::~CConsole()
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    consoleImpl->OLn("CConsole::~CConsole() LAST MESSAGE, BYE!");
#endif

    delete consoleImpl;
    consoleImpl = NULL;
}