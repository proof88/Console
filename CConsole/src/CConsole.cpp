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
#include <filesystem>  // requires cpp17
#include <iostream>
#include <map>
#include <mutex>       // requires cpp11
#include <set>
#include <string>
#include <thread>      // requires cpp11

#include <winsock.h>   // for gethostname()

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

// TODO: how to generate stacktrace:
// - https://github.com/JochenKalmbach/StackWalker
// - https://github.com/GPMueller/mwe-cpp-exception
// - https://stackoverflow.com/questions/77005/how-to-automatically-generate-a-stacktrace-when-my-program-crashes
// - https://stackoverflow.com/questions/11665829/how-can-i-print-stack-trace-for-caught-exceptions-in-c-code-injection-in-c/11674810#11674810

using namespace std;

static constexpr auto CCONSOLE_VERSION = "v1.2 built on " __DATE__" @ " __TIME__;

static std::mutex mainMutex;  // did not want to put this into CConsoleImpl because then CConsole::IsInitialized() could not be protected by this mutex when impl is not yet existing

/*
   CConsole::CConsoleImpl
   ###########################################################################
   Not thread-safe, but not public anyway.
   Only CConsole uses it, and CConsole is thread-safe.
*/

class CConsole::CConsoleImpl
{
public:
    void DeleteOldLogFiles(size_t nKeep);                        /**< Deletes the old log files. */
    void SetLoggerModuleName(const char* loggerModuleName);      /**< Sets the current logger module name. */
    bool getLoggingState(const char* loggerModuleName);          /**< Gets logging state for the given logger module. */
    void SetLoggingState(const char* loggerModule, bool state);  /**< Sets logging on or off for the given logger module. */
    void SetErrorsAlwaysOn(bool state);                          /**< Sets errors always appear irrespective of logging state of current logger module. */

    int  getIndent();             /**< Gets the current indentation. */
    void SetIndent(int value);    /**< Sets the current indentation. */
    void Indent();                /**< Increases indentation. */
    void IndentBy(int value);     /**< Increases indentation by the given value. */
    void Outdent();               /**< Decreases indentation. */
    void OutdentBy(int value);    /**< Decreases indentation by the given value. */

    void LoadColors();            /**< Loads previously saved colors. */
    void SaveColors();            /**< Saves current colors. */
    void RestoreDefaultColors();  /**< Restores default colors. */

    WORD        getFGColor();                  /**< Gets foreground color. */
    const char* getFGColorHtml();              /**< Gets html foreground color. */
    void        SetFGColor(
        WORD clr, const char* html = NULL);    /**< Sets foreground color. */
    WORD        getBGColor();                  /**< Gets background color. */
    void        SetBGColor(WORD clr);          /**< Sets background color. */

    WORD        getIntsColor();                /**< Gets ints color. */
    const char* getIntsColorHtml();            /**< Gets ints html color. */
    void        SetIntsColor(
        WORD clr, const char* html = NULL);    /**< Sets ints color. */

    WORD        getStringsColor();             /**< Gets strings color. */
    const char* getStringsColorHtml();         /**< Gets strings html color. */
    void        SetStringsColor(
        WORD clr, const char* html = NULL);    /**< Sets strings color. */

    WORD        getFloatsColor();              /**< Gets floats color. */
    const char* getFloatsColorHtml();          /**< Gets floats html color. */
    void        SetFloatsColor(
        WORD clr, const char* html = NULL);    /**< Sets floats color. */

    WORD        getBoolsColor();               /**< Gets bools color. */
    const char* getBoolsColorHtml();           /**< Gets bools html color. */
    void        SetBoolsColor(
        WORD clr, const char* html = NULL);    /**< Sets bools color. */

    void O(const char* text, va_list list);       /**< Prints text to console. */
    void O(const char* text, ...);                /**< Prints text to console. */
    void OLn(const char* text, va_list list);     /**< Prints text to console and adds a new line. */
    void OLn(const char* text, ...);              /**< Prints text to console and adds a new line. */
    void OI();                                    /**< Indent(). */
    void OIO(const char* text, va_list list);     /**< OI() + O(text). */
    void OIOLn(const char* text, va_list list);   /**< OI() + OLn(text). */
    void OLnOI(const char* text, va_list list);   /**< OLn(text) + OI(). */
    void OIb(int value);                          /**< IndentBy(). */
    void OO();                                    /**< Outdent(). */
    void OOO(const char* text, va_list list);     /**< OO() + O(text). */
    void OOOLn(const char* text, va_list list);   /**< OO() + OLn(text). */
    void OLnOO(const char* text, va_list list);   /**< OLn(text) + OO(). */
    void OOb(int value);                          /**< OutdentBy(). */
    void OIOLnOO(const char* text, va_list list); /**< OI() + OLn(text) + OO(). */
    void L(int n = 20);                           /**< Prints line to console and adds a new line. */

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
    void ResetErrorOutsCount();         /**< Resets total count of printouts-with-newline during error-mode. */
    void ResetSuccessOutsCount();       /**< Resets total count of printouts-with-newline during success-mode. */

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

    static const int HTML_CLR_S = 7 * sizeof(char);   /**< Size to store 1 HTML-color. */

    static CConsoleImpl consoleImplInstance;

    static int      nErrorOutCount;         /**< Total OLn() during error mode. */
    static int      nSuccessOutCount;       /**< Total OLn() during success mode. */

