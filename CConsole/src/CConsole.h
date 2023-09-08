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
    Class handling a console window.
    Intentionally pimpl instead of abstract interface, please don't try to change.
    Thread-safe.
    Unfortunately logs are not buffered, instead they are written to console and log file
    immediately. This makes logging a bit slower, but currently there is no need for a more
    efficient behavior.

    Known issues:
    A) Reference-counting is done per-process, not per-thread.
       Reference count is increased with every call to Initialize().
       When Deinitialize() is invoked, we cannot decide when we can
       free up per-thread log state data. We free up per-thread state data only when
       reference count reaches 0. However, if threads born and die and born again,
       a previously used thread id might be already used again by a newly born thread,
       and that newly born thread will use the log state data of a previously died
       thread instead of clean data.
       
       Workaround:
       Threads should manually invoke RestoreDefaultColors() and SaveColors() after they
       invoke their FIRST Initialize() during their birth.
       
       Solution:
       Reference counting should be per-thread. This also means Deinitialize() should
       summarize all reference counts before deciding to clean everything up at the end.

    B) Threads don't wait for each other to finish their current line.
       Functions that don't start a new log line after printing the log might cause wrong log output.
       Example: O(). This function is typically used when we are iteratively building up
       a log line. In between consecutive calls to this function, other threads might also
       write to log.
       
       Workaround:
       If multiple threads are allowed to log, use functions that also start a new log line
       after printing the log e.g. OLn(). Don't use O() and similar functions when multiple
       threads are expected to log.
       
       Possible Solution:
       With lock_guard and unique_lock, the mutex should be owned until the owner finally ends the
       current log line. Currently only lock_guard is used in all public interfacing functions, but
       this should be changed to unique_lock in those functions that doesn't end current log line.
       So when O() is invoked, unique_lock should be used which won't unlock at the end of function,
       so the lock (the mutex) is kept even when the functions ends. Ending the current line in
       another call should always unlock this lock / release the mutex.
       To achieve this, mainMutex should be recursive, otherwise the same thread won't be able to
       invoke any consecutive public logging function.
       Whenever a new line is added (the current line is ended), the lock holding this mutex should be
       unlocked as many times as needed to be finally unlocked (because the lock with recursive mutex
       should be unlocked as many times as it was previously locked to finally release the mutex).
       
       Drawbacks:
       Other threads will have to wait until a new line is started (when the thread owning the mutex
       finally ends the current line). This means that some logs from other threads will be delayed,
       although they would log something earlier. Buffering (queueing) would solve this issue too,
       but currently it would take some more time to implement.
       
       Decision:
       I decided not to fix this because even if I fix this issue, still there is another legacy issue
       even in a single-threaded scenario which makes use of O() and similar functions a bit annoying.
       This is explained in known issue C).

    C) Setting error mode for only a part of a log line can make error-only logging look weird and useless.
       For example, if you log something with O() and then process something (e.g. load a bitmap), and
       then you would finish the line with either error mode or success mode based on the result of the
       processing (e.g. bitmap load failed or succeeded), then the error mode will be applied to that
       part of the log line only where you enabled the error mode. If normal logging is disabled, and only
       errors are enabled, this would lead to printing only that part of the log line, without the
       earlier part of the line. This looks bad as you dont really know what really failed.
       
       Workaround:
       Use functions that also start a new log line after printing the log, e.g. EOLn(), so the whole line
       will be logged in error mode. Don't use O() and similar functions.
       
       Possible Solution:
       In case of error, the previous few logged lines should be printed to log even if they were filtered
       out by default, so the log reader has a clue what really went wrong. To accomplish this, we need to
       maintain a fifo buffer with the most recent log lines, and being able to log it on-demand.
       Note that this could bring in another issue:
       usually if something fails and logs error, something else depending on the previous result will
       also fail and log error. In such case, it can happen that the fifo buffer is printed multiple times,
       whenever an error is logged. This also doesn't look good. To overcome this, we should delay the fifo
       buffer printout until the first next non-error printout.
       Also I think that checking how the indentation changes after an error is logged is also helpful because
       usually if something fails, then the indentation is expected to be decreased and some other error
       might be also logged, and whenever the indentation increases we most probably left that code area
       which handled the failure so we can print out the fifo buffer.  

    D) Non-breakable spaces are not always put into html log file.
       Consider the following example:
       OLn("apple     banana");
       The number of spaces will be correct in the console window, but only 1 space will be visible in the
       html log file.

       Workaround:
       O("apple");
       O("     ");
       OLn("banana");

       Possible Solution:
       I've just quickly checked and saw CConsoleImpl::WriteText() puts nbsp characters only when the whole
       given string consists of space characters only. I think this should be changed: anywhere in the string
       where we find more than 1 spaces next to each other, nbsp chars should be inserted into the html file.
       Also it is worth noting that if the string is a formatter string (contains %), then the big switch-case
       of WriteFormattedTextEx() will be executed instead of WriteText(), and that switch case walks the string
       char-by-char. Similar logic should be inserted there as well: if we are encountering a space char right
       after another space char, we should write nbsp to the html file instead of regular space char.
*/

class CConsole
{
public:

    enum FormatSignal
    {
        NL, /* new line */
        S,  /* success mode */
        E,  /* error error */
        N   /* normal mode */
    };

    // ---------------------------------------------------------------------------

    static CConsole& getConsoleInstance(const char* loggerModuleName = "");   /**< Gets the singleton instance. */

    // ---------------------------------------------------------------------------

    bool getLoggingState(const char* loggerModuleName) const;        /**< Gets logging state for the given logger module. */
    void SetLoggingState(const char* loggerModuleName, bool state);  /**< Sets logging on or off for the given logger module. */
    void SetErrorsAlwaysOn(bool state);                              /**< Sets errors always appear irrespective of logging state of current logger module. */

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
    void ResetErrorOutsCount();         /**< Resets total count of printouts-with-newline during error-mode. */
    void ResetSuccessOutsCount();       /**< Resets total count of printouts-with-newline during success-mode. */

    CConsole& operator<<(const char* text);  /**< O("%s", text). */
    CConsole& operator<<(const bool& b);     /**< O("%b", b). */
    CConsole& operator<<(const int& n);      /**< O("%d", n). */
    CConsole& operator<<(const float& f);    /**< O("%f", f). */
    CConsole& operator<<(
        const CConsole::FormatSignal& fs);   /**< Changes current mode or adds a new line. */

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


