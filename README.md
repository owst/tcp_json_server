# C TCP JSON Server and non-blocking Ruby client

This is a very simple C implementation of a forking TCP server that writes out
JSON-formatted, NULL byte delimited messages to a non-blocking Ruby client,
written mostly to learn a bit more about socket programming in C, and
non-blocking IO in Ruby.

To use, compile and run the server in one terminal with:
```
gcc -Wall server.c -o server && ./server
```
and in another terminal run the client with:
```
ruby client.rb
```

Which should produce server output similar to:
```
Server listening on port 2048
Accepted new client: 127.0.0.1:59397 with sleep 1889
Handling client 127.0.0.1:59397 in child PID 83730 of parent with PID 83716
Accepted new client: 127.0.0.1:59399 with sleep 7292
Handling client 127.0.0.1:59399 in child PID 83731 of parent with PID 83716
Written 43 to client 127.0.0.1:59397
Written 43 to client 127.0.0.1:59397
Written 43 to client 127.0.0.1:59397
Child with PID 83730 exited
Written 43 to client 127.0.0.1:59399
Written 43 to client 127.0.0.1:59399
Written 43 to client 127.0.0.1:59399
Child with PID 83731 exited
```
and client output similar to:
```
{"iteration":0,"client":"127.0.0.1:59397"}
{"iteration":1,"client":"127.0.0.1:59397"}
{"iteration":2,"client":"127.0.0.1:59397"}
Socket with source port 59397 EOF!
{"iteration":0,"client":"127.0.0.1:59399"}
{"iteration":1,"client":"127.0.0.1:59399"}
{"iteration":2,"client":"127.0.0.1:59399"}
Socket with source port 59399 EOF!
```