    struct LogState
    {
        int  nIndentValue{0};                     /**< Current indentation. */
        std::string sLoggerName;                  /**< Name of the current logger module that last invoked getConsoleInstance(). */
        int  nMode{0};                            /**< Current mode: 0 if normal, 1 is error, 2 is success (EOn()/EOff()/SOn()/SOff()/NOn()/RestoreDefaultColors() set this). */
        WORD clrFG{CCONSOLE_DEF_CLR_FG},
             clrBG{0};                            /**< Current foreground and background colors. */
        char clrFGhtml[HTML_CLR_S]{0};            /**< Current foreground html color. */
        WORD clrInts{CCONSOLE_DEF_CLR_FG},
            clrFloats{CCONSOLE_DEF_CLR_FG},
            clrStrings{CCONSOLE_DEF_CLR_FG},
            clrBools{CCONSOLE_DEF_CLR_FG};        /**< Current colors for ints, strings, floats and bools. */
        char clrIntsHtml[HTML_CLR_S]{0},
            clrFloatsHtml[HTML_CLR_S]{0},
            clrStringsHtml[HTML_CLR_S]{0},
            clrBoolsHtml[HTML_CLR_S]{0};          /**< Current html colors for ints, strings, floats and bools. */
        WORD dLastFGColor{0};                     /**< Saved foreground color. */
        char dLastFGColorHtml[HTML_CLR_S]{0};     /**< Saved html foreground color. */
        WORD dLastIntsColor{0},
            dLastFloatsColor{0},
            dLastStringsColor{0},
            dLastBoolsColor{0};                   /**< Saved colors for ints, strings, floats and bools. */
        char dLastIntsColorHtml[HTML_CLR_S]{0},
            dLastFloatsColorHtml[HTML_CLR_S]{0},
            dLastStringsColorHtml[HTML_CLR_S]{0},
            dLastBoolsColorHtml[HTML_CLR_S]{0};   /**< Saved html colors for ints, strings, floats and bools. */
    };

    // ---------------------------------------------------------------------------

    bool bInited;                              /**< False by default, Initialize() sets it to true, Deinitialize() sets it to false. */
    int  nRefCount;                            /**< 0 by default, Initialize() increases it by 1, Deinitialize() decreases it by 1. */ 
    bool bFirstWriteTextCallAfterWriteTextLn;  /**< True if we are at the 1st no-new-line-print after a new-line-print. */

    std::map<std::thread::id, LogState> logState;  /**< Per-thread log state. */
    
    HANDLE hConsole;                    /**< Console output handle. */

    char  vmi[80];                   /**< Temp, always used for the current printout. */
    DWORD wrt;                       /**< Temp, always used for the current printout. */
    WORD  oldClrFG;                  /**< Temp, always used for the current printout. */
    char  oldClrFGhtml[HTML_CLR_S];  /**< Temp, always used for the current printout. */

    std::ofstream fLog;
    bool bAllowLogFile;

    std::set<std::string> enabledModules;  /**< Contains logger module names for which logging is enabled. */
    bool        bErrorsAlwaysOn;           /**< Should module error logs always appear or not. */

    // ---------------------------------------------------------------------------

    CConsoleImpl();

    CConsoleImpl(const CConsoleImpl&);
    CConsoleImpl& operator=(const CConsoleImpl&);

    virtual ~CConsoleImpl();

