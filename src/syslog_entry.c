/*
 * Syslog File Converter
 * Copyright © 2019 Anton Kikin <a.kikin@tano-systems.com>
 *
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file for more details.
 */

/**
 * @file
 * @brief Syslog entry source
 *
 * @author Anton Kikin <a.kikin@tano-systems.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define SYSLOG_NAMES
#include <syslog.h> /* prioritynames, facilitynames */

#include <syslog_fc.h>

/**
 * @name Extended syslog entry format specificators
 * @{
 */

/** @brief Drop field data from output */
#define SYSLOG_EXT_SPEC_DROP '!'

/** @brief Do not trim spaces at the beginning */
#define SYSLOG_EXT_SPEC_NOTRIM '_'

/** @brief Do not validate parsed value */
#define SYSLOG_EXT_SPEC_NOVALIDATION '@'

/** @} */

/* ----------------------------------------------------------------------- */

static int validate_facility(const struct syslog_field *field);
static int validate_priority(const struct syslog_field *field);

/**
 * @brief Available syslog fields information array
 */
static syslog_field_info_t syslog_field_info[] =
{
	{
		.id         = SYSLOG_FIELD_ID_TIMESTAMP,
		.type       = SYSLOG_FIELD_TYPE_TIME,
		.spec       = 'T',
		.param_name = "timestamp",
		.human_name = "Timestamp",
	},
	{
		.type       = SYSLOG_FIELD_TYPE_STRING,
		.id         = SYSLOG_FIELD_ID_HOSTNAME,
		.spec       = 'H',
		.param_name = "hostname",
		.human_name = "Hostname",
	},
	{
		.id         = SYSLOG_FIELD_ID_FACILITY,
		.type       = SYSLOG_FIELD_TYPE_STRING,
		.spec       = 'F',
		.param_name = "facility",
		.human_name = "Facility",
		.validator  = validate_facility
	},
	{
		.id         = SYSLOG_FIELD_ID_PRIORITY,
		.type       = SYSLOG_FIELD_TYPE_STRING,
		.spec       = 'P',
		.param_name = "priority",
		.human_name = "Priority",
		.validator  = validate_priority
	},
	{
		.id         = SYSLOG_FIELD_ID_TAG,
		.type       = SYSLOG_FIELD_TYPE_STRING,
		.spec       = 'G',
		.param_name = "tag",
		.human_name = "Tag",
	},
	{
		.id         = SYSLOG_FIELD_ID_MESSAGE,
		.type       = SYSLOG_FIELD_TYPE_STRING,
		.spec       = 'M',
		.param_name = "message",
		.human_name = "Message",
	},
};

/* ----------------------------------------------------------------------- */

/**
 * Checks for the presence of the specified name in the array
 * of the CODE items.
 *
 * Type of the structure CODE declared in default header file
 * @p <sys/syslog.h>.
 *
 * @param[in] code Pointer to the array of the CODE items
 * @param[in] name Name to check
 *
 * @return Pointer to the founded name from @p code list
 * @return NULL if name is not founded in the @p code list
 */
static const CODE *find_syslog_name(
	const CODE *code,
	const char *name)
{
	const CODE *c;

	assert(code);
	assert(name);

	for (c = code; c->c_name; c++)
	{
		if (strcmp(name, c->c_name) == 0)
			return c;
	}

	return NULL;
}

/**
 * Validate syslog facility name in the syslog entry field value.
 *
 * @param[in] field  Pointer to the syslog entry field.
 *
 * @return 1 if fields priority name is valid, 0 otherwise.
 */
static int validate_facility(const struct syslog_field *field)
{
	assert(field);
	assert(field->info->id == SYSLOG_FIELD_ID_FACILITY);

	if (find_syslog_name(facilitynames, field->value.string))
		return 0;

	return -EINVAL;
}

/**
 * Validate syslog priority name in the syslog entry field value.
 *
 * @param[in] field  Pointer to the syslog entry field.
 *
 * @return 1 if fields priority name is valid, 0 otherwise.
 */
static int validate_priority(const struct syslog_field *field)
{
	assert(field);
	assert(field->info->id == SYSLOG_FIELD_ID_PRIORITY);

	if (find_syslog_name(prioritynames, field->value.string))
		return 0;

	return -EINVAL;
}

/* ----------------------------------------------------------------------- */

