/*
 * Syslog File Converter
 * Copyright © 2019 Anton Kikin <a.kikin@tano-systems.com>
 *
 * This tool can convert syslog files into different text
 * formats, such as CSV, JSON or HTML.
 *
 * Default syslog file format:
 *   <timestamp> <facility>.<priority> <tag>: <message>
 *
 * Default syslog file format can be changed by option --entry-spec.
 *
 * Default <timestamp> format is "%a %b %d %H:%M:%S %Y" (strptime format).
 * Custom <timestamp> format can be specified by option --ts-parse-spec (-p).
 *
 * For more details see README.md.
 *
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file for more details.
 */

/**
 * @file
 * @brief Main source file
 *
 * @author Anton Kikin <a.kikin@tano-systems.com>
 */

#include <getopt.h>

#include <syslog_fc.h>

#include <fmt_plain.h>
#include <fmt_json.h>
#include <fmt_csv.h>
#include <fmt_md.h>
#include <fmt_html.h>
#include <fmt_asciidoc.h>

/**
 * @brief Output formats
 */
static const output_fmt_t *fmt_avail[] =
{
	&fmt_plain,
	&fmt_md,
	&fmt_csv,
	&fmt_json,
	&fmt_html,
	&fmt_asciidoc,
	NULL
};

/**
 * @brief Default configuration structure
 */
static const config_t default_config =
{
	.is_stdin          =  0,
	.output_fmt        = &fmt_plain,
	.entry_spec        = "%T %F.%P %G: %_M",
	.ts_parse_spec     = "%a %b %d %H:%M:%S %Y", /* Mon Jun 24 18:12:50 2019 */
	.ts_output_spec    = "",                     /* UNIX timestamp */
	.input_filename    =  NULL,
	.csv_delimeter     = ",",
	.html_class_prefix = "syslog-",
	.html_cell_classes =  0,
};

/**
 * @brief Global configuration structure
 */
config_t config = { 0 };

/**
 * @brief Short command line options list
 */
static const char *opts_str = "hf:e:sp:o:d:x:c:";

/**
 * @brief Long command line options list
 */
static const struct option opts[] =
{
	{ .name = "help",              .val = 'h' },
	{ .name = "format",            .val = 'f', .has_arg = 1 },
	{ .name = "stdin",             .val = 's' },
	{ .name = "entry-spec",        .val = 'e', .has_arg = 1 },
	{ .name = "ts-parse-spec",     .val = 'p', .has_arg = 1 },
	{ .name = "ts-output-spec",    .val = 'o', .has_arg = 1 },
	{ .name = "csv-delimeter",     .val = 'd', .has_arg = 1 },
	{ .name = "html-class-prefix", .val = 'x', .has_arg = 1 },
	{ .name = "html-cell-classes", .val = 'c', .has_arg = 1 },
	{ 0 }
};

/**
 * Display program usage help
 */