    bool canWeWriteBasedOnFilterSettings();

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
    Deletes the old log files.
    @param nKeep How many log files are allowed to be kept in their directory with the new log file being created.
*/
void CConsole::CConsoleImpl::DeleteOldLogFiles(size_t nKeep)
{
    std::set<std::filesystem::path> logFiles;
    for (const auto& entry : std::filesystem::directory_iterator("."))
    {
        // Log file name looks like this when iterated here: ".\log_%hostname_%Y-%m-%d_%H-%M-%S.html"
        if ((entry.path().extension().string() == ".html") && (entry.path().string().find("log_") == 2))
        {
            logFiles.insert(entry.path());  // inserting to set automatically makes them sorted from oldest to newest
        }
    }
    if (logFiles.size() > (nKeep-1))
    {
        const size_t nLogFilesToDelete = logFiles.size() - (nKeep - 1);
        OLn("Deleting the following %d oldest log file(s):", nLogFilesToDelete);
        size_t iLogFileToDelete = 0;
        for (const auto& logFile : logFiles)
        {
            OLn("  %s", logFile.string().c_str());
            std::error_code errCode;
            if (!std::filesystem::remove(logFile, errCode))
            {
                EOLn("  ERROR: Could not remove above file, error code: %d, message: %s", errCode.value(), errCode.message().c_str());
            }
            if (++iLogFileToDelete == nLogFilesToDelete)
            {
                break;
            }
        }
    }
}


/**
    Sets the current logger module name.
    Per-thread property.
*/
void CConsole::CConsoleImpl::SetLoggerModuleName(const char* loggerModuleName)
{
    logState[std::this_thread::get_id()].sLoggerName = loggerModuleName;
}


/**
    Gets logging state for the given logger module.
    See more explanation about logger module state at SetLoggingState().
    Per-process property.

    @param loggerModuleName Name of the logger whose logging state we are interested in.
    @return Logging state of the given logger module. Always true for empty string.
*/
bool CConsole::CConsoleImpl::getLoggingState(const char* loggerModuleName)
{
    if (!bInited)
    {
        return false;
    }

    std::string sLoggerModuleName = loggerModuleName;
    if (sLoggerModuleName.empty())
    {
        return true;
    }

    return enabledModules.end() != enabledModules.find(sLoggerModuleName);
}


/**
    Sets logging on or off for the given logger module.
    By default logging is NOT enabled for any logger modules.
    Initially logging can be done only with empty loggerModule name.
    For specific modules that invoke getConsoleInstance() with their module name, logging
    state must be enabled in order to make their logs actually appear.
    Per-process property: changing logging state of a logger module will have the same effect on
    all threads using the same logger module name.

    @param loggerModuleName Name of the logger who wants to change its logging state.
                            If this is "4LLM0DUL3S", the given state turns full verbose logging on or off, regardless of any other logging state.
    @param state True to enable logging of the loggerModule, false to disable.
*/
void CConsole::CConsoleImpl::SetLoggingState(const char* loggerModuleName, bool state)
{
    if ( !bInited )
        return;

    const size_t sizeOfLoggerModuleNameBuffer = sizeof(char) * (strlen(loggerModuleName) + 1);
    char* const newNameLoggerModule = (char* const)malloc(sizeOfLoggerModuleNameBuffer);
    if (newNameLoggerModule == nullptr)
        return;

    strncpy_s(newNameLoggerModule, sizeOfLoggerModuleNameBuffer, loggerModuleName, sizeOfLoggerModuleNameBuffer);
    PFL::strClr(newNameLoggerModule);
    if (strlen(newNameLoggerModule) == 0)
    {
        free(newNameLoggerModule);
        return;
    }
    free(newNameLoggerModule);

    if (state)
    {
        enabledModules.insert(loggerModuleName);
    }
    else
    {
        enabledModules.erase(loggerModuleName);
    }
} // SetLoggingState 


/**
    Sets errors always appear irrespective of logging state of current logger module.
    Default value is true.
    Per-process property.

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
    Per-thread property.
*/
int CConsole::CConsoleImpl::getIndent()
{
    if ( !bInited )
        return 0;

    return logState[std::this_thread::get_id()].nIndentValue;
} // getIndent()


/**
    Sets the current indentation.
    Per-thread property.
*/
void CConsole::CConsoleImpl::SetIndent(int value)
{
    if ( !bInited )
        return;

    logState[std::this_thread::get_id()].nIndentValue = value;
    if (logState[std::this_thread::get_id()].nIndentValue < 0)
        logState[std::this_thread::get_id()].nIndentValue = 0;
} // SetIndent()


/**
    Increases indentation.
    Per-thread property.
*/
void CConsole::CConsoleImpl::Indent()
{
    if ( !bInited )
        return;

    logState[std::this_thread::get_id()].nIndentValue += CConsoleImpl::CCONSOLE_INDENTATION_CHANGE;
} // Indent()


/**
    Increases indentation by the given value.
    Per-thread property.
*/
void CConsole::CConsoleImpl::IndentBy(int value)
{
    if ( !bInited )
        return;

    logState[std::this_thread::get_id()].nIndentValue += value;
    if (logState[std::this_thread::get_id()].nIndentValue < 0)
        logState[std::this_thread::get_id()].nIndentValue = 0;
} // IndentBy()


/**
    Decreases indentation.
    Per-thread property.
*/
void CConsole::CConsoleImpl::Outdent()
{
    if ( !bInited )
        return;

    logState[std::this_thread::get_id()].nIndentValue -= CConsoleImpl::CCONSOLE_INDENTATION_CHANGE;
    if (logState[std::this_thread::get_id()].nIndentValue < 0)
        logState[std::this_thread::get_id()].nIndentValue = 0;
} // Outdent()


/**
    Decreases indentation by the given value.
    Per-thread property.
*/
void CConsole::CConsoleImpl::OutdentBy(int value)
{
    if ( !bInited )
        return;

    logState[std::this_thread::get_id()].nIndentValue -= value;
    if (logState[std::this_thread::get_id()].nIndentValue < 0)
        logState[std::this_thread::get_id()].nIndentValue = 0;
} // OutdentBy()


/**
    Loads previously saved colors.
    Per-thread property.
*/
void CConsole::CConsoleImpl::LoadColors()
{
    if ( !bInited )
        return;

    SetFGColor(logState[std::this_thread::get_id()].dLastFGColor, logState[std::this_thread::get_id()].dLastBoolsColorHtml);
    SetStringsColor(logState[std::this_thread::get_id()].dLastStringsColor, logState[std::this_thread::get_id()].dLastStringsColorHtml);
    SetFloatsColor(logState[std::this_thread::get_id()].dLastFloatsColor, logState[std::this_thread::get_id()].dLastFloatsColorHtml);
    SetIntsColor(logState[std::this_thread::get_id()].dLastIntsColor, logState[std::this_thread::get_id()].dLastIntsColorHtml);
    SetBoolsColor(logState[std::this_thread::get_id()].dLastBoolsColor, logState[std::this_thread::get_id()].dLastBoolsColorHtml);
} // LoadColors()


/**
    Saves current colors.
    Per-thread property.
*/
void CConsole::CConsoleImpl::SaveColors()
{
#ifdef CCONSOLE_IS_ENABLED
    // This should also work if we are not yet initialized, since Initialize() calls this
    //if (!bInited)
    //    return;

    logState[std::this_thread::get_id()].dLastFGColor = logState[std::this_thread::get_id()].clrFG;
    logState[std::this_thread::get_id()].dLastStringsColor = logState[std::this_thread::get_id()].clrStrings;
    logState[std::this_thread::get_id()].dLastFloatsColor = logState[std::this_thread::get_id()].clrFloats;
    logState[std::this_thread::get_id()].dLastIntsColor = logState[std::this_thread::get_id()].clrInts;
    logState[std::this_thread::get_id()].dLastBoolsColor = logState[std::this_thread::get_id()].clrBools;
    strcpy(logState[std::this_thread::get_id()].dLastFGColorHtml, logState[std::this_thread::get_id()].clrFGhtml);
    strcpy(logState[std::this_thread::get_id()].dLastStringsColorHtml, logState[std::this_thread::get_id()].clrStringsHtml);
    strcpy(logState[std::this_thread::get_id()].dLastFloatsColorHtml, logState[std::this_thread::get_id()].clrFloatsHtml);
    strcpy(logState[std::this_thread::get_id()].dLastIntsColorHtml, logState[std::this_thread::get_id()].clrIntsHtml);
    strcpy(logState[std::this_thread::get_id()].dLastBoolsColorHtml, logState[std::this_thread::get_id()].clrBoolsHtml);
#endif
}


/**
    Restores default colors.
    Per-thread property.
*/
void CConsole::CConsoleImpl::RestoreDefaultColors()
{
#ifdef CCONSOLE_IS_ENABLED
    // This should also work if we are not yet initialized, since Initialize() calls this
    //if (!bInited)
    //    return;

    logState[std::this_thread::get_id()].clrFG = CCONSOLE_DEF_CLR_FG;
    logState[std::this_thread::get_id()].clrBG = 0;
    logState[std::this_thread::get_id()].clrInts = logState[std::this_thread::get_id()].clrFG;
    logState[std::this_thread::get_id()].clrFloats = logState[std::this_thread::get_id()].clrFG;
    logState[std::this_thread::get_id()].clrStrings = logState[std::this_thread::get_id()].clrFG;
    logState[std::this_thread::get_id()].clrBools = logState[std::this_thread::get_id()].clrFG;

    memset(logState[std::this_thread::get_id()].clrFGhtml, 0, HTML_CLR_S);
    memset(logState[std::this_thread::get_id()].clrStringsHtml, 0, HTML_CLR_S);
    memset(logState[std::this_thread::get_id()].clrIntsHtml, 0, HTML_CLR_S);
    memset(logState[std::this_thread::get_id()].clrFloatsHtml, 0, HTML_CLR_S);
    memset(logState[std::this_thread::get_id()].clrBoolsHtml, 0, HTML_CLR_S);
    memset(logState[std::this_thread::get_id()].dLastFGColorHtml, 0, HTML_CLR_S);
    memset(logState[std::this_thread::get_id()].dLastStringsColorHtml, 0, HTML_CLR_S);
    memset(logState[std::this_thread::get_id()].dLastIntsColorHtml, 0, HTML_CLR_S);
    memset(logState[std::this_thread::get_id()].dLastFloatsColorHtml, 0, HTML_CLR_S);
    memset(logState[std::this_thread::get_id()].dLastBoolsColorHtml, 0, HTML_CLR_S);

    strcpy(logState[std::this_thread::get_id()].clrFGhtml, "999999");
    strcpy(logState[std::this_thread::get_id()].clrIntsHtml, logState[std::this_thread::get_id()].clrFGhtml);
    strcpy(logState[std::this_thread::get_id()].clrFloatsHtml, logState[std::this_thread::get_id()].clrFGhtml);
    strcpy(logState[std::this_thread::get_id()].clrStringsHtml, logState[std::this_thread::get_id()].clrFGhtml);
    strcpy(logState[std::this_thread::get_id()].clrBoolsHtml, logState[std::this_thread::get_id()].clrFGhtml);
#endif
}


/**
    Gets foreground color.
    Per-thread property.
*/
WORD CConsole::CConsoleImpl::getFGColor()
{
    if ( !bInited )
        return 0;

    return logState[std::this_thread::get_id()].clrFG;
} // getFGColor()


/**
    Gets html foreground color.
    Per-thread property.
*/
const char* CConsole::CConsoleImpl::getFGColorHtml()
{
    if ( !bInited )
        return "#DDBEEF";

    return logState[std::this_thread::get_id()].clrFGhtml;
} // getFGColorHtml()


/**
    Sets foreground color.
    Per-thread property.
*/
void CConsole::CConsoleImpl::SetFGColor(WORD clr, const char* html)
{
    if ( !bInited )
        return;

    logState[std::this_thread::get_id()].clrFG = clr;
    SetConsoleTextAttribute(hConsole, logState[std::this_thread::get_id()].clrFG | logState[std::this_thread::get_id()].clrBG);
    if (html)
        strcpy_s(logState[std::this_thread::get_id()].clrFGhtml, CConsoleImpl::HTML_CLR_S, html);
} // SetFGColor()


/**
    Gets background color.
    Per-thread property.
*/
WORD CConsole::CConsoleImpl::getBGColor()
{
    if ( !bInited )
        return 0;

    return logState[std::this_thread::get_id()].clrBG;
} // getBGColor()


/**
    Sets background color.
    Per-thread property.
*/
void CConsole::CConsoleImpl::SetBGColor(WORD clr)
{
    if ( !bInited )
        return;

    logState[std::this_thread::get_id()].clrBG = clr;
    SetConsoleTextAttribute(hConsole, logState[std::this_thread::get_id()].clrFG | logState[std::this_thread::get_id()].clrBG);
} // SetBGColor()


/**
    Gets ints color.
    Per-thread property.
*/
WORD CConsole::CConsoleImpl::getIntsColor()
{
    if ( !bInited )
        return 0;

    return logState[std::this_thread::get_id()].clrInts;
} // getIntsColor()


/**
    Gets ints html color.
    Per-thread property.
*/
const char* CConsole::CConsoleImpl::getIntsColorHtml()
{
    if ( !bInited )
        return "#DDBEEF";

    return logState[std::this_thread::get_id()].clrIntsHtml;
} // getIntsColorHtml()


/**
    Sets ints color.
    Per-thread property.
*/
void CConsole::CConsoleImpl::SetIntsColor(WORD clr, const char* html)
{
    if ( !bInited )
        return;

    logState[std::this_thread::get_id()].clrInts = clr;
    if (html)
        strcpy_s(logState[std::this_thread::get_id()].clrIntsHtml, CConsoleImpl::HTML_CLR_S, html);
} // SetIntsColor()


/**
    Gets strings color.
    Per-thread property.
*/
WORD CConsole::CConsoleImpl::getStringsColor()
{
    if ( !bInited )
        return 0;

    return logState[std::this_thread::get_id()].clrStrings;
} // getStringsColor()


/**
    Gets strings html color.
    Per-thread property.
*/
const char* CConsole::CConsoleImpl::getStringsColorHtml()
{
    if ( !bInited )
        return "#DDBEEF";

    return logState[std::this_thread::get_id()].clrStringsHtml;
} // getStringsColorHtml()


/**
    Sets strings color.
    Per-thread property.
*/
void CConsole::CConsoleImpl::SetStringsColor(WORD clr, const char* html)
{
    if ( !bInited )
        return;

    logState[std::this_thread::get_id()].clrStrings = clr;
    if (html)
        strcpy_s(logState[std::this_thread::get_id()].clrStringsHtml, CConsoleImpl::HTML_CLR_S, html);
} // SetStringsColor()


/**
    Gets floats color.
    Per-thread property.
*/
WORD CConsole::CConsoleImpl::getFloatsColor()
{
    if ( !bInited )
        return 0;

    return logState[std::this_thread::get_id()].clrFloats;
} // getFloatsColor()


/**
    Gets floats html color.
    Per-thread property.
*/
const char* CConsole::CConsoleImpl::getFloatsColorHtml()
{
    if ( !bInited )
        return "#DDBEEF";

    return logState[std::this_thread::get_id()].clrFloatsHtml;
} // getFloatsColorHtml()


/**
    Sets floats color.
    Per-thread property.
*/
void CConsole::CConsoleImpl::SetFloatsColor(WORD clr, const char* html)
{
    if ( !bInited )
        return;

    logState[std::this_thread::get_id()].clrFloats = clr;
    if (html)
        strcpy_s(logState[std::this_thread::get_id()].clrFloatsHtml, CConsoleImpl::HTML_CLR_S, html);
} // SetFloatsColor()


/**
    Gets bools color.
    Per-thread property.
*/
WORD CConsole::CConsoleImpl::getBoolsColor()
{
    if ( !bInited )
        return 0;

    return logState[std::this_thread::get_id()].clrBools;
} // getBoolsColor()


/**
    Gets bools html color.
    Per-thread property.
*/
const char* CConsole::CConsoleImpl::getBoolsColorHtml()
{
    if ( !bInited )
        return "#DDBEEF";

    return logState[std::this_thread::get_id()].clrBoolsHtml;
} // getBoolsColorHtml()


/**
    Sets bools color.
    Per-thread property.
*/
void CConsole::CConsoleImpl::SetBoolsColor(WORD clr, const char* html)
{
    if ( !bInited )
        return;

    logState[std::this_thread::get_id()].clrBools = clr;
    if (html)
        strcpy_s(logState[std::this_thread::get_id()].clrBoolsHtml, CConsoleImpl::HTML_CLR_S, html);
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
    Prints text to console.
*/
void CConsole::CConsoleImpl::O(const char* text, ...)
{
    if (!bInited)
        return;

    va_list list;
    va_start(list, text);
    O(text, list);
    va_end(list);
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

    // we need to invoke O() instead of WriteText(), because latter won't take care of the indentation properly for html
    for (int i = 0; i < n; i++)
    {
        O("-=");
    }
    OLn("\n\r");
} // L()


/**
    Normal-mode on.
    Per-thread property.
*/
void CConsole::CConsoleImpl::NOn()
{
    if ( !bInited )
        return;

    logState[std::this_thread::get_id()].nMode = 0;
    LoadColors();
} // NOn()


/**
    Error-mode on.
    Per-thread property.
*/
void CConsole::CConsoleImpl::EOn()
{
    if ( !bInited )
        return;

    if (logState[std::this_thread::get_id()].nMode == 1)
    {
        return;
    }
    else if (logState[std::this_thread::get_id()].nMode == 2)
    {
        SOff();
    }

    logState[std::this_thread::get_id()].nMode = 1;
    SaveColors();
    SetFGColor(FOREGROUND_RED | FOREGROUND_INTENSITY, "FF0000");
    SetStringsColor(FOREGROUND_RED | FOREGROUND_GREEN, "DDDD00");
    SetFloatsColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "FFFF00");
    SetIntsColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "FFFF00");
    SetBoolsColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "FFFF00");
} // EOn()