int syslog_entry_init(
	syslog_entry_t *entry,
	const char *entry_spec
)
{
	int i;
	unsigned int flags = 0;

	syslog_field_t *last_field = NULL;
	syslog_field_t *field = NULL;

	const syslog_field_info_t *field_info = NULL;
	char parse_start_char = 0;
	char ch;

	const char *p = entry_spec;

	assert(entry);
	assert(entry_spec);

	entry->num = 0;
	entry->fields = NULL;
	entry->fields_mask = 0;

	for( ; *p; p++)
	{
		if (*p != '%')
		{
			if (last_field && (last_field->parse_stop_char == '\0'))
				last_field->parse_stop_char = *p;
			else
				parse_start_char = *p;

			continue;
		}

		if (last_field && (last_field->parse_stop_char == '\0'))
		{
			/* Invalid format specification.
			 * No character between two specificators */
			return -EINVAL;
		}

	get_next_ch:
		ch = *(++p);

		if (ch == SYSLOG_EXT_SPEC_DROP)
		{
			flags |= SYSLOG_FIELD_FLAG_DROP;
			goto get_next_ch;
		}
		else if (ch == SYSLOG_EXT_SPEC_NOTRIM)
		{
			flags |= SYSLOG_FIELD_FLAG_NOTRIM;
			goto get_next_ch;
		}
		else if (ch == SYSLOG_EXT_SPEC_NOVALIDATION)
		{
			flags |= SYSLOG_FIELD_FLAG_NOVALIDATION;
			goto get_next_ch;
		}

		for (i = 0; i < ARRAY_SIZE(syslog_field_info); i++)
		{
			if (ch == syslog_field_info[i].spec)
			{
				field_info = &syslog_field_info[i];
				break;
			}
		}

		if (!field_info)
		{
			if (ch == '%')
			{
				if (last_field && (last_field->parse_stop_char == 0))
					last_field->parse_stop_char = ch;

				continue;
			}

			/* Invalid format specification.
			 * Unknown syslog file entry specificator (ch) */
			return -EINVAL;
		}

		if (!(flags & SYSLOG_FIELD_FLAG_DROP) &&
		    entry->fields_mask & (1 << field_info->id))
		{
			/* Invalid format specification.
			 * Duplicate entry specificator (ch) */
			return -EINVAL;
		}

		entry->fields_mask |= (1 << field_info->id);

		if (!field)
		{
			field = malloc(sizeof(syslog_field_t));
			if (!field)
				return -ENOMEM;
		}

		field->flags            = flags;
		field->info             = field_info;
		field->next             = NULL;
		field->parse_start_char = parse_start_char;
		field->parse_stop_char  = 0;

		if (!entry->fields)
		{
			entry->fields = last_field = field;
		}
		else
		{
			last_field->next = field;
			last_field = field;
		}

		flags = 0;
		field = NULL;
		parse_start_char = 0;
	}

	return 0;

}

void syslog_entry_destroy(syslog_entry_t *entry)
{
	syslog_field_t *field = entry->fields;

	while(field)
	{
		syslog_field_t *field_next = field->next;
		free(field);
		field = field_next;
	}
}

/* ----------------------------------------------------------------------- */

/**
 * Skip spaces at beginning of string.
 *
 * @param[in] p Pointer to the string.
 *
 * @return Pointer to the first non-space character in the string @p p.
 */
static char *strskipspaces(const char *p)
{
	while(isspace(*p))
		p++;

	return (char *)p;
}

/* ----------------------------------------------------------------------- */

/**
 * Timestamp parsing
 *
 * Parsed timestamp will be stored into @p field->value.time
 * (@ref syslog_field_t::syslog_field_value_union.time).
 *
 * @param[in,out] data  Pointer to the buffer with syslog file data.
 * @param[in,out] field Pointer to the syslog entry field data structure.
 *
 * @return 0 on success
 * @return <0 on error
 */
static int parse_timestamp(
	char **data,
	syslog_field_t *field
)
{
	*data = strptime(
		*data, config.ts_parse_spec,
		&field->value.time.timestamp
	);

	if (*data)
	{
		field->value.time.unixtime =
			timelocal(&field->value.time.timestamp);
	}

	return *data ? 0 : -EILSEQ;
}

/**
 * String parsing
 *
 * Pointer to the parsed string will be stored into @p field->value.string
 * (@ref syslog_field_t::syslog_field_value_union.string).
 *
 * @param[in,out] data  Pointer to the buffer with syslog file data.
 * @param[in,out] field Pointer to the syslog entry field data structure.
 *
 * @return 0 on success
 * @return <0 on error
 */
