# Audioserver OSC control interface

By design, the main communication means with the audio server is by
OpenSoundControl (OSC) messages. The primary application is to use
wireless control interfaces running a web browser that are served
their GUI as a web application from the audio server's embedded http
server. OSC communication will take place then using the WebSocket
protocol on the HTTP server's `/ws` resource by encapsulating OSC
messages in WebSocket frames (as done by, e.g., the
[OSC.js](https://github.com/colinbdclark/osc.js) library).

Additionally, the audio server may be directly controlled with OSC
using "traditional" UDP transport.

## Audio server network ports

Default listening ports of the audio server:

| Port | Protocol | Remark                                                              |
|------|----------|---------------------------------------------------------------------|
| 7080 | TCP/HTTP | Webserver for remote GUI and OSC via WebSocket (`/ws` resource)     |
| 7081 | UDP/OSC  | Primary OSC server (UDP)                                            |
| 7081 | TCP/OSC  | TCP socket bound for implementation reasons but not actually served |

Configurations may define sub-servers listening on additional ports,
usually accessing a sub-tree of the global OSC address space for
application-specific communication.

## OSC message kinds and conventions

For practical reasons, the audio server's osc address space mixes
different semantics of OSC messages, depending on the respective OSC
address node. Namely, the main difference is between nodes
representing *state* (`STATE`) and those representing a *command* (`CMD`) (*REST*
vs. *RPC* according to
[Best Practices for Open Sound Control](https://parlab.eecs.berkeley.edu/sites/all/parlab/files/osc-best-practices.pdf)).
Additionally, there are OSC addresses only used in *responses* by the
server (`RESP`) and messages transparently *passed* to other OSC clients (e.g.,
managed apps, `PASS`).

### `RESP`: Response messages

`RESP` messages are sent from the audio server to the client, usually
upon a `STATE` request or a `CMD`.

### `STATE`: State messages

`STATE` messages represent an audio server state, which might be
read-only (default) or read/write (`STATE_RW`).

* `STATE` updates on the server will be sent to all OSC clients (e.g.,
  connected WebGUI instances, app clients or built-in clients covering
  the respective OSC address subtree) by means of an OSC message to
  the respective address with arguments reflecting the current state.
* A state value may be queried by sending an OSC message to the
  respective address without arguments. The server will respond with a
  `RESP` message to the querying client with the same OSC address and
  arguments reflecting the current value of the queried state.
* `STATE_RW` states may be directly altered by sending an OSC message to the
  audio server with the respective address and corresponding
  arguments. The resulting state update(s) will be notified to all
  affected OSC clients, except of the client that issued the status
  update (no echo in the current implementation).

### `CMD`: Command messages

`CMD` messages issue an action on the audio server by sending an OSC
message to the respective address with appropriate arguments,
depending on the command.

* `CMD` nodes do not represent an audio server state, so they cannot
  be queried.
* Issuing a `CMD` does not cause a reply or acknowledgment message
  sent by the server.
* Issuing a `CMD` command might have side-effects on the server state,
  which are notified as usual to all clients (including the one that
  issued the command).
* On failure, a `CMD` message may cause an error `RESP` message to be
  sent back to the issuing client.

### `PASS`: Pass-through messages

`PASS` messages serve as a transparent OSC communication hub for an
entire OSC address subtree to arbitrary clients with no state
representation nor direct effects on the audio server, e.g., a
third-party app managed by the audio server such as *Reaper.*

Usually, a `PASS` OSC address subtree is associated with a dedicated
communication transport, such as a separate UDP listening port that is
only used by the associated third-party app.

* All OSC messages sent to an address below the specified `PASS`
  subtree are transparently forwarded to all affected clients, except
  of the sending client (no echo).
* WebGUI instances receive `PASS` messages sent to any of those
  subtrees implicitly, as they receive traffic for the entire OSC
  address space.
* Additional clients (e.g., app clients) receive traffic to the `PASS`
  address subtree they were configured for, with the subtree component
  stripped from the OSC address, and the configured app prefix (if
  any) prepended to the resulting OSC address.
* OSC messages sent by additional clients (e.g., app clients) are
  filtered according to their configured app prefix (if any), the
  configured prefix (if any) is stripped from the incoming OSC address
  and the configured subtree is prepended to the resulting OSC
  address. This results in the client’s OSC traffic appearing under
  the configured `PASS` subtree in the global OSC address space,
  potentially filtered/stripped by the configured client prefix.

## OSC wildcard matching

Generally, OSC wildcard matching is employed according to the
[OSC specification](https://opensoundcontrol.stanford.edu/spec-1_0.html#osc-message-dispatching-and-pattern-matching) for the entire OSC address space managed by the audio server.

Example to stop all currently running apps:

```
/app/*/control quit
```

### Exceptions of wildcard matching in the current implementation

> [!NOTE]
> The `/config/<id>/...` subtree currently does not support wildcard matching.

## OSC address space reference

### General

| Kind   | Address  | Types   | Description                                                                                                                                                                |
|--------|----------|---------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `CMD`  | `/ping`  | `...`   | Will reply with `/pong` to the querying client, any arguments of the /ping message will be sent back                                                                       |
| `RESP` | `/ping`  | `...`   | Response to a `/ping` message with a copy of its arguments                                                                                                                 |
| `RESP` | `/error` | `is...` | Response from the server if an error occurred, with arguments error code (`i`), error description (`s`) and further error-specific additional arguments. |

### *Config* management

| Kind    | Address                | Types  | Description                                                                         |
|---------|------------------------|--------|-------------------------------------------------------------------------------------|
| `STATE` | `/configmgr/num`       | `i`    | Number of available (i.e., currently loaded) configs                                |
| `STATE` | `/configmgr/list`      | `s...` | List of ids of available (i.e., currently loaded) configs                           |
| `CMD`   | `/configmgr/control`   | `s`    | `rescan`: Trigger (re-)loading all files in config dir (including newly added ones) |
| `CMD`   | `/config/<id>/control` | `s`    | `reload`: Trigger reloading the configuration from its config file                  |

### *App* management

| Kind    | Address                   | Types | Description                                                                                                                           |
|---------|---------------------------|-------|---------------------------------------------------------------------------------------------------------------------------------------|
| `STATE` | `/app/<id>/state`         | `is`  | Current app state with arguments state code (`i`) and state description (`s`)                                                         |
| `STATE` | `/app/<id>/exitcode`      | `i`   | Exit code of the app in case it stopped                                                                                               |
| `CMD`   | `/app/<id>/control`       | `s`   | `launch`: Trigger *startCmd* of a configured app<br>`quit`: Trigger *stopCmd* of a configured app<br>`kill`: Unconditionally kill the app |
| `PASS`  | `/app/<id>/<subtree>/...` | `...` | Transparent pass-through example, `<subtree>` depending on configuration.                                                             |

> [!NOTE]
> `/app/<id>/control kill` will kill the process that was directly
> started by the audio server. If that is, e.g., a command interpreter
> (shell) that started further processes (via a script), the command
> interpreter will be killed but not necessarily the additional
> processes.

### *Prodigy matrix* control

See [OSC_Prodigy_Audiomatrix](OSC_Prodigy_Audiomatrix.md) for details on using the `/matrix/...` subtree (matrix state).

| Kind       | Address              | Types | Description                                                                                                                      |
|------------|----------------------|-------|----------------------------------------------------------------------------------------------------------------------------------|
| `STATE`    | `/matrixmgr/state`   | `is`  | Current matrix connection state with arguments state code (`i`) and state description (`s`)                                      |
| `CMD`      | `/matrixmgr/control` | `s`   | `connect`: Manually attempt to connect audioserver to matrix<br>`disconnect`: Manually disconnect audioserver from matrix        |
| `CMD`      | `/matrixcmd/...`     | `...` | Issue a CMD message to the matrix, optionally with the given payload argument.                                                   |
| `STATE_RW` | `/matrix/...`        | `...` | Subtree for controlling/querying the Prodigy MP audio matrix state tree, laid out according to the Prodigy JSON state hierarchy. |

### *Totalmix* control

See [OSC_TotalMix](OSC_TotalMix.md) for details on using the `/totalmix/...` subtree.

| Kind   | Address         | Types | Description                                                    |
|--------|-----------------|-------|----------------------------------------------------------------|
| `PASS` | `/totalmix/...` | `...` | Transparent pass-through subtree for controlling RME TotalMix. |