/**
    Error-mode off.
    Per-thread property.
*/
void CConsole::CConsoleImpl::EOff()
{
    if ( !bInited )
        return;

    NOn();
} // EOff()


/**
    Success-mode on.
    Per-thread property.
*/
void CConsole::CConsoleImpl::SOn()
{
    if ( !bInited )
        return;

    if (logState[std::this_thread::get_id()].nMode == 2)
    {
        return;
    }
    else if (logState[std::this_thread::get_id()].nMode == 1)
    {
        EOff();
    }

    logState[std::this_thread::get_id()].nMode = 2;
    SaveColors();
    SetFGColor(FOREGROUND_GREEN, "00DD00");
    SetStringsColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY, "00FF00");
    SetFloatsColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "FFFF00");
    SetIntsColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "FFFF00");
    SetBoolsColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "FFFF00");
} // SOn()


/**
    Success-mode off.
    Per-thread property.
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


/**
    Resets total count of printouts-with-newline during error-mode.
    Per-process property.
*/
void CConsole::CConsoleImpl::ResetErrorOutsCount()
{
    if (!bInited)
        return;

    nErrorOutCount = 0;
}


/**
    Resets total count of printouts-with-newline during success-mode.
    Per-process property.
*/
void CConsole::CConsoleImpl::ResetSuccessOutsCount()
{
    if (!bInited)
        return;

    nSuccessOutCount = 0;
}


