# Parser and reward calculator for joined binary schema v2 files

## Binary log file format
The binary log file consists of a sequence of *messages*, with each message being one of five types. The message types are FILEMAGIC, HEADER, REGULAR, CHECKPOINT, and EOF. The binary log should begin with a FILEMAGIC message and end with an EOF message, although this is not mandatory.

Each message contains a *payload*, except for the FILEMAGIC message which has an *inline payload*. For an inline payload, the payload size field is used to store the payload data itself (which must be 4 bytes long), and the later fields are omitted.

The general format of each message is shown in this table. Details for each message type are described later.

| Size (bytes) | Description |
| ---- | ----------- |
| 4 | message type identifier |
| 4 | payload size (or inline payload data) |
| `payload_size` | payload content (required if not inline payload) |
| `payload_size % 8` | padding (required if not inline payload) |

### Byte ordering

The byte order for message type identifier and payload size must be little-endian. The payload itself can be message type defined, but since all messages are either empty or flatbuffers (which are themselves little-endian) in practice this means the payload is also little-endian.

### Message type
The first 4 bytes in each message is an unique indentifier:
- `MSG_TYPE_FILEMAGIC = 0x42465756` (in ASCII this is `VWFB`)
- `MSG_TYPE_HEADER = 0x55555555`
- `MSG_TYPE_REGULAR = 0xFFFFFFFF`
- `MSG_TYPE_CHECKPOINT = 0x11111111`
- `MSG_TYPE_EOF = 0xAAAAAAAA`

### Payload size
For all message types except the FILEMAGIC message, the payload size field should store the size of the payload as an unsigned 32-bit integer. We will refer to this value as `payload_size`.

For the FILEMAGIC message, the payload size field is used to store an inline payload containing the version of the binary log file format. As of now the only existing version is `1` (one), so the FILEMAGIC message must have its payload size field equal to one.

Note that the binary log format version is not the same as the Flatbuffer schema version, which is version 2 for all the flatbuffers described here.

### Payload content
The following `payload_size` bytes of the file are interpreted as the payload data itself.

### Padding
The file format requires `payload_size % 8` bytes of padding at the end of every payload. Note that this does not align the total message size to a multiple of 8 bytes. (For example a one byte payload gets one padding byte appended to make 2 bytes total.) It is merely a file format requirement and is necessary for the next message to be correctly parsed.

Padding bytes should not be included when computing `payload_size`. Padding bytes should have a value of zero, but this is not enforced.

### Recomended message ordering
The recomended ordering of messages in a file is shown here.

- 1 FILEMAGIC message
- 1 HEADER message
- `N` times:
    - 1 CHECKPOINT message
    - `M` REGULAR messages (to be processed with the same checkpoint information)
- 1 EOF message

Only the REGULAR message is strictly necessary. Although it is not recommended, a binary log without the other message types can still be parsed correctly.

