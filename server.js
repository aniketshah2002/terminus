// server.js
// This version uses a dynamic port for deployment.

const express = require('express');
const http = require('http');
const path = require('path');
const { WebSocketServer } = require('ws');
const pty = require('node-pty');

const app = express();
const server = http.createServer(app);
const wss = new WebSocketServer({ server });

// Use the port provided by the hosting service, or 3000 for local development.
const PORT = process.env.PORT || 3000;

app.use(express.static(path.join(__dirname, 'public')));

wss.on('connection', (ws) => {
    console.log('Client connected via WebSocket');

    const shellCommand = path.join(__dirname, 'shell');
    const shellPty = pty.spawn(shellCommand, [], {
        name: 'xterm-color',
        cols: 80,
        rows: 30,
        cwd: __dirname,
        env: process.env
    });

    shellPty.onData(data => {
        ws.send(data);
    });

    ws.on('message', (command) => {
        shellPty.write(command.toString());
    });

    ws.on('close', () => {
        console.log('Client disconnected');
        shellPty.kill();
    });

    shellPty.on('exit', (code, signal) => {
        console.log(`PTY process exited with code ${code}, signal ${signal}`);
        ws.close();
    });
});

server.listen(PORT, () => {
    console.log(`Server is running on port ${PORT}`);
});