/**
    O("%s", text).
*/
CConsole::CConsoleImpl& CConsole::CConsoleImpl::operator<<(const char* text)
{
    if ( !bInited )
        return *this;

    if (bFirstWriteTextCallAfterWriteTextLn)
    {
        for (int i = 0; i < logState[std::this_thread::get_id()].nIndentValue; i++)
        {
            WriteText(" ");
        }
    }
    ImmediateWriteString(text);
    return *this;
} // operator<<()


/**
    O("%b", b).
*/
CConsole::CConsoleImpl& CConsole::CConsoleImpl::operator<<(const bool& b)
{
    if ( !bInited )
        return *this;

    if (bFirstWriteTextCallAfterWriteTextLn)
    {
        for (int i = 0; i < logState[std::this_thread::get_id()].nIndentValue; i++)
        {
            WriteText(" ");
        }
    }
    ImmediateWriteBool(b);
    return *this;
} // operator<<()


/**
    O("%d", n).
*/
CConsole::CConsoleImpl& CConsole::CConsoleImpl::operator<<(const int& n)
{
    if ( !bInited )
        return *this;

    if (bFirstWriteTextCallAfterWriteTextLn)
    {
        for (int i = 0; i < logState[std::this_thread::get_id()].nIndentValue; i++)
        {
            WriteText(" ");
        }
    }
    ImmediateWriteInt(n);
    return *this;
} // operator<<()