static void display_usage(void)
{
	int i;

	fprintf(stdout,
		"\n"
		"Syslog File Converter version " SYSLOG_FC_VERSION "\n"
		"Copyright (c) 2019 Anton Kikin <a.kikin@tano-systems.com>\n"
		"\n"
		"Usage: syslog_fc [options] <input-file>\n"
		"\n"
		"Options:\n"
		"  -h, --help\n"
		"        Show this help text.\n"
		"\n"
		"  -s, --stdin\n"
		"        Read data from stdin instead of file.\n"
		"\n"
		"  -f, --format <format>\n"
		"        Select output format.\n"
		"\n"
		"        Available output formats:\n"
	);

	/* Display available formats */
	for (i = 0; fmt_avail[i]; i++)
	{
		fprintf(stdout, "%12s%-8s - %s\n",
			"", /* left indentation */
			fmt_avail[i]->name,
			fmt_avail[i]->description
		);
	}

	fprintf(stdout,
		"\n"
		"        Default: \"%s\"\n"
		"\n"
		"  -e, --entry-spec <spec>\n"
		"        Syslog entry fields specification.\n"
		"        See README.md for details.\n"
		"\n"
		"        Allowed format specificators:\n"
		"            %%T - Timestamp\n"
		"            %%H - Hostname\n"
		"            %%F - Facility\n"
		"            %%P - Priority\n"
		"            %%G - Tag\n"
		"            %%M - Message\n"
		"\n"
		"        Default: \"%s\"\n"
		"\n"
		"  -p, --ts-parse-spec <format>\n"
		"        Timestamp format specification for parsing.\n"
		"        See 'man strptime' for available specificators description.\n"
		"\n"
		"        Default: \"%s\"\n"
		"\n"
		"  -o, --ts-output-spec <format>\n"
		"        Timestamp format specification for output.\n"
		"        Keep empty for output time as UNIX timestamp.\n"
		"        See 'man strftime' for available specificators description.\n"
		"\n"
		"        Default: \"%s\"\n"
		"\n"
		"  -d, --csv-delimeter <delimeter>\n"
		"        Specifiy delimeter for CSV output format.\n"
		"\n"
		"        Default: \"%s\"\n"
		"\n"
		"  -x, --html-class-prefix <prefix>\n"
		"        Prefix for HTML classes.\n"
		"\n"
		"        Default: \"%s\"\n"
		"\n"
		"  -c, --html-cell-classes <on|off>\n"
		"        Add HTML classes for each table cell.\n"
		"\n"
		"        Default: \"%s\"\n"
		"\n",
		default_config.output_fmt->name,
		default_config.entry_spec,
		default_config.ts_parse_spec,
		default_config.ts_output_spec ? default_config.ts_output_spec : "",
		default_config.csv_delimeter,
		default_config.html_class_prefix,
		default_config.html_cell_classes ? "on" : "off"
	);
}

/**
 * Parse command line arguments into @ref config global structure
 *
 * @param[in] argc  Number of arguments
 * @param[in] argv  Array of the pointers to the arguments
 *
 * @return 0 on success
 * @return <0 on error
 */
static int cli_args(int argc, char *argv[])
{
	int opt;

	while((opt = getopt_long(argc, argv, opts_str, opts, NULL)) != EOF)
	{
		switch(opt)
		{
			case '?':
			{
				/* Invalid option */
				return -EINVAL;
			}

			case 'h': /* --help */
			{
				display_usage();
				exit(0);
			}

			case 's': /* --stdin */
			{
				config.is_stdin = 1;
				break;
			}

			case 'f': /* --format */
			{
				int i;
				const output_fmt_t *new_output_fmt = NULL;

				for (i = 0; fmt_avail[i]; i++)
				{
					if (!strcmp(optarg, fmt_avail[i]->name))
					{
						new_output_fmt = fmt_avail[i];
						break;
					}
				}

				if (!new_output_fmt)
				{
					fprintf(stderr, "%s: invalid format '%s'\n",
						argv[0], optarg);

					return -EINVAL;
				}

				config.output_fmt = new_output_fmt;
				break;
			}

			case 'e': /* --entry-spec */
			{
				config.entry_spec = optarg;
				break;
			}

			case 'd': /* --csv-delimeter */
			{
				config.csv_delimeter = optarg;
				break;
			}

			case 'p': /* --ts-parse-spec */
			{
				config.ts_parse_spec = optarg;
				break;
			}

			case 'o': /* --ts-output-spec */
			{
				config.ts_output_spec = optarg;
				break;
			}

			case 'x': /* --html-class-prefix */
			{
				config.html_class_prefix = optarg;
				break;
			}

			case 'c': /* --html-cell-classes */
			{
				if ((strcmp(optarg, "on") == 0) ||
				    (strcmp(optarg, "true") == 0) ||
				    (strcmp(optarg, "1") == 0))
					config.html_cell_classes = 1;
				else
					config.html_cell_classes = 0;

				break;
			}

			default:
				break;
		}
	}

	if ((argc == optind) && !config.is_stdin)
	{
		fprintf(stderr, "%s: input file is not specified\n", argv[0]);
		return -EINVAL;
	}
	else
	{
		if (!config.is_stdin)
		{
			config.input_filename = argv[optind];
		}
		else if (argc > optind)
		{
			fprintf(stderr,
				"%s: can't specify both stdin and input file\n",
				argv[0]
			);

			return -EINVAL;
		}
	}

	return 0;
}

