# Syslog File Converter (syslog_fc)

[![Build Status](https://img.shields.io/travis/namedun/syslog_fc/master.svg)](https://travis-ci.org/namedun/syslog_fc) [![Codacy Badge](https://api.codacy.com/project/badge/Grade/f6090a0a38164300a7f95b709b704bd4)](https://www.codacy.com/app/namedun/syslog_fc?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=namedun/syslog_fc&amp;utm_campaign=Badge_Grade)

This utility is designed to convert syslog files to various other text formats, such as CSV, HTML or JSON (full list of the supported output formats are listed below in the "[Supported Output Formats](#supported-output-formats)" section).

## Build and Install

Run CMake (notice the dot at the end):

```shell
$ cmake .
```

Run Make:

```shell
$ make
```

This will produce binary `syslogfc` in the current directory. To install this binary to `/usr/bin` run command as `root` user:

```shell
# make install
```

## Usage

Usage syntax:
```shell
syslogfc [options] <input-file>
```

*   `[options]` is a one or more additional optional options that are described in the "[Options](#options)" section.
*   `<input-file>` is the path to the syslog file to be converted. Input syslog file path should not be specified if selected the input from standard input (stdin) using additional option `--stdin` (see the "[Options](#options)" section).

### Options

#### `-h`, `--help`

Show help and usage text.

#### `-s`, `--stdin`

Use standard input (stdin) for input data. When this option is specified, no `<input-file>` is required.

#### `-f <format>`, `--format=<format>`

Output format selection. Available output formats listed int the "[Supported Output Formats](#supported-output-formats)" section.

Default: `plain`.

#### `-e <spec>`, `--entry-spec=<spec>`

Syslog entry fields specification. This specification determines the order and the presence of certain message fields in the system log files.

Available syslog entry specificators are listed below:

| Specificator | Description                                                                                                                                                                                                                              |
| ------------ | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `%I`         | Numeric identifier                                                                                                                                                                                                                       |
| `%T`         | Timestamp. For parsing this field used timestamp parsing format specification, which can be specified by option `--ts-parse-spec`.                                                                                                       |
| `%K`         | Kernel time.                                                                                                                                                                                                                             |
| `%H`         | Hostname.                                                                                                                                                                                                                                |
| `%F`         | Facility. Valid facilities are `auth`, `authpriv`, `cron`, `daemon`, `ftp`, `kern`, `lpr`, `mail`, `mark`, `news`, `security`, `syslog`, `user`, `uucp`, `local0`, `local1`, `local2`, `local3`, `local4`, `local5`, `local6`, `local7`. |
| `%P`         | Priority. Valid priorities are `alert`, `crit`, `debug`, `emerg`, `err`, `error`, `info`, `none`, `notice`, `panic`, `warn`, `warning`. Numerical values are also accepted.                                                              |
| `%G`         | Tag.                                                                                                                                                                                                                                     |
| `%M`         | Message.                                                                                                                                                                                                                                 |

One or more special modifiers can be specified between the `%` character and the specifier character, for example `%_@P`. The modifiers allows you to control the process of parsing or displaying (output) the values of the corresponding fields. For example, with the modifiers you can prevent the trimming spaces for a specific field, or disable value validation for it.

Available modifiers:

| Modifier | Description                                                                         |
| -------- | ----------------------------------------------------------------------------------- |
| `_`      | Do not trim spaces at the beginning. No effect for timestamp and identifier fields. |
| `@`      | Do not validate parsed value.                                                       |
| `!`      | Do not use this field in the output.                                                |

Default entry fields specification is `%T %F.%P %G: %_M`, which corresponds to the syslog file entries in the form:
```plain
<Timestamp> <Facility>.<Priority> <Tag>: <Message>
```

For example:
```plain
Mon Jun 24 18:12:50 2019 kern.info kernel: br-lan: port 1(sw1p1) entered blocking state
└┬─────────────────────┘ └┬─┘ └┬─┘ └┬───┘  └┬─────────────────────────────────────────┘
 │                        │    │    │       │
 │                        │    │    │       └──> Message
 │                        │    │    └──────────> Tag
 │                        │    └───────────────> Priority
 │                        └────────────────────> Facility
 └─────────────────────────────────────────────> Timestamp
```

#### `-p <spec>`, `--ts-parse-spec=<spec>`

Timestamp parsing format specification.

The format specification `<spec>` is a string and may contain special character sequences called conversion specifications, each of which is introduced by a `%` character and terminated by some other character known as a conversion specifier character. All other character sequences are ordinary character sequences.

All available conversion specifier characters are listed at the `strptime()` function manual page (<https://linux.die.net/man/3/strptime>).

Note, that if the parameter value contains spaces, it must be enclosed in quotes. For example:
```shell
$ syslogfc --ts-parse-spec="%b %d %H:%M:%S" /path/to/syslog.log
```

Default: `%a %b %d %H:%M:%S %Y`.

#### `-o <spec>`, `--ts-output-spec=<spec>`

Timestamp output format specification.

The format specification `<spec>` is a string and may contain special character sequences called conversion specifications, each of which is introduced by a `%` character and terminated by some other character known as a conversion specifier character. All other character sequences are ordinary character sequences.

All available conversion specifier characters are listed at the `strftime()` function manual page (<https://linux.die.net/man/3/strftime>).

Note, that if the parameter value contains spaces, it must be enclosed in quotes. For example:
```shell
$ syslogfc --ts-output-spec="%d.%m.%Y %H:%M:%S" /path/to/syslog.log
```

Default format for output is not specified. If the output format is not specified or specified as empty string, then the time will be output as UNIX timestamp.

#### `-d <delimieter>`, `--csv-delimeter=<delimeter>`

Delimeter for `csv` (CSV) output format.

Default: `,` (comma).

#### `-x <prefix>`, `--html-class-prefix=<prefix>`

Prefix for HTML classes for `html` output format.

Default: `syslog-`.

#### `-c <on|off>`, `--html-cell-classes=<on|off>`

Output HTML classes for each table cell for `html` output format.

Default: `off`.

## Supported Output Formats

| Format     | Description                            |
| ---------- | -------------------------------------- |
| `asciidoc` | AsciiDoc                               |
| `csv`      | CSV (Comma-Separated Values)           |
| `html`     | HTML (HyperText Markup Language) table |
| `json`     | JSON (JavaScript Object Notation)      |
| `md`       | Markdown table                         |
| `plain`    | Plain text format (for testing)        |

## Examples

### JSON

Input syslog file `example1.log`:

```plain
Mon Jun 24 18:12:50 2019 kern.info kernel: br-lan: port 1(sw1p1) entered blocking state
Mon Jun 24 18:12:50 2019 daemon.info mstpd[1882]: set_if_up: Port "sw1p1" : up
Mon Jun 24 18:12:50 2019 daemon.notice netifd: bridge "br-lan" link is up
Mon Jun 24 18:13:07 2019 daemon.err modprobe: xt_connmark is already loaded
```

Convert to JSON:

```shell
$ syslogfc --format=json example1.log
[{"timestamp":"1561389170","facility":"kern","priority":"info","tag":"kernel","message":"br-lan: port 1(sw1p1) entered blocking state"},{"timestamp":"1561389170","facility":"daemon","priority":"info","tag":"mstpd[1882]","message":"set_if_up: Port \"sw1p1\" : up"},{"timestamp":"1561389170","facility":"daemon","priority":"notice","tag":"netifd","message":"bridge \"br-lan\" link is up"},{"timestamp":"1561389187","facility":"daemon","priority":"err","tag":"modprobe","message":"xt_connmark is already loaded"}]
```

If you need formatted JSON output, you can use any external JSON formatting tool. For example:

```shell
$ syslogfc --format=json example1.log | python3 -m json.tool
[
    {
        "timestamp": "1561389170",
        "facility": "kern",
        "priority": "info",
        "tag": "kernel",
        "message": "br-lan: port 1(sw1p1) entered blocking state"
    },
    {
        "timestamp": "1561389170",
        "facility": "daemon",
        "priority": "info",
        "tag": "mstpd[1882]",
        "message": "set_if_up: Port \"sw1p1\" : up"
    },
    {
        "timestamp": "1561389170",
        "facility": "daemon",
        "priority": "notice",
        "tag": "netifd",
        "message": "bridge \"br-lan\" link is up"
    },
    {
        "timestamp": "1561389187",
        "facility": "daemon",
        "priority": "err",
        "tag": "modprobe",
        "message": "xt_connmark is already loaded"
    }
]
```

### CSV

Input syslog file `example2.log`:

```plain
24.06.2019 18:12:50 kern.info kernel: br-lan: port 1(sw1p1) entered blocking state
24.06.2019 18:12:50 daemon.info mstpd[1882]: set_if_up: Port "sw1p1" : up
24.06.2019 18:12:50 daemon.notice netifd: bridge "br-lan" link is up
24.06.2019 18:13:07 daemon.err modprobe: xt_connmark is already loaded
```

Note that the timestamp format is different from the previous example.

If we try to convert the file using the default timestamp parsing format, we get parsing errors:
```shell
$ syslogfc --format=csv example2.log
1: Failed to parse timestamp (-84)
2: Failed to parse timestamp (-84)
3: Failed to parse timestamp (-84)
4: Failed to parse timestamp (-84)
```

For proper conversion, you must specify the correct format:
```shell
$ syslogfc -p "%d.%m.%Y %H:%M:%S" --format=csv example2.log
"1561389170","kern","info","kernel","br-lan: port 1(sw1p1) entered blocking state"
"1561389170","daemon","info","mstpd[1882]","set_if_up: Port ""sw1p1"" : up"
"1561389170","daemon","notice","netifd","bridge ""br-lan"" link is up"
"1561389187","daemon","err","modprobe","xt_connmark is already loaded"
```

For example, let's also change the output time format:
```shell
$ syslogfc -p "%d.%m.%Y %H:%M:%S" -o "%A %d %B %Y" --format=csv example2.log
"Monday 24 June 2019","kern","info","kernel","br-lan: port 1(sw1p1) entered blocking state"
"Monday 24 June 2019","daemon","info","mstpd[1882]","set_if_up: Port ""sw1p1"" : up"
"Monday 24 June 2019","daemon","notice","netifd","bridge ""br-lan"" link is up"
"Monday 24 June 2019","daemon","err","modprobe","xt_connmark is already loaded"
```

## Changelog

See [CHANGELOG.md](CHANGELOG.md).

## License

This work is free. You can redistribute it and/or modify it under the
terms of the Do What The Fuck You Want To Public License, Version 2,
as published by Sam Hocevar. See <http://www.wtfpl.net/> for more details.

## Authors

*   Anton Kikin <mailto:a.kikin@tano-systems.com>
