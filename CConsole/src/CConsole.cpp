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
#include "../../../PFL/PFL/PFL.h"

#include <fstream>
#include <iostream>
#include <set>
#include <string>


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



/*
   CConsole::CConsoleImpl
   ###########################################################################
*/

class CConsole::CConsoleImpl
{
public:

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

    void ImmediateWriteString(
        CConsole* pCaller, const char* text);    /**< Directly writes formatted string value to the console. */
    void ImmediateWriteBool(
        CConsole* pCaller, bool b);              /**< Directly writes formatted boolean value to the console. */
    void ImmediateWriteInt(
        CConsole* pCaller, int n);               /**< Directly writes formatted signed integer value to the console. */
    void ImmediateWriteUInt(
        CConsole* pCaller, unsigned int n);      /**< Directly writes formatted unsigned integer value to the console. */
    void ImmediateWriteFloat(
        CConsole* pCaller, float f);             /**< Directly writes formatted floating-point value to the console. */
    
    void WriteText(const char* text);            /**< Directly writes unformatted text to the console. */
    void WriteFormattedTextEx(
        CConsole* pCaller,
        const char* fmt, va_list list);          /**< Writes text to the console. */

    void WriteFormattedTextExCaller(
        CConsole* pCaller,
        const char* fmt, va_list list, bool nl);  /**< Writes text to the console. */

    void SaveColorsExCaller();            /**< Saves current colors. */
    void RestoreDefaultColorsExCaller();  /**< Restores default colors. */

    friend class CConsole;

}; // class CConsoleImpl



// ############################### PUBLIC ################################


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
    RestoreDefaultColorsExCaller();
    SaveColorsExCaller();
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
void CConsole::CConsoleImpl::ImmediateWriteString(CConsole* pCaller, const char* text)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !canWeWriteBasedOnFilterSettings() )
        return;

    oldClrFG = clrFG;
    //oldClrFGhtml = clrFGhtml;
    if ( text != NULL )
    {
        pCaller->SetFGColor(clrStrings);                             
        WriteConsoleA(hConsole, text, strlen(text), &wrt, 0);
        if ( bAllowLogFile )
            fLog << "<font color=\"#" << clrStringsHtml << "\">" << text << "</font>";
    }
    else
    {
        pCaller->SetFGColor(oldClrFG);
        WriteConsoleA(hConsole, "NULL", 4, &wrt, 0);
        if ( bAllowLogFile )
            fLog << "<font color=\"#" << clrStringsHtml << "\">NULL</font>";
    }
    pCaller->SetFGColor(oldClrFG);
#endif
} // ImmediateWriteString()


/**
    Directly writes formatted boolean value to the console.
    Used by WriteFormattedTextEx() and operator<<()s.
*/
void CConsole::CConsoleImpl::ImmediateWriteBool(CConsole* pCaller, bool l)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !canWeWriteBasedOnFilterSettings() )
        return;

    oldClrFG = clrFG;
    pCaller->SetFGColor(clrBools);
    WriteConsoleA(hConsole, l ? "true" : "false", l ? 4 : 5, &wrt, 0);
    if ( bAllowLogFile )
        fLog << "<font color=\"#" << clrBoolsHtml << "\">" << (l ? "true" : "false") << "</font>";
    pCaller->SetFGColor(oldClrFG);
#endif
} // ImmediateWriteBool()


/**
    Directly writes formatted signed integer value to the console.
    Used by WriteFormattedTextEx() and operator<<()s.
*/
void CConsole::CConsoleImpl::ImmediateWriteInt(CConsole* pCaller, int n)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !canWeWriteBasedOnFilterSettings() )
        return;

    oldClrFG = clrFG;
    pCaller->SetFGColor(clrInts);
    itoa(n,vmi,10);
    WriteConsoleA(hConsole, vmi, strlen(vmi), &wrt, 0);
    if ( bAllowLogFile )
        fLog << "<font color=\"#" << clrIntsHtml << "\">" << vmi << "</font>";
    pCaller->SetFGColor(oldClrFG);
#endif
} // ImmediateWriteInt()


