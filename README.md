# cpp-networking

##Summary
A collection of simple and asynchronous networking wrapper that I (will someday) use. This is very much work a in progress.

The only requirement is BOOST v1.60

##Example Usage
```C++
 Tcp client("www.example.com", 80);
 future = client.send("Hello, World!");
```

```C++
Http client("www.example.com");
future = client.get("/index");
```

On the face I want a very simple interface for complex concurrent / asynchoronous networking operations.

##Features
* RAII connections and disconnections
* Self managed asynchronous I/O event loop (currently running on a single thread)
* Easily customizable HTTP headers
* HTTP v1.1 style of cached and persistent connections
* TODO: Self managed I/O event loop workers that scale depending on load
* TODO: Seperate server / client classes built off of the same base code
