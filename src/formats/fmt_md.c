/*
 * Syslog File Converter
 * Copyright © 2019 Anton Kikin <a.kikin@tano-systems.com>
 *
 * Markdown output format support
 *
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file for more details.
 */

/**
 * @file
 * @brief Markdown output format support
 *
 * @author Anton Kikin <a.kikin@tano-systems.com>
 */

#include <syslog_fc.h>

static void fmt_md_output_start(const syslog_entry_t *entry)
{
	syslog_field_t *field;

	for (field = entry->fields; field; field = field->next)
	{
		if (!(field->flags & SYSLOG_FIELD_FLAG_DROP))
			printf("|%s", field->info->human_name);
	}

	printf("|\n");

	for (field = entry->fields; field; field = field->next)
	{
		if (!(field->flags & SYSLOG_FIELD_FLAG_DROP))
			printf("|---");
	}

	printf("|\n");
}

static void fmt_md_output_entry(const syslog_entry_t *entry)
{
	syslog_field_t *field;

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
	}

	printf("|\n");
}

output_fmt_t fmt_md =
{
	.name              = "md",
	.description       = "Markdown table",
	.fn_output_start   = fmt_md_output_start,
	.fn_output_end     = NULL,
	.fn_output_entry   = fmt_md_output_entry
};