/**
    O("%f", f).
*/
CConsole::CConsoleImpl& CConsole::CConsoleImpl::operator<<(const float& f)
{
    if ( !bInited )
        return *this;

    if (bFirstWriteTextCallAfterWriteTextLn)
    {
        for (int i = 0; i < logState[std::this_thread::get_id()].nIndentValue; i++)
        {
            WriteText(" ");
        }
    }
    ImmediateWriteFloat(f);
    return *this;
} // operator<<()


/**
    Changes current mode or adds a new line.
    Based on value of fs, equals to calling EOn()/EOff()/SOn()/SOff()/NOn() accordingly.
    If fs is FormatSignal::NL, the behavior is same as OLn("").
*/
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
    bFirstWriteTextCallAfterWriteTextLn = true;
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


bool CConsole::CConsoleImpl::canWeWriteBasedOnFilterSettings()
{
    if ( logState[std::this_thread::get_id()].sLoggerName.empty() )
    {
        return true;
    }

    // magic module name for turning on all logging
    auto it = enabledModules.find("4LLM0DUL3S");
    if ( it != enabledModules.end() )
    {
        return true;
    }

    it = enabledModules.find(logState[std::this_thread::get_id()].sLoggerName);
    if ( it != enabledModules.end() )
    {
        return true;
    }

    if ( bErrorsAlwaysOn && (logState[std::this_thread::get_id()].nMode == 1) )
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

    oldClrFG = logState[std::this_thread::get_id()].clrFG;
    //oldClrFGhtml = clrFGhtml;
    if ( text != NULL )
    {
        SetFGColor(logState[std::this_thread::get_id()].clrStrings);
        WriteConsoleA(hConsole, text, strlen(text), &wrt, 0);
        if ( bAllowLogFile )
            fLog << "<font color=\"#" << logState[std::this_thread::get_id()].clrStringsHtml << "\">" << text << "</font>";
    }
    else
    {
        SetFGColor(oldClrFG);
        WriteConsoleA(hConsole, "NULL", 4, &wrt, 0);
        if ( bAllowLogFile )
            fLog << "<font color=\"#" << logState[std::this_thread::get_id()].clrStringsHtml << "\">NULL</font>";
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

    oldClrFG = logState[std::this_thread::get_id()].clrFG;
    SetFGColor(logState[std::this_thread::get_id()].clrBools);
    WriteConsoleA(hConsole, l ? "true" : "false", l ? 4 : 5, &wrt, 0);
    if ( bAllowLogFile )
        fLog << "<font color=\"#" << logState[std::this_thread::get_id()].clrBoolsHtml << "\">" << (l ? "true" : "false") << "</font>";
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

    oldClrFG = logState[std::this_thread::get_id()].clrFG;
    SetFGColor(logState[std::this_thread::get_id()].clrInts);
    itoa(n,vmi,10);
    WriteConsoleA(hConsole, vmi, strlen(vmi), &wrt, 0);
    if ( bAllowLogFile )
        fLog << "<font color=\"#" << logState[std::this_thread::get_id()].clrIntsHtml << "\">" << vmi << "</font>";
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

    oldClrFG = logState[std::this_thread::get_id()].clrFG;
    SetFGColor(logState[std::this_thread::get_id()].clrInts);
    sprintf(vmi, "%u", n);
    WriteConsoleA(hConsole, vmi, strlen(vmi), &wrt, 0);
    if ( bAllowLogFile )
        fLog << "<font color=\"#" << logState[std::this_thread::get_id()].clrIntsHtml << "\">" << vmi << "</font>";
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

    oldClrFG = logState[std::this_thread::get_id()].clrFG;
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

    SetFGColor(logState[std::this_thread::get_id()].clrFloats);
    WriteConsoleA(hConsole, vmi, newlen, &wrt, 0);
    if (bAllowLogFile)
    {
        fLog << "<font color=\"#" << logState[std::this_thread::get_id()].clrFloatsHtml << "\">" << vmi << "</font>";
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

    oldClrFG = logState[std::this_thread::get_id()].clrFG;
    logState[std::this_thread::get_id()].clrFG = logState[std::this_thread::get_id()].clrStrings;
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
    logState[std::this_thread::get_id()].clrFG = oldClrFG;
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
        for (int i = 0; i < logState[std::this_thread::get_id()].nIndentValue; i++)
            WriteText(" ");
    
    oldClrFG = logState[std::this_thread::get_id()].clrFG;
    if (bAllowLogFile)
    {
        if (logState[std::this_thread::get_id()].nMode != 0)
        {
            fLog << "<font color=\"#" << logState[std::this_thread::get_id()].clrFGhtml << "\">";
        }
    }

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
    if (bAllowLogFile)
    {
        if (logState[std::this_thread::get_id()].nMode != 0)
        {
            fLog << "</font>";
        }
    }
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
        if (logState[std::this_thread::get_id()].nMode == 1)
        {
            nErrorOutCount++;
        }
        else if (logState[std::this_thread::get_id()].nMode == 2)
        {
            nSuccessOutCount++;
        }
    }
} // WriteFormattedTextExCaller()


/*
   CConsole
   ###########################################################################
*/


// ############################### PUBLIC ################################


/**
    Gets the singleton instance.
    All public functions of this singleton instance are thread-safe.

    @param loggerModuleName Name of the logger module who wants to use the singleton instance.
           It is recommended to use the same name for same entity, because this logger name
           is the basis of per-module filtering.
           Even a single thread can use different logger modules.
           The given logger module name is used for the invoking thread, so threads logging
           concurrently with different logger module names are not affecting each other's
           enabled state for the given logger module name.
    @return The singleton instance pre-set for the specified logger module.
*/
CConsole& CConsole::getConsoleInstance(const char* loggerModuleName)
{
    std::lock_guard<std::mutex> lock(mainMutex);

    if (consoleInstance.consoleImpl && loggerModuleName)
    {
        consoleInstance.consoleImpl->SetLoggerModuleName(loggerModuleName);
    }
    return consoleInstance;
} // getConsoleInstance()


/**
    Gets logging state for the given logger module.
    See more explanation about logger module state at SetLoggingState().
    Per-process property.

    @param loggerModuleName Name of the logger whose logging state we are interested in.
    @return Logging state of the given logger module. Always true for empty string.
*/
bool CConsole::getLoggingState(const char* loggerModuleName) const
{
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return false;

    if (!loggerModuleName)
        return false;

    return consoleImpl->getLoggingState(loggerModuleName);
}


/**
    Sets logging on or off for the given logger module.
    By default logging is NOT enabled for any logger modules.
    Initially logging can be done only with empty loggerModule name.
    For specific modules that invoke getConsoleInstance() with their module name, logging
    state must be enabled in order to make their logs actually appear.
    Per-process property: changing logging state of a logger module will have the same effect on
    all threads using the same logger module name.

    @param loggerModuleName Name of the logger who wants to change its logging state.
                            If this is "4LLM0DUL3S", the given state turns full verbose logging on or off, regardless of any other logging state.
    @param state True to enable logging of the loggerModule, false to disable.
*/
void CConsole::SetLoggingState(const char* loggerModuleName, bool state)
{
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    consoleImpl->SetLoggingState(loggerModuleName, state);
} // SetLoggingState 


/**
    Sets errors always appear irrespective of logging state of current logger module.
    Default value is true.
    Per-process property.

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
    In case of multiple threads using CConsole, all threads should invoke Initialize() once at thread startup, and invoke
    Deinitialize() once at thread shutdown.
    If CConsole is already initialized, then the parameters used in subsequent calls to Initialize() are ignored.

    @param title The caption text that will be set in the appearing console window.
                 Ignored in subsequent calls, when CConsole is already initialized.
    @param createLogFile If true, a HTML-based log file will be created and logs will be written into that in parallel with writing to the console window.
                         Ignored in subsequent calls, when CConsole is already initialized.
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

    // refcount always needs to be incremented as soon as we reach this point;
    // even 1 thread can increase it multiple times, that is normal, since multiple independent subsystems/libraries
    // running on same thread invoke Initialize() during their startup
    consoleImpl->nRefCount++;

    const bool bNewThread = consoleImpl->logState.end() == consoleImpl->logState.find(std::this_thread::get_id());
    if (bNewThread)
    {
        // per-thread log state initialization
        // LogState sets some defaults for indentation and colors when created so we just set some necessary stuff here
        consoleImpl->logState.insert({ std::this_thread::get_id(), CConsoleImpl::LogState{} });
        consoleImpl->RestoreDefaultColors();
        consoleImpl->SaveColors();
    }

    if ( !(consoleImpl->bInited) )
    {
        // we come here only once per process, even if Initialize() is invoked multiple consecutive times
        // (of course we might come here later again if sufficient number of calls to Deinitialize() completely shut console down)

        if ( !AllocConsole() )
        {
            return;
        }

        // hack to let logs of this initialize function pass thru 
        const std::string prevLoggerName = consoleImpl->logState[std::this_thread::get_id()].sLoggerName;
        consoleImpl->logState[std::this_thread::get_id()].sLoggerName = "";
        
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
        consoleImpl->OLn("CConsole::%s() %s", __func__, CCONSOLE_VERSION);

        consoleImpl->bAllowLogFile = createLogFile;
        if ( createLogFile )
        {
            const auto time = std::time(nullptr);
            char fLogFilename[300] = "log_";
            size_t nStrLen = strlen(fLogFilename);

            const WORD wWsaVersionRequested = MAKEWORD(2, 2);
            WSADATA wsaData;
            const int nWsaStartupRet = WSAStartup(wWsaVersionRequested, &wsaData);
            if (nWsaStartupRet == 0)
            {
                const int nGetHostNameRet = gethostname(fLogFilename + nStrLen, sizeof(fLogFilename) - nStrLen);
                if (nGetHostNameRet != 0)
                {
                    consoleImpl->EOLn("ERROR: Couldn't get host name, error code: %d", nGetHostNameRet);
                }
                nStrLen = strlen(fLogFilename);
            }
            else
            {
                consoleImpl->EOLn("ERROR: Couldn't initialize WSA, error code: %d", nWsaStartupRet);
            }
            
            if ( 0 == std::strftime(fLogFilename + nStrLen, sizeof(fLogFilename)- nStrLen, "_%Y-%m-%d_%H-%M-%S.html", std::gmtime(&time)) )
            {
                consoleImpl->bAllowLogFile = false;
                consoleImpl->EOLn("ERROR: Couldn't generate file name! Initial name was: \"%s\"", fLogFilename);
            }
            else
            {
                // before opening new file, let's get rid of some older log files
                consoleImpl->DeleteOldLogFiles(3);

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

        consoleImpl->SOLn("CConsole::%s() > CConsole has been initialized with title: %s, refcount: %d!", __func__, title, consoleImpl->nRefCount);

        // now we get rid of our hack
        consoleImpl->logState[std::this_thread::get_id()].sLoggerName = prevLoggerName;
    }
    else
    {
        if (bNewThread)
        {
            consoleImpl->SOLn("CConsole::%s() > Already initialized, but this is a new thread, and new refcount is: %d!", __func__, consoleImpl->nRefCount);
        }
        else
        {
            consoleImpl->SOLn("CConsole::%s() > Already initialized, new refcount is: %d!", __func__, consoleImpl->nRefCount);
        }
    }
#endif
} // Initialize()


/**
    If reference count is positive, it is decreased by 1.
    If reference count reaches 0, console window gets deleted.
    With this simple reference counting, different parts/layers of a process can invoke Initialize() and Deinitialize()
    at their initializing and deinitializing functions, without the fear of 1 single Deinitialize() call might ruin
    the console functionality of other parts of the process.
    In case of multiple threads using CConsole, all threads should invoke Initialize() once at thread startup, and invoke
    Deinitialize() once at thread shutdown.
*/
void CConsole::Deinitialize()
{
#ifdef CCONSOLE_IS_ENABLED   
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !consoleImpl )
        return;

    consoleImpl->nRefCount--;
    consoleImpl->OLn("CConsole::%s() new refcount: %d", __func__, consoleImpl->nRefCount);
    if ( consoleImpl->nRefCount == 0 )
    {
        this->~CConsole();
    }
#endif
} // Deinitialize()