/**
    Directly writes formatted unsigned integer value to the console.
    Used by WriteFormattedTextEx() and operator<<()s.
*/
void CConsole::CConsoleImpl::ImmediateWriteUInt(CConsole* pCaller, unsigned int n)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !canWeWriteBasedOnFilterSettings() )
        return;

    oldClrFG = clrFG;
    pCaller->SetFGColor(clrInts);
    sprintf(vmi, "%u", n);
    WriteConsoleA(hConsole, vmi, strlen(vmi), &wrt, 0);
    if ( bAllowLogFile )
        fLog << "<font color=\"#" << clrIntsHtml << "\">" << vmi << "</font>";
    pCaller->SetFGColor(oldClrFG);
#endif
} // ImmediateWriteUInt()


/**
    Directly writes formatted floating-point value to the console.
    Used by WriteFormattedTextEx() and operator<<()s.
*/
void CConsole::CConsoleImpl::ImmediateWriteFloat(CConsole* pCaller, float f)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !canWeWriteBasedOnFilterSettings() )
        return;

    oldClrFG = clrFG;
    sprintf(vmi, "%0.4f", f);
    size_t newlen = strlen(vmi);
    for (size_t blah = strlen(vmi); (vmi[blah] == '0') || (vmi[blah] == 0); blah--)
        newlen--;
    vmi[newlen] = '\0';
    pCaller->SetFGColor(clrFloats);
    WriteConsoleA(hConsole, vmi, newlen, &wrt, 0);
    if ( bAllowLogFile )
        fLog << "<font color=\"#" << clrFloatsHtml << "\">" << vmi << "</font>";
    pCaller->SetFGColor(oldClrFG);
#endif
} // ImmediateWriteFloat()


/**
    Directly writes unformatted text to the console.
    Used by WriteFormattedTextEx(), WriteFormattedTextExCaller() and operator<<()s.
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
void CConsole::CConsoleImpl::WriteFormattedTextEx(CConsole* pCaller, const char* fmt, va_list list)
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
        pCaller->SetFGColor(oldClrFG);
        WriteText(fmt);
    }
    else
    {
        for (p = fmt; *p; ++p)
        {
            if ( *p != '%' )
            {
                pCaller->SetFGColor(oldClrFG);
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
                        ImmediateWriteString(pCaller, r);
                        continue;
                    }
                case 'i':
                case 'd':
                    {
                        e = va_arg(list, int);
                        ImmediateWriteInt(pCaller,e);
                        continue;
                    }
                case 'u':
                    {
                        ue = va_arg(list, unsigned int);
                        ImmediateWriteUInt(pCaller,ue);
                        continue;
                    }
                case 'b':
                    {
                        l = va_arg(list, bool);
                        ImmediateWriteBool(pCaller,l);
                        continue;
                    }
                case 'f':
                    {                                                        
                        f = (float) va_arg(list, double);
                        ImmediateWriteFloat(pCaller,f);
                        continue;
                    }
                default:
                    {
                        pCaller->SetFGColor(oldClrFG);
                        WriteConsoleA(hConsole, p, sizeof(char), &wrt, 0);
                        if ( bAllowLogFile )
                            fLog << *p;
                    }
                } // switch
            } // else
        } // for p
    } // else
    bFirstWriteTextCallAfterWriteTextLn = ( strstr(fmt, "\n") != NULL );
    pCaller->SetFGColor(oldClrFG);
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
void CConsole::CConsoleImpl::WriteFormattedTextExCaller(CConsole* pCaller, const char* fmt, va_list list, bool nl)
{           
    if ( !canWeWriteBasedOnFilterSettings() )
        return;

    WriteFormattedTextEx(pCaller, fmt, list);
    if ( nl )
    {
        WriteText("\n\r");
        if ( nMode == 1 )
            nErrorOutCount++;
        else if ( nMode == 2 )
            nSuccessOutCount++;
    }
} // WriteFormattedTextExCaller()


void CConsole::CConsoleImpl::SaveColorsExCaller()
{
#ifdef CCONSOLE_IS_ENABLED
    dLastFGColor      = clrFG;
    dLastStringsColor = clrStrings;
    dLastFloatsColor  = clrFloats;
    dLastIntsColor    = clrInts;
    dLastBoolsColor   = clrBools;
    strcpy(dLastFGColorHtml, clrFGhtml);
    strcpy(dLastStringsColorHtml, clrStringsHtml);
    strcpy(dLastFloatsColorHtml, clrFloatsHtml);
    strcpy(dLastIntsColorHtml, clrIntsHtml);
    strcpy(dLastBoolsColorHtml, clrBoolsHtml);
#endif
}


void CConsole::CConsoleImpl::RestoreDefaultColorsExCaller()
{
#ifdef CCONSOLE_IS_ENABLED
    nMode = 0;
    clrFG = CConsoleImpl::CCONSOLE_DEF_CLR_FG;
    clrBG = 0;
    clrInts    = clrFG;
    clrFloats  = clrFG;
    clrStrings = clrFG;
    clrBools   = clrFG;
    strcpy(clrFGhtml, "999999");
    strcpy(clrIntsHtml, clrFGhtml);
    strcpy(clrFloatsHtml, clrFGhtml);
    strcpy(clrStringsHtml, clrFGhtml);
    strcpy(clrBoolsHtml, clrFGhtml);
#endif
}


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
    if ( !isInitialized() )
        return;
    
    const size_t sizeOfLoggerModuleNameBuffer = sizeof(char)*(strlen(loggerModule)+1);
    char* const newNameLoggerModule = (char* const )malloc(sizeOfLoggerModuleNameBuffer);
    if ( newNameLoggerModule == nullptr )
        return;

    strncpy_s(newNameLoggerModule, sizeOfLoggerModuleNameBuffer, loggerModule, sizeOfLoggerModuleNameBuffer);
    PFL::strClr(newNameLoggerModule);
    if ( strlen(newNameLoggerModule) == 0 )
    {
        free(newNameLoggerModule);
        return;
    }
    free(newNameLoggerModule);

    if ( state )
    {
        consoleImpl->enabledModules.insert(loggerModule);
    }
    else 
    {
        consoleImpl->enabledModules.erase(loggerModule);
    }
} // SetLoggingState 


/**
    Sets errors always appear irrespective of logging state of current logger module.
    Default value is true.

    @param state True will make module error logs appear even if module logging state is false for the current module.
                 False will let module errors logs be controlled purely by module logging states.
*/
void CConsole::SetErrorsAlwaysOn(bool state)
{
    if ( !isInitialized() )
        return;

    consoleImpl->bErrorsAlwaysOn = state;
} // SetErrorsAlwaysOn()


