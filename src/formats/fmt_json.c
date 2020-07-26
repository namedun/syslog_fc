/*
 * Syslog File Converter
 * Copyright © 2019 Anton Kikin <a.kikin@tano-systems.com>
 *
 * JSON ouput format support
 *
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file for more details.
 */

/**
 * @file
 * @brief JSON output format support
 *
 * @author Anton Kikin <a.kikin@tano-systems.com>
 */

#include <syslog_fc.h>

static void fmt_json_output_encoded(const char *string)
{
	const char *p = string;
	while (*p)
	{
		switch(*p)
		{
			case '\b': fputs("\\b",  stdout); break;
			case '\f': fputs("\\f",  stdout); break;
			case '\n': fputs("\\n",  stdout); break;
			case '\r': fputs("\\r",  stdout); break;
			case '\t': fputs("\\t",  stdout); break;
			case '\\': fputs("\\\\", stdout); break;
			case '"' : fputs("\\\"", stdout); break;
			case 0x1b: fputs("\\\\033", stdout); break;

			default:
				fputc(*p, stdout);
				break;
		}

		p++;
	}
}

static void fmt_json_output_start(const syslog_entry_t *entry)
{
	fputc('[', stdout);
}

static void fmt_json_output_entry(const syslog_entry_t *entry)
{
	int count = 0;
	syslog_field_t *field;

	printf("%s{", (entry->num > 1) ? "," : "");

	for (field = entry->fields; field; field = field->next)
	{
		if (field->flags & SYSLOG_FIELD_FLAG_DROP)
			continue;

		printf("%s\"%s\":",
			(count > 0) ? "," : "",
			field->info->param_name);

		switch(field->info->type)
		{
			case SYSLOG_FIELD_TYPE_TIME:
				fputc('"', stdout);
				fmt_json_output_encoded(syslog_field_time_fmt(field));
				fputc('"', stdout);
				break;

			case SYSLOG_FIELD_TYPE_INTEGER:
				printf("%ld", field->value.integer);
				break;

			case SYSLOG_FIELD_TYPE_UINTEGER:
				printf("%lu", field->value.uinteger);
				break;

			case SYSLOG_FIELD_TYPE_STRING:
				fputc('"', stdout);
				fmt_json_output_encoded(field->value.string);
				fputc('"', stdout);
				break;
		}

		++count;
	}

	fputc('}', stdout);
}

static void fmt_json_output_end(const syslog_entry_t *entry)
{
	fputc(']', stdout);
}

output_fmt_t fmt_json =
{
	.name              = "json",
	.description       = "JSON (JavaScript Object Notation)",
	.fn_output_start   = fmt_json_output_start,
	.fn_output_end     = fmt_json_output_end,
	.fn_output_entry   = fmt_json_output_entry
};
