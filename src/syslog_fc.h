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
 * @brief Main header file
 *
 * @author Anton Kikin <a.kikin@tano-systems.com>
 */

#ifndef __SYSLOG_FC_H__
#define __SYSLOG_FC_H__

/** @brief Maximum syslog entry line size */
#define SYSLOG_MAX_LINE_SIZE  256

/** @brief Helper macro for array size calculation */
#define ARRAY_SIZE(a)  (sizeof(a) / sizeof(a[0]))

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h> /* struct tm, strptime */
#include <assert.h>

#include <syslog_entry.h>

/* ----------------------------------------------------------------------- */

/**
 * @brief Output format data structure
 */
typedef struct
{
	/** Name */
	char *name;

	/** Description */
	char *description;

	/** Output start callback function */
	void (*fn_output_start)(const syslog_entry_t *);

	/** Output entry data callback function */
	void (*fn_output_entry)(const syslog_entry_t *);

	/** Output end callback function */
	void (*fn_output_end)(const syslog_entry_t *);

} output_fmt_t;

/* ----------------------------------------------------------------------- */

/**
 * @brief Configuration data structure
 */
typedef struct config
{
	/** Read data from stdin */
	int is_stdin;

	/** Input file name */
	const char *input_filename;

	/** Syslog entry format */
	const char *entry_spec;

	/** Default output format */
	const output_fmt_t *output_fmt;

	/** Parsing timestamp conversion format */
	const char *ts_parse_spec;

	/** Output timestamp conversion format */
	const char *ts_output_spec;

	/** CSV delimeter */
	const char *csv_delimeter;

	/** HTML class prefix */
	const char *html_class_prefix;

	/** Enable or disable HTML classes for each cell */
	int html_cell_classes;

} config_t;

/**
 * @brief Global configuration structure
 */
extern config_t config;

/* ----------------------------------------------------------------------- */

#endif /* __SYSLOG_FC_H__ */
