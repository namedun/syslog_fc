/*
 * Syslog File Converter
 * Copyright © 2019 Anton Kikin <a.kikin@tano-systems.com>
 *
 * HTML ouput format support
 *
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file for more details.
 */

/**
 * @file
 * @brief HTML output format support
 *
 * @author Anton Kikin <a.kikin@tano-systems.com>
 */

#include <syslog_fc.h>

static void fmt_html_open_tag(
	const char *tag,
	const char *class_prefix,
	const char *class
)
{
	if (class)
	{
		fprintf(stdout, "<%s class=\"%s%s\">",
			tag, class_prefix ? class_prefix : "", class);
	}
	else
	{
		fprintf(stdout, "<%s>", tag);
	}
}

static void fmt_html_close_tag(const char *tag)
{
	fprintf(stdout, "</%s>", tag);
}

static void fmt_html_output_encoded(const char *string)
{
	const char *p = string;
	while (*p)
	{
		switch(*p)
		{
			case '\n': fputs("<br />",  stdout); break;
			case '&' : fputs("&amp;", stdout); break;
			case '<' : fputs("&lt;", stdout); break;
			case '>' : fputs("&gt;", stdout); break;

			default:
				fputc(*p, stdout);
				break;
		}

		p++;
	}
}

static void fmt_html_output_row(
	const char *html_cell_tag,
	const syslog_entry_t *entry
)
{
	syslog_field_t *field;
	char *tr_class = NULL;

	/* Set <tr> class by priority field value */
	if (entry->num &&
	    syslog_entry_has_field(entry, SYSLOG_FIELD_ID_PRIORITY))
	{
		for (field = entry->fields; field; field = field->next)
		{
			if (field->info->id == SYSLOG_FIELD_ID_PRIORITY)
			{
				tr_class = field->value.string;
				break;
			}
		}
	}

	/* Start table row */
	fmt_html_open_tag("tr", config.html_class_prefix, tr_class);

	for (field = entry->fields; field; field = field->next)
	{
		if (field->flags & SYSLOG_FIELD_FLAG_DROP)
			continue;

		/* Start table cell */
		fmt_html_open_tag(html_cell_tag,
			config.html_class_prefix,
			config.html_cell_classes ? field->info->param_name : NULL);

		if (entry->num)
		{
			/* Value */
			switch(field->info->type)
			{
				case SYSLOG_FIELD_TYPE_TIME:
					fmt_html_output_encoded(syslog_field_time_fmt(field));
					break;

				case SYSLOG_FIELD_TYPE_INTEGER:
					printf("%ld", field->value.integer);
					break;

				case SYSLOG_FIELD_TYPE_UINTEGER:
					printf("%lu", field->value.uinteger);
					break;

				case SYSLOG_FIELD_TYPE_STRING:
					if (field->info->id == SYSLOG_FIELD_ID_MESSAGE)
					{
						fmt_html_open_tag("pre", NULL, NULL);
						fmt_html_output_encoded(field->value.string);
						fmt_html_close_tag("pre");
					}
					else
						fmt_html_output_encoded(field->value.string);

					break;
			}
		}
		else
		{
			/* Header */
			fmt_html_output_encoded(field->info->human_name);
		}

		/* End table cell */
		fmt_html_close_tag(html_cell_tag);
	}

	/* End table row */
	fmt_html_close_tag("tr");
}

static void fmt_html_output_start(const syslog_entry_t *entry)
{
	/* Start table */
	fmt_html_open_tag("table", config.html_class_prefix, "table");

	/* Start heading */
	fmt_html_open_tag("thead", NULL, NULL);

	/* Heading row */
	fmt_html_output_row("th", entry);

	/* End heading */
	fmt_html_close_tag("thead");

	/* Start body */
	fmt_html_open_tag("tbody", NULL, NULL);
}

static void fmt_html_output_entry(const syslog_entry_t *entry)
{
	/* Heading row */
	fmt_html_output_row("td", entry);
}

static void fmt_html_output_end(const syslog_entry_t *entry)
{
	/* End body and table */
	fmt_html_close_tag("tbody");
	fmt_html_close_tag("table");
}

output_fmt_t fmt_html =
{
	.name              = "html",
	.description       = "HTML (HyperText Markup Language) table",
	.fn_output_start   = fmt_html_output_start,
	.fn_output_end     = fmt_html_output_end,
	.fn_output_entry   = fmt_html_output_entry
};
