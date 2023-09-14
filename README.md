# Globed

Globed is a multiplayer mod for Geometry Dash, powered by [Geode](https://geode-sdk.org/)

## Architecture

The server consists of two parts: a single central server and many game servers. The central server has two endpoints: `/version` (which simply returns the version in Cargo.toml), and `/servers` (which sends a JSON file with a list of all game servers, [see below](#central-server-configuration)).

Upon connection, the client tries to send a request to `/version` and compare if the versions are compatible. Then it fetches all the game servers and sends pings to them every 5 seconds, to show three things: whether the server is online, how many players it has, and what the latency is to that server.

The player can pick the server they want and press "Join", which would initiate the connection with the game server (which is where the magic happens). If you want to learn more about the game server without reading the code, you can peek into [this document](server/game/protocol.md).

## Central server configuration

* `GLOBED_SERVER_FILE_PATH` - must be passed, the server reads game servers from this file. see the format below.
* `GLOBED_ADDRESS` - default `0.0.0.0`, bind address for the HTTP server
* `GLOBED_PORT` - default `41000`, bind port for the HTTP server
* `GLOBED_MOUNT_POINT` - default `/`, mount point for the server. if set to `/globed/` it will mean all requests must be made to `http://your.server/globed/...`
* `GLOBED_LOG_LEVEL` - default `trace` in debug builds, `info` in release builds. Can be one of: `off`, `trace`, `debug`, `info`, `warn`, `error`. Indicates the minimum log priority, anything less important than the specified level will not be logged. For logs made by any other crate than the server itself, the minimum log level is always `warn` (unless you set this environment variable to `error` or `off`, that will impact those logs too)

The JSON file passed in `GLOBED_SERVER_FILE_PATH` should have a format like this:

```json5
[
    {
        "name": "Server name",
        "id": "server-id", // can be any string, preferrably a unique kebab-case string
        "region": "Europe", // can be any string, if you want you can put a specific country
        "address": "127.0.0.1:8080" // port is assumed 41001 on the client if unspecified
    },
    // ...
]
```

## Game server configuration

* `GLOBED_GS_ADDRESS` - default `0.0.0.0`, bind address for the Globed game server
* `GLOBED_GS_PORT` - default `41001`, bind port for the Globed game server
* `GLOBED_GS_TPS` - default `30`, dictates how many times per second the server sends data to the clients
* `GLOBED_GS_LOG_LEVEL` - same as in central server

## Issues & PRs

If you find any issues or potential improvements to the mod or the server, feel free to make an issue or a pull request. If you are trying to report a fatal error, please make sure to include the logs.

Additionally, if you have other requests (like if you want to host a server), feel free to contact me on discord @dank_meme01