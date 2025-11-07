# KEA Interfaces Commands Hook

## Introduction

This repository provides a KEA hook that allows interaction with the network interfaces of the DHCP4 and DHCP6 servers on a global and per-subnet basis.

## Service Info
The hook is implemented for `kea-dhcp4` and `kea-dhcp6` service.

## Parameters

### Required
`dhcp4-config-path` - String type. Specifies the path to the DHCP4 server configuration file.

`dhcp6-config-path` -  String type. Specifies the path to the DHCP6 server configuration file.

## Example configuration
```json
{
  "dhcp4-config-path":  "/etc/kea/kea-dhcp4.conf",
  "dhcp6-config-path":  "/etc/kea/kea-dhcp6.conf"
}
```

## API Parameters
`section`  - String type (enum: "global" | "subnet"). Determines from which configuration section network interfaces should be managed.
`subnet_id` - Unsigned interger type. For requests to get, add, and delete a configuration, specifies the subnet ID for which manipulations should be performed. This only works if the `"section": "subnet"` parameter is specified.
`interfaces` - List of strings type. For configuration add and remove requests, specifies the name of the network interface to be manipulated. 

## API Commands
Interaction with the hook is carried out similarly to the hooks "class_cmds", "host_cmds",  "subnet_cmds" and others.

### `available-interfaces` command
Takes zero parameters.
Returns a list of network interfaces, both in conjunction with the IP and individually (if activated in "kea-dhcp4," then only v4; if in "kea-dhcp4," then only v6).

**Payload call**
```json
{
  "command": "available-interfaces",
  "service": ["dhcp4"],
  "arguments": {}
}
```
**Response**
```json
[
  {
    "arguments": {
      "interfaces": [
        "lo",
        "enp1s0",
        "enp6s0"
      ],
      "interfaces-with-ip": [
        "lo/127.0.0.1",
        "enp1s0/10.81.16.63"
      ]
    },
    "result": 0,
    "text": "3 interfaces found."
  }
]
```


### `get-interfaces` command
Takes a `section`  and `subnet_id` (optionally) parameters.
Returns a list of network interfaces specified in the server configuration.

**Payload for global call**
```json
{
  "command": "get-interfaces",
  "service": ["dhcp4"],
  "arguments": { "section": "global" }
}
```

**Response for global call**
```json
[
  {
    "arguments": {
      "interfaces": [
        "enp6s0"
      ]
    },
    "result": 0,
    "text": "in section global 1 interfaces found."
  }
]
```

**Payload for subnet call**
For subnets, either an empty list is always returned if no interface is specified in the subnet, or a single-item list if one is specified.
```json
{
  "command": "get-interfaces",
  "service": ["dhcp4"],
  "arguments": { "section": "subnet", "subnet_id": 1 }
}
```

**Response for subnet call**
```json
[
  {
    "arguments": {
      "interfaces": [
        "enp2s0"
      ]
    },
    "result": 0,
    "text": "in section subnet with id 1 interface found."
  }
]
```

### `append-interfaces` command
Takes a `section` , `interfaces` and `subnet_id` (optionally) parameters.
Adds interfaces to an existing configuration. If the interface has already been added, the request will return an error message.
**Payload for global call**
```json
{
  "command": "append-interfaces",
  "service": ["dhcp4"],
  "arguments": { "section": "global", "interfaces": ["lo"] }
}
```

**Response for global call**
```json
[
  {
    "result": 0,
    "text": "Configuration updated successfully."
  }
]
```

**Payload for global call**
```json
{
  "command": "append-interfaces",
  "service": ["dhcp4"],
  "arguments": { "section": "subnet", "subnet_id": 1, "interfaces": ["lo"] }
}
```

**Response for subnet call**
```json
[
  {
    "result": 0,
    "text": "Configuration updated successfully."
  }
]
```

### `delete-interfaces` command
Takes a `section` , `interfaces` and `subnet_id` (optionally) parameters.
Remove interfaces to an existing configuration. If the interface not exists, the request will return an error message.
**Payload for global call**
```json
{
  "command": "delete-interfaces",
  "service": ["dhcp4"],
  "arguments": { "section": "global", "interfaces": ["lo"] }
}
```

**Response for global call**
```json
[
  {
    "result": 0,
    "text": "Configuration updated successfully."
  }
]
```

**Payload for global call**
```json
{
  "command": "delete-interfaces",
  "service": ["dhcp4"],
  "arguments": { "section": "subnet", "subnet_id": 1, "interfaces": ["lo"] }
}
```

**Response for subnet call**
```json
[
  {
    "result": 0,
    "text": "Configuration updated successfully."
  }
]
```

## Build

### Build with G++
1. Install kea-dev package (In Arch it is part of the KEA package, in other distributions it may be a separate package).
2. Clone this repository `git clone https://github.com/sanua356/kea-interfaces-cmds-hook.git`.
3. Go to the repository directory `cd ./kea-interfaces-cmds-hook`.
4. Make the build script executable `chmod +x ./build.sh`.
5. Run build script `./build.sh` (sudo rights may be required).
6. Find ".so" library in current directory.

### Build with Meson
1. Clone KEA official repository: `git clone https://gitlab.isc.org/isc-projects/kea.git`.
2. Clone current repository in KEA repository path `/src/hooks/dhcp/`.
3. In file `meson.build` by path `/src/hooks/dhcp` paste new line `subdir('kea-interfaces-cmds-hook')` .
4. Run `meson compile -C build` in terminal.
5. After compilation the library file is located at the path `/build/src/hooks/dhcp/kea-interfaces-cmds-hook`.

## License

MIT
