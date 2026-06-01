# valkey-lite

An in-memory key-value store with a TCP server interface compatible with [Valkey](https://valkey.io/) / Redis clients. Connect with `nc` or `telnet`.

```sh
./valkey              # start on default port 6379
./valkey --maxmemory 64mb
```

```sh
$ redis-cli
127.0.0.1:6379> SET city Helsinki
OK
127.0.0.1:6379> GET city
"Helsinki"
127.0.0.1:6379> GEOADD locations 37.6176 55.7556 moscow
(integer) 1
127.0.0.1:6379> GEOADD locations 16.3738 48.2081 vienna
(integer) 1
127.0.0.1:6379> GEODIST locations moscow vienna km
"1670.xxxxxx"
```

## Server

The TCP server listens on port 6379 by default. Each client connects, sends commands line by line, and receives responses until it disconnects or sends `EXIT` (case-insensetive).

```
socket() → bind() → listen() → accept() → recv/send loop → close()
```

The server is single-threaded — one client is served at a time. The `Server` class owns the socket descriptor and closes it in its destructor.

## Supported data types

| Type | Underlying structure |
|---|---|
| String | `std::string` |
| List | `std::list<std::string>` |
| Set | `std::unordered_set<std::string>` |
| Geospatial index | `std::unordered_map<std::string, GeoPoint>` |

All four types are stored in a single `std::variant` per key. Type mismatches return `WRONGTYPE` errors without touching the store.

## Commands

**String** — `SET`, `GET`, `STRLEN`, `APPEND`, `EXPIRE`, `TTL`

**List** — `LPUSH`, `RPUSH`, `LPOP`, `RPOP`, `LLEN`, `LRANGE`, `LINDEX`, `LSET`, `LINSERT`

**Set** — `SADD`, `SREM`, `SISMEMBER`, `SMEMBERS`, `SCARD`, `SUNION`, `SINTER`, `SDIFF`, `SMOVE`

**Geo** — `GEOADD`, `GEOPOS`, `GEODIST`, `GEOSEARCH`, `GEOSEARCHSTORE`

**Common** — `TYPE`, `DEL`, `EXISTS`, `KEYS`, `FLUSHDB`, `DBSIZE`, `EXPIRE`, `TTL`, `CONFIG SET/GET`, `MEMORY USAGE`

Command names are case-insensitive; keys are case-sensitive.

## Memory limit

Set at startup or at runtime:

```
CONFIG SET maxmemory 128mb
CONFIG GET maxmemory
```

Once the limit is reached, any write command returns:

```
(error) OOM command not allowed when used memory > 'maxmemory'
```

Supported suffixes: `b`, `kb`, `mb`, `gb`.

## Architecture

Commands are registered in a dispatch table at construction time — each entry holds min/max argument counts and a `std::function` handler. Adding a new command is a single-line change with no branching in the execute loop.

The `Overload` pattern with `std::visit` dispatches operations over the variant without `if/else` chains. Geo distance uses the Haversine formula with Earth radius 6372.8 km.

## Build

```sh
cmake -S . -B build
cmake --build build

# run tests
ctest --test-dir build --verbose

# start the server
./build/bin/valkey
./build/bin/valkey --maxmemory 32mb
```

Requirements: C++23, CMake ≥ 3.20, Google Test

## Tech

C++23 · BSD sockets · `std::variant` · `std::visit` · Overload pattern · Haversine formula · CMake · Google Test