## Message details
The following gives details about each message type. For information on flatbuffer payload schemas, see [`rlclientlib/schema/v2`](https://github.com/VowpalWabbit/reinforcement_learning/tree/master/rlclientlib/schema/v2)

### File Magic message
The FILEMAGIC message does not contain a normal payload but instead must have `payload_size = 1` as described above.

The FILEMAGIC message is optional, but it is recommended to begin every binary log file with it.

### Header message
The payload for a HEADER message is a flatbuffer of type `FileHeader`. The HEADER message is optional and does not affect how the binary log is parsed.

```
table FileHeader {
    join_time: TimeStamp;
    properties: [KeyValue];
}
```
(see [`FileFormat.fbs`](https://github.com/VowpalWabbit/reinforcement_learning/blob/master/rlclientlib/schema/v2/FileFormat.fbs))

This message contains informational data about this file. It should include provenance details such as how it was generated, the version and parameters of the program used, generation time, and other information that helps troubleshooting.

### Checkpoint message
The payload for a CHECKPOINT message is a flatbuffer of type `CheckpointInfo`.

```
table CheckpointInfo {
    reward_function_type: RewardFunctionType;
    default_reward: float;
    learning_mode_config: LearningModeType;
    problem_type_config: ProblemType;
    use_client_time: bool;
}
```
(see [`FileFormat.fbs`](https://github.com/VowpalWabbit/reinforcement_learning/blob/master/rlclientlib/schema/v2/FileFormat.fbs))

This message includes information on how to join events comming after it. If a large binary log file must be split, it is recommended to split at the start of a CHECKPOINT message as they contain all info required to process the stream that follows it.

The CHECKPOINT message is optional. If it is not present in a binary log, events will be processed with the default joiner configuration. If it is only present after encountering any REGULAR messages, all the preceding REGULAR messages will be processed with the default joiner configuration.

### Regular message
The payload for a REGULAR message is a flatbuffer of type `JoinedPayload`.

```
table JoinedPayload {
    events: [JoinedEvent];
}

table JoinedEvent {
    event: [ubyte];
    timestamp: TimeStamp;
}
```
(see [`FileFormat.fbs`](https://github.com/VowpalWabbit/reinforcement_learning/blob/master/rlclientlib/schema/v2/FileFormat.fbs))

The `JoinedPayload` flatbuffer internally contains a vector of `JoinedEvent`s. All of these `JoinedEvent`s should share the same event ID. For a binary log containing more than one event ID, multiple REGULAR messages should be used with one for each ID.

The `JoinedEvent` flatbuffer has an `event` field that contains a serialized `Event` flatbuffer. The `Event` flatbuffer contains a metadata field and a `payload` field that contains a serialized type-specific event flatbuffer (CaEvent, CbEvent, OutcomeEvent, etc.). In other words, the final event data is contained in a flatbuffer (type specific) inside a flatbuffer (Event) inside a flatbuffer (JoinedPayload) inside a custom message format (REGULAR message).

```
table Event {
    meta:Metadata;
    payload:[ubyte];
}

table Metadata {
    id:string;
    client_time_utc:TimeStamp;
    app_id:string;
    payload_type:PayloadType;
    pass_probability:float;
    encoding: EventEncoding;
}
```
(see [`Event.fbs`](https://github.com/VowpalWabbit/reinforcement_learning/blob/master/rlclientlib/schema/v2/Event.fbs) and [`Metadata.fbs`](https://github.com/VowpalWabbit/reinforcement_learning/blob/master/rlclientlib/schema/v2/Metadata.fbs))

### EOF message
The EOF (End Of File) message stops file parsing as soon as it is read. It should contain a payload size of zero and no payload, but any non-zero payload will be simply ignored as parsing will stop when the EOF message identifier is read.

The EOF message is optional, but it is recommended to end every binary log file with it. If it is not present, parsing will continue until the end of the input data is reached or an error is encountered.

# Build instructions
The binary parser may be build either along with rlclientlib or as a standalone project.

## Building with rlclientlib
To build with rlclientlib, add `-DRL_BUILD_EXTERNAL_PARSER=On` to the CMake command line when configuring rlclientlib. Compiling rlclientlib will then produce an executable at `build/external_parser/vw`.

The following instructions assume building the parser as a standalone project.

## Linux
### Build
**Note**: To statically link, add `-DSTATIC_LINK_BINARY_PARSER=ON` to the `cmake` command line

Run these commands from `external_parser` directory:
- `cmake -S . -B build`
- `cmake --build build`

The output is a `vw` executable located at: `external_parser/build/vw`

### Run
`./vw -d <file> --binary_parser [other vw args]`

## Windows
### Install dependencies
**Note**: to link statically then replace `x64-windows` with `x64-windows-static-md` during vcpkg installation and in the cmake `-DVCPKG_TARGET_TRIPLET`

**Note**: vcpkg doesn't play well with nugets in visual studio so if you are trying to build something else in visual studio that uses the below via nugets you might get linking errors

- `vcpkg install boost-filesystem:x64-windows`
- `vcpkg install boost-thread:x64-windows`
- `vcpkg install boost-program-options:x64-windows`
- `vcpkg install boost-test:x64-windows`
- `vcpkg install flatbuffers:x64-windows`

### Build
Run these commands from `external_parser` (replace `Release` directory with `Debug` if you want a debug build):
- `cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=<VCPKG_INSTALLATION_ROOT>\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows -G "Visual Studio 15 2017" -A x64 -DBUILD_FLATBUFFERS=OFF -DCMAKE_CONFIGURATION_TYPES="Release" -DWARNINGS=OFF -DWARNINGS=OFF -DWARNING_AS_ERROR=OFF -DDO_NOT_BUILD_VW_C_WRAPPER=OFF  -DBUILD_JAVA=OFF -DBUILD_PYTHON=OFF -DBUILD_TESTING=OFF -DBUILD_EXPERIMENTAL_BINDING=OFF`
- `<MSBUILD_PATH> /verbosity:normal /m /p:Configuration=Release;Platform=x64 vw_binary_parser.sln`

`vw_binary_parser.sln` will be available under `build` and can be used to open the solution in visual studio

The output is a `vw` executable located at: `external_parser\build\Release\vw.exe`

### Run
`.\build\Release\vw.exe -d <file> --binary_parser [other vw args]`