/**
    Tells if console window is already initialized.
    In case of multiple threads using CConsole, all threads should invoke Initialize() once at thread startup, and invoke
    Deinitialize() once at thread shutdown.
    Note that if one thread successfully initializes CConsole, this function will return true even if invoked by some other thread, even if
    that other thread not yet had initialized CConsole. Still all threads should invoke Initialize() if they want to use CConsole,
    regardless of the return value of this function.

    @return True if console window is initialized, false otherwise.
*/
bool CConsole::isInitialized() const
{
    std::lock_guard<std::mutex> lock(mainMutex);
    return consoleImpl && (consoleImpl->bInited);
}


/**
    Gets the current indentation.
    Per-thread property.
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
    Per-thread property.
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
    Per-thread property.
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
    Per-thread property.
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
    Per-thread property.
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
    Per-thread property.
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
    Per-thread property.
*/
void CConsole::LoadColors()
{
#ifdef CCONSOLE_IS_ENABLED
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return;

    consoleImpl->LoadColors();
#endif
} // LoadColors()


/** 
    Saves current colors.
    Per-thread property.
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
    Per-thread property.
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
    Per-thread property.
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
    Per-thread property.
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
    Per-thread property.
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
    Per-thread property.
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
    Per-thread property.
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
    Per-thread property.
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
    Per-thread property.
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
    Per-thread property.
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
    Per-thread property.
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
    Per-thread property.
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
    Per-thread property.
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
    Per-thread property.
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
    Per-thread property.
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
    Per-thread property.
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
    Per-thread property.
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
    Per-thread property.
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
    Per-thread property.
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
    Per-thread property.
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
    Per-thread property.
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
    Error-mode off.
    Per-thread property.
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
    Per-thread property.
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
    Per-thread property.
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
    Per-process property.
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
    Per-process property.
*/
int CConsole::getSuccessOutsCount() const    
{
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return 0;

    return consoleImpl->getSuccessOutsCount();
} // getSuccessOutsCount()


