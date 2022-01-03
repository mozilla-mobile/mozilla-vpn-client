
## Mozilla VPN Crash Reporting - What is in a Crash Report?

In the event that Mozilla VPN exits unexpectedly or crashes, you may be prompted to upload a "Crash Report."  What does that mean, and what does it mean for your privacy?

A crash report contains detailed information about the state of a process at the time it unexpectedly quits.  This includes primarily stack traces, the contents of the heap memory and the content of registers.  What are those?

**Stack Traces** - Stack traces are the list of functions that were called up to the point the application has crashed.  You can think of it as a trail of breadcrumbs through the code of the program.  This helps us determine exactly what was happening when the application quit.

**Heap Memory** - Heap memory can contain anything the program has allocated dynamically while it is running.  It is filled with bits of text or numbers or other information the program needs to store while it is running in order to work.  This lets us know what state the application was in when it exited.

**Registers** - Registers are much smaller bits of memory that are loaded as the program is executing code.  This also help us determine the state of the program.

### Can my personal information be in a crash report?

The VPN crash report could contain information about the topology of your network.  This would be things like the addresses of routers, DNS servers, or the VPN endpoint you are connected to.  It could also contain your Mozilla username and email address.  Your password is not stored in memory.  The rest of the memory contents could be anything the application has stored at the time of the crash.

Because of how Mozilla VPN is written, no internet traffic or browsing history will be in the report.

### Where does this crash report go?  Who can see it?

The crash report is uploaded to a Mozilla server.  Once it is uploaded, only people who have special access can see the crash reports.  This access is tightly controlled internally, and is not generally available to everyone at Mozilla.
How long do you keep the report?

The report is kept for 6 months before it is expired and deleted.
  
As always you can refer to the overall [Mozilla privacy](https://www.mozilla.org/en-US/privacy/) policy and use the link at the bottom to contact us for any issues or concerns.
