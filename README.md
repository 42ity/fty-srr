# fty-srr

fty-srr is an agent who has in charge Save, Restore, and Reset the system.

## How to build

To build, run:

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=On ..
make
sudo make install
```

## How to run

To run:

* from within the build tree, run:

```bash
./fty-srr
```

For the other options available, refer to the manual page of fty-srr.

* from an installed base, using systemd, run:

```bash
systemctl start fty-srr
```

## Configuration file

Agent has a configuration file: fty-srr.cfg.

Except standard server and malamute options, there are two other options:
* server/check_interval for how often to publish Linux system metrics
* parameters/path for REST API root used by IPM Infra software

Agent reads environment variable BIOS_LOG_LEVEL, which sets verbosity level of the agent.

## CLI (internal)

```bash
  Usage: fty-srr-cmd <list|save|restore|reset> [options]

  -h, --help       Show this help
  -p, --passphrase Passhphrase to save/restore groups
  -pwd, --password Password to restore groups (reauthentication)
  -t, --token      Session token to save/restore groups if needed
  -g, --groups     Select groups to save (default to all groups)
  -f, --file       Path to the JSON file to save/restore. If not specified, standard input/output is used
  -F, --force      Force restore (discards data integrity check)
```