/**
    This creates actually the console window if not created yet.
    An internal reference count is also increased by 1. Reference count explanation is described at Deinitialize().
*/
void CConsole::Initialize(const char* title, bool createLogFile)
{
#ifdef CCONSOLE_IS_ENABLED
    consoleImpl->nRefCount++;

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
        RestoreDefaultColors();
        consoleImpl->bAllowLogFile = createLogFile;
        if ( createLogFile )
        {
            consoleImpl->fLog.open("log.html");
            if ( consoleImpl->fLog.fail() )
            {
                consoleImpl->bAllowLogFile = false;
                EOLn("ERROR: Couldn't open output html for writing!");
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

        SOLn(" > CConsole has been initialized with title: %s, refcount: %d!", title, consoleImpl->nRefCount);

        // now we get rid of our hack
        consoleImpl->loggerName = prevLoggerName;

    }
    else
    {
        SOLn(" > CConsole is already initialized, new refcount: %d!", consoleImpl->nRefCount);
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
    if ( !isInitialized() )
        return;

    consoleImpl->nRefCount--;
    OLn("CConsole::Deinitialize() new refcount: %d", consoleImpl->nRefCount);
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
bool CConsole::isInitialized() const {
    return consoleImpl && (consoleImpl->bInited);
}


/**
    Gets the current indentation.
*/
int CConsole::getIndent() const
{
    if ( !isInitialized() )
        return 0;

    return consoleImpl->nIndentValue;
} // getIndent()


/**
    Sets the current indentation.
*/
void CConsole::SetIndent(int value)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    consoleImpl->nIndentValue = value;
    if ( consoleImpl->nIndentValue < 0 )
        consoleImpl->nIndentValue = 0;
#endif
} // SetIndent()


/**
    Increases indentation.
*/
void CConsole::Indent()
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    consoleImpl->nIndentValue += CConsoleImpl::CCONSOLE_INDENTATION_CHANGE;
#endif
} // Indent()


/**
    Increases indentation by the given value.
*/
void CConsole::IndentBy(int value)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    consoleImpl->nIndentValue += value;
    if ( consoleImpl->nIndentValue < 0 )
        consoleImpl->nIndentValue = 0;
#endif
} // IndentBy()