/**
 * Convert syslog file into other text format
 *
 * @param[in] input Pointer to the input syslog file structure
 *
 * @return 0 on success
 * @return <0 on error
 */
static int convert_syslog(FILE *input)
{
	int ret = 0;
	syslog_entry_t entry;

	char *buffer;
	size_t buffer_size = SYSLOG_BUFFER_SIZE;

	unsigned int parsed_n = 0;
	unsigned int line_n = 0;

	buffer = malloc(buffer_size);
	if (!buffer)
	{
		fprintf(stderr,
			"Failed to allocate memory for line buffer\n");

		return -ENOMEM;
	}

	ret = syslog_entry_init(&entry, config.entry_spec);
	if (ret)
	{
		fprintf(stderr,
			"Syslog entry initialization failed (%d)\n", ret);

		free(buffer);
		return ret;
	}

	if (config.output_fmt->fn_output_start)
		config.output_fmt->fn_output_start(&entry);

	while (1)
	{
		int status;
		size_t buffer_offset = 0;
		size_t line_len = 0;

		line_n++;

		while (1)
		{
			char *new_buffer;
			char *line;

			line = fgets(buffer + buffer_offset,
				buffer_size - buffer_offset, input);

			if (!line)
				break;

			line_len += strlen(line);

			if ((buffer[line_len - 1] == '\r') ||
			    (buffer[line_len - 1] == '\n'))
				break;

			/* Increase line buffer size */
			buffer_size += SYSLOG_BUFFER_SIZE;
			if (buffer_size > SYSLOG_MAX_BUFFER_SIZE)
			{
				fprintf(stderr,
					"line %u: Line buffer size limit (%u) reached\n",
					line_n, SYSLOG_MAX_BUFFER_SIZE);

				free(buffer);
				return -EINVAL;
			}

			new_buffer = realloc(buffer, buffer_size);
			if (!new_buffer)
			{
				fprintf(stderr,
					"line %u: Failed to reallocate memory for line buffer "
					"(%lu -> %lu)\n",
					line_n,
					buffer_size - SYSLOG_BUFFER_SIZE,
					buffer_size);

				free(buffer);
				return -ENOMEM;
			}

			buffer = new_buffer;
			buffer_offset = line_len;
		}

		if (!line_len) /* EOF */
			break;

		status = syslog_entry_parse(
			&entry, line_n, buffer);

		if (!status)
		{
			entry.num = ++parsed_n;

			if (config.output_fmt->fn_output_entry)
				config.output_fmt->fn_output_entry(&entry);
		}
	}

	if (config.output_fmt->fn_output_end)
		config.output_fmt->fn_output_end(&entry);

	syslog_entry_destroy(&entry);

	free(buffer);
	return ret;
}

/**
 * Program start point
 *
 * @param[in] argc  Number of arguments
 * @param[in] argv  Array of the pointers to the arguments
 *
 * @return 0 on success
 * @return <0 on error
 */
int main(int argc, char *argv[])
{
	int ret = 0;
	FILE *input;

	memcpy(&config, &default_config, sizeof(config));

	if (cli_args(argc, argv))
	{
		display_usage();
		return -EINVAL;
	}

	if (config.is_stdin)
		input = stdin;
	else
	{
		input = fopen(config.input_filename, "rb");
		if (!input)
		{
			fprintf(stderr, "%s: could not open file '%s'\n",
				argv[0], config.input_filename);

			return -ENODEV;
		}
	}

	ret = convert_syslog(input);

	if (!config.is_stdin)
		fclose(input);

	return ret;
}
