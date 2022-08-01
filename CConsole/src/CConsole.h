#pragma once

/*
    ###################################################################################
    CConsole.h
    Class to handle console window.
    Made by PR00F88
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/



// copied NULL from stdlib.h to avoid including big ass headers
/* Define NULL pointer value */
#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif


// copied WORD and DWORD from minwindef.h to avoid including big ass headers
#ifndef _MINWINDEF_
typedef unsigned long       DWORD;
typedef unsigned short      WORD;
#endif
    

/**
    Class handling console window.
    Intentionally pimpl instead of abstract interface, please don't try to change.
    Thread-safe.
*/

class CConsole
{
//#pragma message("  CConsole is included")

public:

    enum FormatSignal
    {
        NL, /* new line */
        S,  /* success mode */
        E,  /* error error */
        N   /* normal mode */
    };

    // ---------------------------------------------------------------------------

    static CConsole& getConsoleInstance(const char* loggerModule = "");   /**< Gets the singleton instance. */

    // ---------------------------------------------------------------------------

    void SetLoggingState(const char* loggerModule, bool state);  /**< Sets logging on or off for the given logger module. */
    void SetErrorsAlwaysOn(bool state);                          /**< Sets errors always appear irrespective of logging state of current logger module. */

    void Initialize(
        const char* title,
        bool createLogFile );     /**< This creates actually the console window. */
    void Deinitialize();          /**< This deletes the console window. */
    bool isInitialized() const;   /**< Tells if console window is already initialized. */

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
        WORD clr, const char* html = NULL );   /**< Sets foreground color. */
    WORD        getBGColor() const;            /**< Gets background color. */
    void        SetBGColor(WORD clr);          /**< Sets background color. */

    WORD        getIntsColor() const;          /**< Gets ints color. */
    const char* getIntsColorHtml() const;      /**< Gets ints html color. */
    void        SetIntsColor(
        WORD clr, const char* html = NULL );   /**< Sets ints color. */

    WORD        getStringsColor() const;       /**< Gets strings color. */
    const char* getStringsColorHtml() const;   /**< Gets strings html color. */
    void        SetStringsColor(
        WORD clr, const char* html = NULL );   /**< Sets strings color. */

    WORD        getFloatsColor() const;        /**< Gets floats color. */
    const char* getFloatsColorHtml() const;    /**< Gets floats html color. */
    void        SetFloatsColor(
        WORD clr, const char* html = NULL );   /**< Sets floats color. */

    WORD        getBoolsColor() const;         /**< Gets bools color. */
    const char* getBoolsColorHtml() const;     /**< Gets bools html color. */
    void        SetBoolsColor(
        WORD clr, const char* html = NULL );   /**< Sets bools color. */

    void O(const char* text, ...);       /**< Prints text to console. */
    void OLn(const char* text, ...);     /**< Prints text to console and adds a new line. */
    void OI();                           /**< Indent(). */
    void OIO(const char* text, ...);     /**< OI() + O(text). */
    void OIOLn(const char* text, ...);   /**< OI() + OLn(text). */
    void OLnOI(const char* text, ...);   /**< OLn(text) + OI(). */
    void OIb(int value);                 /**< IndentBy(). */
    void OO();                           /**< Outdent(). */
    void OOO(const char* text, ...);     /**< OO() + O(text). */
    void OOOLn(const char* text, ...);   /**< OO() + OLn(text). */
    void OLnOO(const char* text, ...);   /**< OLn(text) + OO(). */
    void OOb(int value);                 /**< OutdentBy(). */
    void OIOLnOO(const char* text, ...); /**< OI() + OLn(text) + OO(). */
    void L(int n = 20);                  /**< Prints line to console and adds a new line. */

    void NOn();                          /**< Normal-mode on. */ 
    void EOn();                          /**< Error-mode on. */
    void EOff();                         /**< Error-mode  off. */
    void SOn();                          /**< Success-mode on. */
    void SOff();                         /**< Success-mode off. */
                                        
    void SO(const char* text, ...);      /**< SOn() + O(text) + SOff(). */
    void SOLn(const char* text, ...);    /**< SOn() + OLn(text) + SOff(). */
    void EO(const char* text, ...);      /**< EOn() + O(text) + EOff(). */
    void EOLn(const char* text, ...);    /**< EOn() + OLn(text) + EOff(). */

    void OISO(const char* text, ...);    /**< OI() + SO(text). */
    void OISOLn(const char* text, ...);  /**< OI() + SOLn(text). */
    void OOSO(const char* text, ...);    /**< OO() + SO(text). */
    void OOSOLn(const char* text, ...);  /**< OO() + SOLn(text). */
    void OIEO(const char* text, ...);    /**< OI() + EO(text). */
    void OIEOLn(const char* text, ...);  /**< OI() + EOLn(text). */
    void OOEO(const char* text, ...);    /**< OO() + EO(text). */
    void OOEOLn(const char* text, ...);  /**< OO() + EOLn(text). */

    void SOOI(const char* text, ...);    /**< SO(text) + OI(). */
    void SOLnOI(const char* text, ...);  /**< SOLn(text) + OI(). */
    void SOOO(const char* text, ...);    /**< SO(text) + OO(). */
    void SOLnOO(const char* text, ...);  /**< SOLn(text) + OO(). */
    void EOOI(const char* text, ...);    /**< EO(text) + OI(). */
    void EOLnOI(const char* text, ...);  /**< EOLn(text) + OI(). */
    void EOOO(const char* text, ...);    /**< EO(text) + OO(). */
    void EOLnOO(const char* text, ...);  /**< EOLn(text) + OO(). */

    void OISOOO(const char* text, ...);    /**< OI() + SO(text) + OO(). */
    void OISOLnOO(const char* text, ...);  /**< OI() + SOLn(text) + OO(). */
    void OIEOOO(const char* text, ...);    /**< OI() + EO(text) + OO(). */
    void OIEOLnOO(const char* text, ...);  /**< OI() + EOLn(text) + OO(). */

    int getErrorOutsCount() const;      /**< Gets total count of printouts-with-newline during error-mode. */
    int getSuccessOutsCount() const;    /**< Gets total count of printouts-with-newline during success-mode. */

    CConsole& operator<<(const char* text);
    CConsole& operator<<(const bool& b);
    CConsole& operator<<(const int& n);
    CConsole& operator<<(const float& f);
    CConsole& operator<<(const CConsole::FormatSignal& fs); 


private:

    static CConsole consoleInstance;

    class CConsoleImpl;
    CConsoleImpl* consoleImpl;

    // ---------------------------------------------------------------------------

    CConsole(); 
    CConsole(const CConsole&);
    CConsole& operator= (const CConsole&);
    virtual ~CConsole();

}; // class CConsole


