# Console
Console class for logging purpose on Windows and hopefully later on Linux.  
I wouldn't write this from scratch today, but back in 2011 I wanted to write a logger class that has function with variable number of parameter count, so I made this.  
Today I would rather just use a nice open-source product maintained by someone else. But since I wrote this, I'm still using it!

**Features** of v1.2:
 - logging to console window (currently only on Windows);
 - logging to html file, with same indentations and colors as to console window;
 - per-module log filtering: you can decide what a module is (e.g. a class) and add per-module (e.g. per-class) usage of CConsole::SetLoggingState() with your module string to turn on/off logging;
 - thread-safe.

**Missing Features:**
 - turn on logging only into html file, not to console window (currently logging to html works only when console window is already present);
 - timestamping;
 - config file (that could contain e.g. module logging setting);
 - no log file rotation implemented;
 - no log forwarding to external server;
 - no actions to be executed when a specific log appears.

Currently I'm not planning to implement the missing features.

**Known Issues:**
As described in comment in [CConsole.h](https://github.com/proof88/Console/blob/master/CConsole/src/CConsole.h).

**Requires**: [PFL v1.1](https://github.com/proof88/PFL).

Note that this project is NOT unit-tested, only some showcase tests are implemented in [conmain.cpp](https://github.com/proof88/Console/blob/master/CConsole/src/conmain.cpp).  
If you want to run this, you have to go to **Project Settings** in Visual Studio, then **General Settings**, and change **Configuration Type** from **Static Library** to **Application**. Then simply Build & Run!

<p align="center">
  Example: Windows console window during demo run:<br/>
  <a href="cconsole-shot-01.PNG" target="_blank"><img src="cconsole-shot-01.PNG" width="475" height="301"></a>
</p>
  
<p align="center">
  The generated log HTML opened in web browser:<br/>
  <a href="cconsole-shot-02.PNG" target="_blank"><img src="cconsole-shot-02.PNG" width="450" height="373"></a>
</p>

The Visual Studio project file is included.<br/>
However, if you want to see example of integration in other projects, you may be interested in the Visual Studio solution including other projects as well in [PGE-misc](https://github.com/proof88/PGE-misc) repo.
