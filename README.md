<h1>simple HTTP webserver</h1>
Simple, minimal, single-threaded and lightweight HTTP webserver written in C++
Only Linux is supported.

=====================WARNING===========================================<br>
This code probably has bugs and security vulnerabilities!
Please be careful and do not handle sensitive data with the server. 
The HTTP protocol is used so anyone can see (and modify) the transmitted data.
I am in no way responsible for damages caused by the software in this repository.

<h2>Usage</h2>

<ol type="1">
    <li>Compile with <code>g++ -o webserver webserver.cpp</code>
      Or download from releases</li>
  <li>Create a folder called <code>webserver</code> in the same folder as the executable</li>
  <li>Place all the <code>.html</code> files inside the <code>webserver</code> folder. This folder is publically accessible.
  <li>Start the server with: <code>sudo ./webserver</code> from the commandline. (or use <code>sudo ./webserver >> log.txt</code> to create a log file.)
<li>Stop the server with <kbd>crtl+C</kbd>
</ol>