/**
    Decreases indentation.
*/
void CConsole::Outdent()
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    consoleImpl->nIndentValue -= CConsoleImpl::CCONSOLE_INDENTATION_CHANGE;
    if ( consoleImpl->nIndentValue < 0 )
        consoleImpl->nIndentValue = 0;
#endif
} // Outdent()


/**
    Decreases indentation by the given value.
*/
void CConsole::OutdentBy(int value)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    consoleImpl->nIndentValue -= value;
    if ( consoleImpl->nIndentValue < 0 )
        consoleImpl->nIndentValue = 0;
#endif
} // OutdentBy()


/**
    Loads previously saved colors.
*/
void CConsole::LoadColors()
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    SetFGColor(consoleImpl->dLastFGColor, consoleImpl->dLastBoolsColorHtml);
    SetStringsColor(consoleImpl->dLastStringsColor, consoleImpl->dLastStringsColorHtml);
    SetFloatsColor(consoleImpl->dLastFloatsColor, consoleImpl->dLastFloatsColorHtml);
    SetIntsColor(consoleImpl->dLastIntsColor, consoleImpl->dLastIntsColorHtml);
    SetBoolsColor(consoleImpl->dLastBoolsColor, consoleImpl->dLastBoolsColorHtml);
#endif
} // LoadColors()


/** 
    Saves current colors.
*/
void CConsole::SaveColors()
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    consoleImpl->SaveColorsExCaller();
#endif
} // SaveColors()


/**
    Restores default colors.
*/
void CConsole::RestoreDefaultColors()
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    consoleImpl->RestoreDefaultColorsExCaller();
#endif
} // RestoreDefaultColors()


/**
    Gets foreground color.
*/
WORD CConsole::getFGColor() const
{
    if ( !isInitialized() )
        return 0;

    return consoleImpl->clrFG;
} // getFGColor()


/**
    Gets html foreground color.
*/
const char* CConsole::getFGColorHtml() const
{
    if ( !isInitialized() )
        return "#DDBEEF";

    return consoleImpl->clrFGhtml;
} // getFGColorHtml()


/**
    Sets foreground color.
*/
void CConsole::SetFGColor(WORD clr, const char* html)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    consoleImpl->clrFG = clr;
    SetConsoleTextAttribute(consoleImpl->hConsole, consoleImpl->clrFG | consoleImpl->clrBG);
    if ( html )
        strcpy_s(consoleImpl->clrFGhtml, CConsoleImpl::HTML_CLR_S, html);
#endif
} // SetFGColor()


/**
    Gets background color.
*/
WORD CConsole::getBGColor() const
{
    if ( !isInitialized() )
        return 0;

    return consoleImpl->clrBG;
} // getBGColor()


/**
    Sets background color.
*/
void CConsole::SetBGColor(WORD clr)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    consoleImpl->clrBG = clr;
    SetConsoleTextAttribute(consoleImpl->hConsole, consoleImpl->clrFG | consoleImpl->clrBG);
#endif
} // SetBGColor()


/**
    Gets ints color.
*/
WORD CConsole::getIntsColor() const
{
    if ( !isInitialized() )
        return 0;

    return consoleImpl->clrInts;
} // getIntsColor()


/**
    Gets ints html color.
*/
const char* CConsole::getIntsColorHtml() const
{
    if ( !isInitialized() )
        return "#DDBEEF";

    return consoleImpl->clrIntsHtml;
} // getIntsColorHtml()


/**
    Sets ints color.
*/
void CConsole::SetIntsColor(WORD clr, const char* html)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    consoleImpl->clrInts = clr;
    if ( html )
        strcpy_s(consoleImpl->clrIntsHtml, CConsoleImpl::HTML_CLR_S, html);
#endif
} // SetIntsColor()