/**
    Resets total count of printouts-with-newline during error-mode.
    Per-process property.
*/
void CConsole::ResetErrorOutsCount()
{
    std::lock_guard<std::mutex> lock(mainMutex);

    if (!(consoleImpl && (consoleImpl->bInited)))
        return;

    consoleImpl->ResetErrorOutsCount();
}

/**
    Resets total count of printouts-with-newline during success-mode.
    Per-process property.
*/
void CConsole::ResetSuccessOutsCount()
{
    std::lock_guard<std::mutex> lock(mainMutex);

    if (!(consoleImpl && (consoleImpl->bInited)))
        return;

    consoleImpl->ResetSuccessOutsCount();
}


/**
    O("%s", text).
*/
CConsole& CConsole::operator<<(const char* text)
{
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return *this;

    *consoleImpl << text;
    return *this;
} // operator<<()


/**
    O("%b", b).
*/
CConsole& CConsole::operator<<(const bool& b)
{
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return *this;

    *consoleImpl << b;
    return *this;
} // operator<<()


/**
    O("%d", n).
*/
CConsole& CConsole::operator<<(const int& n)
{
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return *this;

    *consoleImpl << n;
    return *this;
} // operator<<()


/**
    O("%f", f).
*/
CConsole& CConsole::operator<<(const float& f)
{
    std::lock_guard<std::mutex> lock(mainMutex);

    if ( !(consoleImpl && (consoleImpl->bInited)) )
        return *this;

    *consoleImpl << f;
    return *this;
} // operator<<()


/**
    Changes current mode or adds a new line.
    Based on value of fs, equals to calling EOn()/EOff()/SOn()/SOff()/NOn() accordingly.
    If fs is FormatSignal::NL, the behavior is same as OLn("").
*/
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
    // dtor is private, no need to use mutex here, and also it worth mentioning that
    // CConsole is existing as a single static instance, our mainMutex is also static,
    // so at the end of the running program, I'm not sure about the release order of
    // these resources but better not use that static mutex here.
    // And also, a mutex might throw exception, we should never throw exception in
    // dtor. If someone really needs mutex here, use a non-throwing mutex (see more on stackoverflow).

    if ( !consoleImpl )
        return;

    consoleImpl->OLn("CConsole::~CConsole() LAST MESSAGE, BYE!");
#endif

    delete consoleImpl;
    consoleImpl = NULL;
    // I know this might look unusual why I set sg to null in dtor, the answer is that the dtor is invoked
    // manually by Deinitialize() too, in such case CConsole instance is not destroyed completely, in such
    // case we need variables like this to be reset.
}