static int parse_string(
	char **data,
	syslog_field_t *field
)
{
	char *p = *data;

	if (field->flags & SYSLOG_FIELD_FLAG_NOTRIM)
	{
		/*
		 * If we do not remove spaces from the data, then we must find
		 * the first non-space character in order to correctly find
		 * stop character, that can be space space character.
		 */
		p = strskipspaces(p);
	}

	p = strchr(p, field->parse_stop_char);
	if (!p)
		return -EILSEQ;

	*p = '\0';

	field->value.string = *data;
	*data = p + 1;

	if ((p = strpbrk(field->value.string, "\r\n")) != NULL)
		*p = '\0';

	return 0;
}

/**
 * Integer parsing
 *
 * Parsed integer will be stored into @p field->value.integer
 * (@ref syslog_field_t::syslog_field_value_union.integer).
 *
 * @param[in,out] data  Pointer to the buffer with syslog file data.
 * @param[in,out] field Pointer to the syslog entry field data structure.
 *
 * @return 0 on success
 * @return <0 on error
 */
static int parse_integer(
	char **data,
	syslog_field_t *field
)
{
	int ret = parse_string(data, field);

	if (!ret)
		field->value.integer = strtol(field->value.string, NULL, 0);

	return ret;
}

/**
 * Unsigned integer parsing
 *
 * Parsed unsigned integer will be stored into @p field->value.uinteger
 * (@ref syslog_field_t::syslog_field_value_union.uinteger).
 *
 * @param[in,out] data  Pointer to the buffer with syslog file data.
 * @param[in,out] field Pointer to the syslog entry field data structure.
 *
 * @return 0 on success
 * @return <0 on error
 */
static int parse_uinteger(
	char **data,
	syslog_field_t *field
)
{
	int ret = parse_string(data, field);

	if (!ret)
		field->value.uinteger = strtoul(field->value.string, NULL, 0);

	return ret;
}

/**
 * Syslog entry field parsing function
 *
 * @param[in]     line_n  Input line number (used only for output
 *                        in error messages).
 * @param[in,out] data    Pointer to the buffer with syslog file data.
 * @param[in,out] field   Pointer to the syslog entry field data structure.
 *
 * @return 0 on success
 * @return <0 on error
 */
static int syslog_entry_field_parse(
	unsigned int line_n,
	char **data,
	syslog_field_t *field
)
{
	int ret;

	if (field->parse_start_char)
	{
		*data = strchr(*data, field->parse_start_char);
		if (!*data)
		{
			/* Can't find start char */
			return -EILSEQ;
		}

		++(*data);
	}

	if (!(field->flags & SYSLOG_FIELD_FLAG_NOTRIM))
		*data = strskipspaces(*data);

	switch(field->info->type)
	{
		case SYSLOG_FIELD_TYPE_TIME:
			ret = parse_timestamp(data, field);
			break;

		case SYSLOG_FIELD_TYPE_STRING:
			ret = parse_string(data, field);
			break;

		case SYSLOG_FIELD_TYPE_INTEGER:
			ret = parse_integer(data, field);
			break;

		case SYSLOG_FIELD_TYPE_UINTEGER:
			ret = parse_uinteger(data, field);
			break;

		default:
			ret = -EINVAL;
			break;
	}

	if (ret)
	{
		fprintf(stderr,
			"line %u: Failed to parse '%s' field (%d)\n",
			line_n, field->info->param_name, ret
		);

		return ret;
	}

	/* Validate parsed value */
	if (!(field->flags & SYSLOG_FIELD_FLAG_NOVALIDATION) &&
	    field->info->validator)
	{
		ret = field->info->validator(field);
		if (ret)
		{
			fprintf(stderr,
				"line %u: Readed invalid value for field '%s' (%d)\n",
				line_n, field->info->param_name, ret
			);

			return ret;
		}
	}

	return 0;
}

/* ----------------------------------------------------------------------- */

int syslog_entry_parse(
	syslog_entry_t *entry,
	unsigned int line_n,
	char *line
)
{
	syslog_field_t *field;
	char *data = line;

	assert(entry);
	assert(line);

	for (field = entry->fields; field; field = field->next)
	{
		int ret = syslog_entry_field_parse(line_n, &data, field);
		if (ret)
			return ret;
	}

	return 0;
}

/* ----------------------------------------------------------------------- */

char *syslog_field_time_fmt(const syslog_field_t *field)
{
	static char buffer[128];

	if (config.ts_output_spec && config.ts_output_spec[0])
	{
		strftime(buffer, sizeof(buffer), config.ts_output_spec,
			&field->value.time.timestamp);
	}
	else
	{
		snprintf(buffer, sizeof(buffer), "%lu",
			field->value.time.unixtime);
	}

	return buffer;
}

/* ----------------------------------------------------------------------- */