/**
    Gets strings color.
*/
WORD CConsole::getStringsColor() const
{
    if ( !isInitialized() )
        return 0;

    return consoleImpl->clrStrings;
} // getStringsColor()


/**
    Gets strings html color.
*/
const char* CConsole::getStringsColorHtml() const
{
    if ( !isInitialized() )
        return "#DDBEEF";

    return consoleImpl->clrStringsHtml;
} // getStringsColorHtml()


/**
    Sets strings color.
*/
void CConsole::SetStringsColor(WORD clr, const char* html)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    consoleImpl->clrStrings = clr;
    if ( html )
        strcpy_s(consoleImpl->clrStringsHtml, CConsoleImpl::HTML_CLR_S, html);
#endif
} // SetStringsColor()


/**
    Gets floats color.
*/
WORD CConsole::getFloatsColor() const
{
    if ( !isInitialized() )
        return 0;

    return consoleImpl->clrFloats;
} // getFloatsColor()


/**
    Gets floats html color.
*/
const char* CConsole::getFloatsColorHtml() const
{
    if ( !isInitialized() )
        return "#DDBEEF";

    return consoleImpl->clrFloatsHtml;
} // getFloatsColorHtml()


/**
    Sets floats color.
*/
void CConsole::SetFloatsColor(WORD clr, const char* html)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    consoleImpl->clrFloats = clr;
    if ( html )
        strcpy_s(consoleImpl->clrFloatsHtml, CConsoleImpl::HTML_CLR_S, html);
#endif
} // SetFloatsColor()


/**
    Gets bools color.
*/
WORD CConsole::getBoolsColor() const
{
    if ( !isInitialized() )
        return 0;

    return consoleImpl->clrBools;
} // getBoolsColor()


/**
    Gets bools html color.
*/
const char* CConsole::getBoolsColorHtml() const
{
    if ( !isInitialized() )
        return "#DDBEEF";

    return consoleImpl->clrBoolsHtml;
} // getBoolsColorHtml()


/**
    Sets bools color.
*/
void CConsole::SetBoolsColor(WORD clr, const char* html)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    consoleImpl->clrBools = clr;
    if ( html )
        strcpy_s(consoleImpl->clrBoolsHtml, CConsoleImpl::HTML_CLR_S, html);
#endif
} // SetBoolsColor()


/**
    Prints text to console.
*/
void CConsole::O(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    va_list list;
    va_start(list, text);
    consoleImpl->WriteFormattedTextExCaller(this, text, list, false);
    va_end(list);
#endif
} // O()


/**
    Prints text to console and adds a new line.
*/
void CConsole::OLn(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    va_list list;
    va_start(list, text);
    consoleImpl->WriteFormattedTextExCaller(this, text, list, true);
    va_end(list);
#endif
} // OLn()


/**
    Indent().
*/
void CConsole::OI()
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    Indent();
#endif
} // OI()


/**
    OI() + O(text).
*/
void CConsole::OIO(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    va_list list;
    OI();
    va_start(list, text);
    consoleImpl->WriteFormattedTextExCaller(this, text, list, false);
    va_end(list);
#endif
} // OI()


/**
    OI() + OLn(text).
*/
void CConsole::OIOLn(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    va_list list;
    OI();
    va_start(list, text);
    consoleImpl->WriteFormattedTextExCaller(this, text, list, true);
    va_end(list);
#endif
} // OIOLn()


/**
    OLn(text) + OI().
*/
void CConsole::OLnOI(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    va_list list;
    va_start(list, text);
    consoleImpl->WriteFormattedTextExCaller(this, text, list, true);
    va_end(list);
    OI();
#endif
} // OLnOI()


/**
    IndentBy().
*/
void CConsole::OIb(int value)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    IndentBy(value);
#endif
} // OIb()


/**
    Outdent().
*/
void CConsole::OO()
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    Outdent();
#endif
} // OO()


/**
    OO() + O(text).
*/
void CConsole::OOO(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    va_list list;
    OO();
    va_start(list, text);
    consoleImpl->WriteFormattedTextExCaller(this, text, list, false);
    va_end(list);
#endif
} // OO()


/**
    OO() + OLn(text).
*/
void CConsole::OOOLn(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    va_list list;
    OO();
    va_start(list, text);
    consoleImpl->WriteFormattedTextExCaller(this, text, list, true);
    va_end(list);
#endif
} // OOLn()


