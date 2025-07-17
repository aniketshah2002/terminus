**Project Terminus: A Full-Stack Command-Line Shell**
A custom command-line interpreter written in C, accessible through a modern, interactive web interface built with Node.js and WebSockets.

Animated Demo

![Project Terminus Demo](assets/demo.gif)

**About The Project**

Project Terminus is a deep dive into core operating system concepts and full-stack web development. It demonstrates a comprehensive understanding of process management, inter-process communication, and real-time web technologies. The backend is a powerful shell written from scratch in C, while the frontend is a sleek, responsive terminal emulator.

**Features**

Shell Core (C):

✅ Command Execution: Uses fork() and execvp() to run standard system commands.

✅ Pipes (|): Chains multiple commands together, redirecting stdout to stdin.

✅ I/O Redirection (<, >): Redirects input from and output to files.

✅ Logical Operators (&&, ||): Executes commands conditionally based on the success or failure of the previous command.

✅ Built-in Commands: Includes cd, exit, alias, and unalias handled directly by the shell.

✅ Background Jobs (&): Runs processes in the background without blocking the shell.

✅ Signal Handling: Gracefully handles SIGINT (Ctrl+C) without terminating the main shell.

Web Interface (Node.js & JavaScript):

✅ Live Interactive Terminal: Built with xterm.js for a realistic terminal experience.

✅ Real-time Communication: Uses WebSockets for a persistent, low-latency connection.

✅ Pseudo-Terminal (PTY): The node-pty library ensures a robust, stable connection to the underlying C shell.

✅ Modern UI: A responsive, visually appealing interface with a futuristic theme.

Tech Stack

**_Backend:_** C, Node.js, Express.js

**Real-time:**__ WebSockets (ws library)

Terminal Emulation: xterm.js

Process Management: node-pty

Deployment: Render (or your chosen platform)

**How to Run Locally**__

Clone the repository:

git clone https://github.com/your-username/your-repo-name.git
cd your-repo-name


Compile the C shell:

gcc shell.c -o shell


Install Node.js dependencies:

npm install


Start the server:

npm start


Open your browser and navigate to http://localhost:3000.

