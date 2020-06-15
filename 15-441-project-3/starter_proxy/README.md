# Starter Proxy Code

This directory contains some code to help you get started
with 15-441/641 Project 3.

## How To Run

  1. Run `echo_server.py`. By default it listens on (127.0.0.1, 10000) - please note that this requires python 3 or above
  2. Run `make` in the root directory
  3. Run `./proxy` by default it listens on (127.0.0.1, 8888)
  4. Run `telnet localhost 8888` to test with telnet

## File Description

  - `proxy.c`: Contains the major network and multiplexing related code
  - `httpparser.c`: Contains some http parsing and header value extraction code
  - `customsocket.c` Contains some helper functions for creating sockets