/**
    OLn(text) + OO().
*/
void CConsole::OLnOO(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    va_list list;
    va_start(list, text);
    consoleImpl->WriteFormattedTextExCaller(this, text, list, true);
    va_end(list);
    OO();
#endif
} // OLnOI()


/**
    OutdentBy().
*/
void CConsole::OOb(int value)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    OutdentBy(value);
#endif
} // OOb()


/**
    OI() + OLn(text) + OO().
*/
void CConsole::OIOLnOO(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    va_list list;
    OI();
    va_start(list, text);
    consoleImpl->WriteFormattedTextExCaller(this, text, list, true);
    va_end(list);
    OO();
#endif
}


/**
    Prints line to console and adds a new line.
    @param n How many times the "-=" chars should be repeated.
*/
void CConsole::L(int n)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    for (int i = 0; i < n; i++)
        O("-=");
    OLn("-");
#endif
} // L()


/**
    Normal-mode on.
*/
void CConsole::NOn()
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    consoleImpl->nMode = 0;
    LoadColors();
#endif
} // NOn()


/**
    Error-mode on.
*/
void CConsole::EOn()
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    if ( consoleImpl->nMode == 1 )
        return;
    else if ( consoleImpl->nMode == 2 )
        SOff();
    consoleImpl->nMode = 1;
    SaveColors();
    SetFGColor( FOREGROUND_RED | FOREGROUND_INTENSITY, "FF0000" );
    SetStringsColor(FOREGROUND_RED | FOREGROUND_GREEN, "DDDD00" );
    SetFloatsColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "FFFF00" );
    SetIntsColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "FFFF00" );
    SetBoolsColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "FFFF00" );
#endif
} // EOn()


/**
    Error-mode  off.
*/
void CConsole::EOff()
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    NOn();
#endif
} // EOff()


/**
    Success-mode on.
*/
void CConsole::SOn()
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    if ( consoleImpl->nMode == 2 )
        return;
    else if ( consoleImpl->nMode == 1 )
        EOff();
    consoleImpl->nMode = 2;
    SaveColors();
    SetFGColor( FOREGROUND_GREEN | FOREGROUND_INTENSITY, "00FF00" );
    SetStringsColor(FOREGROUND_RED | FOREGROUND_GREEN, "DDDD00" );
    SetFloatsColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "FFFF00" );
    SetIntsColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "FFFF00" );
    SetBoolsColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "FFFF00" );
#endif
} // SOn()


/**
    Success-mode off. 
*/
void CConsole::SOff()
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    NOn();
#endif
} // SOff()


/**
    SOn() + O(text) + SOff().
*/
void CConsole::SO(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    va_list list;
    SOn();
    va_start(list, text);
    consoleImpl->WriteFormattedTextExCaller(this, text, list, false);
    va_end(list);
    SOff();
#endif
} // SO()


/**
    SOn() + OLn(text) + SOff().
*/
void CConsole::SOLn(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    va_list list;
    SOn();
    va_start(list, text);
    consoleImpl->WriteFormattedTextExCaller(this, text, list, true);
    va_end(list);
    SOff();
#endif
} // SOln()


/**
    EOn() + O(text) + EOff().
*/
void CConsole::EO(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    va_list list;
    EOn();
    va_start(list, text);
    consoleImpl->WriteFormattedTextExCaller(this, text, list, false);
    va_end(list);
    EOff();
#endif
} // EO()


/**
    EOn() + OLn(text) + EOff().
*/
void CConsole::EOLn(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    va_list list;
    EOn();
    va_start(list, text);
    consoleImpl->WriteFormattedTextExCaller(this, text, list, true);
    va_end(list);
    EOff();
#endif
} // EOln()


/**
    OI() + SO(text).
*/
void CConsole::OISO(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    va_list list;
    OI();
    SOn();
    va_start(list, text);
    consoleImpl->WriteFormattedTextExCaller(this, text, list, false);
    va_end(list);
    SOff();
#endif
} // OISO()


