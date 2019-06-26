/*
 * Syslog File Converter
 * Copyright © 2019 Anton Kikin <a.kikin@tano-systems.com>
 *
 * AsciiDoc output format support
 *
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file for more details.
 */

/**
 * @file
 * @brief AsciiDoc output format support
 *
 * @author Anton Kikin <a.kikin@tano-systems.com>
 */

#include <syslog_fc.h>

static void fmt_asciidoc_output_start(const syslog_entry_t *entry)
{
	int count = 0;
	syslog_field_t *field;

	fputs("[cols=\"", stdout);

	for (field = entry->fields; field; field = field->next)
	{
		if (field->flags & SYSLOG_FIELD_FLAG_DROP)
			continue;

		if (count)
			fputs(",", stdout);

		if (field->info->id == SYSLOG_FIELD_ID_TIMESTAMP)
			fputs("30", stdout);
		else if (field->info->id == SYSLOG_FIELD_ID_MESSAGE)
			fputs("70", stdout);
		else
			fputs("1", stdout);

		count++;
	}
	
	fputs("\", options=\"header\"]\n", stdout);
	fputs("|===\n", stdout);

	for (field = entry->fields; field; field = field->next)
	{
		if (!(field->flags & SYSLOG_FIELD_FLAG_DROP))
			printf("|%s\n", field->info->human_name);
	}
}

static void fmt_asciidoc_output_stop(const syslog_entry_t *entry)
{
	printf("|===\n");
}

static void fmt_asciidoc_output_entry(const syslog_entry_t *entry)
{
	syslog_field_t *field;

	printf("\n");

	for (field = entry->fields; field; field = field->next)
	{
		if (field->flags & SYSLOG_FIELD_FLAG_DROP)
			continue;

		printf("|");

		switch(field->info->type)
		{
			case SYSLOG_FIELD_TYPE_TIME:
				printf("%s", syslog_field_time_fmt(field));
				break;

			case SYSLOG_FIELD_TYPE_INTEGER:
				printf("%ld", field->value.integer);
				break;

			case SYSLOG_FIELD_TYPE_UINTEGER:
				printf("%lu", field->value.uinteger);
				break;

			case SYSLOG_FIELD_TYPE_STRING:
				if (field->info->id == SYSLOG_FIELD_ID_MESSAGE)
					printf("`%s`", field->value.string);
				else
					printf("%s", field->value.string);

				break;
		}

		printf("\n");
	}
}

output_fmt_t fmt_asciidoc =
{
	.name              = "asciidoc",
	.description       = "AsciiDoc table",
	.fn_output_start   = fmt_asciidoc_output_start,
	.fn_output_end     = fmt_asciidoc_output_stop,
	.fn_output_entry   = fmt_asciidoc_output_entry
};
