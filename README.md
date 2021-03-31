# Console
Console class for logging purpose on Windows and hopefully later on Linux.  
I wouldn't write this from scratch today, but back in 2011 I wanted to write a logger class that has function with variable number of parameter count, so I made this.  
Today I would rather just use a nice open-source product maintained by someone else. But since I wrote this, I'm still using it!

**Features**
 - logging to console window (currently only on Windows);
 - logging to html file, with same indentations and colors as to console window;
 - per-module log filtering: you can decide what a module is (e.g. a class) and add per-module (e.g. per-class) usage of CConsole::SetLoggingState() with your module string to turn on/off logging.

**Missing Features**
 - no log file rotation implemented, the last logging session overwrites the log file;
 - no log forwarding to external server;
 - no actions to be executed when a specific log appears.

Currently I'm not planning to implement the missing features.

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