/**
    OI() + SOLn(text).
*/
void CConsole::OISOLn(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    va_list list;
    OI();
    SOn();
    va_start(list, text);
    consoleImpl->WriteFormattedTextExCaller(this, text, list, true);
    va_end(list);
    SOff();
#endif
} // OISOLn()


/**
    OO() + SO(text).
*/
void CConsole::OOSO(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    va_list list;
    OO();
    SOn();
    va_start(list, text);
    consoleImpl->WriteFormattedTextExCaller(this, text, list, false);
    va_end(list);
    SOff();
#endif
} // OOSO()


/**
    OO() + SOLn(text).
*/
void CConsole::OOSOLn(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    va_list list;
    OO();
    SOn();
    va_start(list, text);
    consoleImpl->WriteFormattedTextExCaller(this, text, list, true);
    va_end(list);
    SOff();
#endif
} // OOSOLn()


/**
    OI() + EO(text).
*/
void CConsole::OIEO(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    va_list list;
    OI();
    EOn();
    va_start(list, text);
    consoleImpl->WriteFormattedTextExCaller(this, text, list, false);
    va_end(list);
    EOff();
#endif
} // OIEO()


/**
    OI() + EOLn(text).
*/
void CConsole::OIEOLn(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    va_list list;
    OI();
    EOn();
    va_start(list, text);
    consoleImpl->WriteFormattedTextExCaller(this, text, list, true);
    va_end(list);
    EOff();
#endif
} // OIEOLn()


/**
    OO() + EO(text).
*/
void CConsole::OOEO(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    va_list list;
    OO();
    EOn();
    va_start(list, text);
    consoleImpl->WriteFormattedTextExCaller(this, text, list, false);
    va_end(list);
    EOff();
#endif
} // OOEO()


/**
    OO() + EOLn(text).
*/
void CConsole::OOEOLn(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    va_list list;
    OO();
    EOn();
    va_start(list, text);
    consoleImpl->WriteFormattedTextExCaller(this, text, list, true);
    va_end(list);
    EOff();
#endif
} // OOEOLn()


/**
    SO(text) + OI().
*/
void CConsole::SOOI(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    va_list list;
    SOn();
    va_start(list, text);
    consoleImpl->WriteFormattedTextExCaller(this, text, list, false);
    va_end(list);
    SOff();
    OI();
#endif
} // SOOI()


/**
    SOLn(text) + OI().
*/
void CConsole::SOLnOI(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    va_list list;
    SOn();
    va_start(list, text);
    consoleImpl->WriteFormattedTextExCaller(this, text, list, true);
    va_end(list);
    SOff();
    OI();
#endif
} // SOLnOI()


/**
    SO(text) + OO().
*/
void CConsole::SOOO(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    va_list list;
    SOn();
    va_start(list, text);
    consoleImpl->WriteFormattedTextExCaller(this, text, list, false);
    va_end(list);
    SOff();
    OO();
#endif
} // SOOO()


/**
    SOLn(text) + OO().
*/
void CConsole::SOLnOO(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    va_list list;
    SOn();
    va_start(list, text);
    consoleImpl->WriteFormattedTextExCaller(this, text, list, true);
    va_end(list);
    SOff();
    OO();
#endif
} // SOLnOO()


/**
    EO(text) + OI().
*/
void CConsole::EOOI(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    va_list list;
    EOn();
    va_start(list, text);
    consoleImpl->WriteFormattedTextExCaller(this, text, list, false);
    va_end(list);
    EOff();
    OI();
#endif
} // EOOI()


/**
    EOLn(text) + OI().
*/
void CConsole::EOLnOI(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    va_list list;
    EOn();
    va_start(list, text);
    consoleImpl->WriteFormattedTextExCaller(this, text, list, true);
    va_end(list);
    EOff();
    OI();
#endif
} // EOLnOI()


/**
    EO(text) + OO().
*/
void CConsole::EOOO(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    va_list list;
    EOn();
    va_start(list, text);
    consoleImpl->WriteFormattedTextExCaller(this, text, list, false);
    va_end(list);
    EOff();
    OO();
#endif
} // EOOO()


/**
    EOLn(text) + OO().
*/
void CConsole::EOLnOO(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    va_list list;
    EOn();
    va_start(list, text);
    consoleImpl->WriteFormattedTextExCaller(this, text, list, true);
    va_end(list);
    EOff();
    OO();
#endif
} // EOLnOO()


