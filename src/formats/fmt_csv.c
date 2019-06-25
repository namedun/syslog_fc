/*
 * Syslog File Converter
 * Copyright © 2019 Anton Kikin <a.kikin@tano-systems.com>
 *
 * CSV output format support
 *
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file for more details.
 */

/**
 * @file
 * @brief CSV output format support
 *
 * @author Anton Kikin <a.kikin@tano-systems.com>
 */

#include <syslog_fc.h>

static void fmt_csv_output_encoded(const char *string)
{
	const char *p = string;

	fputc('"', stdout);

	while (*p)
	{
		if (*p == '"')
		{
			/*
			 * RFC 4180:
			 *
			 * If double-quotes are used to enclose fields,
			 * then a double-quote appearing inside a field
			 * must be escaped by preceding it with another
			 * double quote.  For example:
			 */
			fputs("\"\"", stdout);
		}
		else
			fputc(*p, stdout);

		p++;
	}

	fputc('"', stdout);
}

static void fmt_csv_output_start(const syslog_entry_t *entry)
{
	int count = 0;
	syslog_field_t *field;

	for (field = entry->fields; field; field = field->next)
	{
		if (field->flags & SYSLOG_FIELD_FLAG_DROP)
			continue;

		if (count)
			fputs(config.csv_delimeter, stdout);

		printf("%s", field->info->human_name);
			
		++count;
	}

	fputc('\n', stdout);
}

static void fmt_csv_output_entry(const syslog_entry_t *entry)
{
	int count = 0;
	syslog_field_t *field;

	for (field = entry->fields; field; field = field->next)
	{
		if (field->flags & SYSLOG_FIELD_FLAG_DROP)
			continue;

		if (count)
			fputs(config.csv_delimeter, stdout);

		switch(field->info->type)
		{
			case SYSLOG_FIELD_TYPE_TIME:
				fmt_csv_output_encoded(syslog_field_time_fmt(field));
				break;

			case SYSLOG_FIELD_TYPE_INTEGER:
				printf("%ld", field->value.integer);
				break;

			case SYSLOG_FIELD_TYPE_UINTEGER:
				printf("%lu", field->value.uinteger);
				break;

			case SYSLOG_FIELD_TYPE_STRING:
				fmt_csv_output_encoded(field->value.string);
				break;
		}

		++count;
	}

	fputc('\n', stdout);
}

output_fmt_t fmt_csv =
{
	.name              = "csv",
	.description       = "CSV (Comma-Separated Values)",
	.fn_output_start   = fmt_csv_output_start,
	.fn_output_end     = NULL,
	.fn_output_entry   = fmt_csv_output_entry
};
