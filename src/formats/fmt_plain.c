/*
 * Syslog File Converter
 * Copyright © 2019 Anton Kikin <a.kikin@tano-systems.com>
 *
 * Plain text output format support
 *
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file for more details.
 */

/**
 * @file
 * @brief Plaintext output format support
 *
 * @author Anton Kikin <a.kikin@tano-systems.com>
 */

#include <syslog_fc.h>

static void fmt_plain_output_entry(const syslog_entry_t *entry)
{
	syslog_field_t *field;

	for (field = entry->fields; field; field = field->next)
	{
		if (field->flags & SYSLOG_FIELD_FLAG_DROP)
			continue;

		printf("%-10s : ", field->info->human_name);

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
				printf("%s", field->value.string);
				break;
		}

		fputc('\n', stdout);
	}

	fputc('\n', stdout);
}

output_fmt_t fmt_plain =
{
	.name              = "plain",
	.description       = "Plain text",
	.fn_output_start   = NULL,
	.fn_output_end     = NULL,
	.fn_output_entry   = fmt_plain_output_entry
};