/**
    OI() + SO(text) + OO().
*/
void CConsole::OISOOO(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    va_list list;
    OI();
    SOn();
    va_start(list, text);
    consoleImpl->WriteFormattedTextExCaller(this, text, list, false);
    va_end(list);
    SOff();
    OO();
#endif
} // OISOOO


/**
    OI() + SOLn(text) + OO().
*/
void CConsole::OISOLnOO(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    va_list list;
    OI();
    SOn();
    va_start(list, text);
    consoleImpl->WriteFormattedTextExCaller(this, text, list, true);
    va_end(list);
    SOff();
    OO();
#endif
} // OISOLnOO


/**
    OI() + EO(text) + OO().
*/
void CConsole::OIEOOO(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    va_list list;
    OI();
    EOn();
    va_start(list, text);
    consoleImpl->WriteFormattedTextExCaller(this, text, list, false);
    va_end(list);
    EOff();
    OO();
#endif
} // OIEOOO


/**
    OI() + EOLn(text) + OO().
*/
void CConsole::OIEOLnOO(const char* text, ...)
{
#ifdef CCONSOLE_IS_ENABLED
    if ( !isInitialized() )
        return;

    va_list list;
    OI();
    EOn();
    va_start(list, text);
    consoleImpl->WriteFormattedTextExCaller(this, text, list, true);
    va_end(list);
    EOff();
    OO();
#endif
} // OIEOLnOO


/**
    Gets total count of printouts-with-newline during error-mode.
*/
int CConsole::getErrorOutsCount() const
{
    if ( !isInitialized() )
        return 0;

    return consoleImpl->nErrorOutCount;
} // getErrorOutsCount()


/**
    Gets total count of printouts-with-newline during success-mode.
*/
int CConsole::getSuccessOutsCount() const    
{
    if ( !isInitialized() )
        return 0;

    return consoleImpl->nSuccessOutCount;
} // getSuccessOutsCount()


CConsole& CConsole::operator<<(const char* text)
{
    if ( !isInitialized() )
        return *this;

    if ( consoleImpl->bFirstWriteTextCallAfterWriteTextLn )
        for (int i = 0; i < consoleImpl->nIndentValue; i++)
            consoleImpl->WriteText(" ");
    consoleImpl->ImmediateWriteString(this, text);
    return *this;
} // operator<<()

CConsole& CConsole::operator<<(const bool& b)
{
    if ( !isInitialized() )
        return *this;

    if ( consoleImpl->bFirstWriteTextCallAfterWriteTextLn )
        for (int i = 0; i < consoleImpl->nIndentValue; i++)
            consoleImpl->WriteText(" ");
    consoleImpl->ImmediateWriteBool(this, b);
    return *this;
} // operator<<()

CConsole& CConsole::operator<<(const int& n)
{
    if ( !isInitialized() )
        return *this;

    if ( consoleImpl->bFirstWriteTextCallAfterWriteTextLn )
        for (int i = 0; i < consoleImpl->nIndentValue; i++)
            consoleImpl->WriteText(" ");
    consoleImpl->ImmediateWriteInt(this, n);
    return *this;
} // operator<<()

CConsole& CConsole::operator<<(const float& f)
{
    if ( !isInitialized() )
        return *this;

    if ( consoleImpl->bFirstWriteTextCallAfterWriteTextLn )
        for (int i = 0; i < consoleImpl->nIndentValue; i++)
            consoleImpl->WriteText(" ");
    consoleImpl->ImmediateWriteFloat(this, f);
    return *this;
} // operator<<()

CConsole& CConsole::operator<<(const CConsole::FormatSignal& fs)
{
    if ( !isInitialized() )
        return *this;

    switch (fs)
    {
    case NL: OLn(""); break;
    case  S: SOn(); break;
    case  E: EOn(); break;
    default: NOn(); break;
    }
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
    if ( !isInitialized() )
        return;

    OLn("CConsole::~CConsole() LAST MESSAGE, BYE!");
#endif

    consoleImpl->hConsole = NULL;
    consoleImpl->bInited = false;
    consoleImpl->nMode = 0;

    delete consoleImpl;
    consoleImpl = NULL;
}