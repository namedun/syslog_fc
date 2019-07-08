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
 * @brief Syslog entry header
 *
 * @author Anton Kikin <a.kikin@tano-systems.com>
 */

#ifndef __SYSLOG_ENTRY_H__
#define __SYSLOG_ENTRY_H__

struct syslog_entry;
struct syslog_field;

/* ----------------------------------------------------------------------- */

/**
 * @name Syslog entry field flags
 * @{
 */

/** @brief Drop field data from output */
#define SYSLOG_FIELD_FLAG_DROP  (1 << 0)

/** @brief Do not skip spaces at the beginning */
#define SYSLOG_FIELD_FLAG_NOTRIM  (1 << 1)

/** @brief Skip validation */
#define SYSLOG_FIELD_FLAG_NOVALIDATION (1 << 2)

/** @} */

/* ----------------------------------------------------------------------- */

/**
 * @brief Syslog entry field types.
 */
typedef enum
{
	SYSLOG_FIELD_TYPE_TIME,     /**< Date and time */
	SYSLOG_FIELD_TYPE_INTEGER,  /**< Signed integer */
	SYSLOG_FIELD_TYPE_UINTEGER, /**< Unsigned integer */
	SYSLOG_FIELD_TYPE_STRING,   /**< String */

} syslog_field_type_t;

/**
 * @brief Syslog entriy field identifiers
 */
typedef enum
{
	SYSLOG_FIELD_ID_ID,         /**< Numeric identifier */
	SYSLOG_FIELD_ID_TIMESTAMP,  /**< Date and time */
	SYSLOG_FIELD_ID_KTIME,      /**< Kernel time */
	SYSLOG_FIELD_ID_HOSTNAME,   /**< Hostname */
	SYSLOG_FIELD_ID_FACILITY,   /**< Facility */
	SYSLOG_FIELD_ID_PRIORITY,   /**< Priority */
	SYSLOG_FIELD_ID_TAG,        /**< Tag */
	SYSLOG_FIELD_ID_MESSAGE,    /**< Message */

} syslog_field_id_t;

/**
 * @brief Syslog field information structure
 */
typedef struct syslog_field_info
{
	syslog_field_id_t id;       /**< Identifier */
	syslog_field_type_t type;   /**< Type */
	char spec;                  /**< Specificator character */
	char *param_name;           /**< Parameter name */
	char *human_name;           /**< Human-readable name */

	/**
	 * @brief Validation function pointer
	 *
	 * Function has one argument @p field, which
	 * is a pointer to the syslog entry field data structure.
	 *
	 * Function must return 0 if field data is valid,
	 * and < 0 otherwise.
	 */
	int (*validator)(const struct syslog_field *field);

} syslog_field_info_t;

/**
 * @brief Syslog field data structure
 */
typedef struct syslog_field
{
	/** Field information */
	const syslog_field_info_t *info;

	/** Additional field flags */
	unsigned int flags;

	/** Field value */
	union syslog_field_value_union
	{
		/**
		 * @brief Structure used to store data for
		 *        type #SYSLOG_FIELD_TYPE_TIME
		 */
		struct
		{
			struct tm timestamp;    /**< Date/time structure */
			unsigned long unixtime; /**< Unix timestamp */

		} time;

		/** Variable to store data for the
		 *  type #SYSLOG_FIELD_TYPE_UINTEGER */
		unsigned long uinteger;

		/** Variable to store data for the
		 *  type #SYSLOG_FIELD_TYPE_INTEGER */
		long integer;

		/** Variable to store data for the
		 *  type #SYSLOG_FIELD_TYPE_STRING */
		char *string;

	} value;

	/** Parsing start character */
	char parse_start_char;

	/** Parsing stop character */
	char parse_stop_char;

	/** Next field pointer */
	struct syslog_field *next;

} syslog_field_t;

/**
 * @brief Syslog entry data structure
 */
typedef struct syslog_entry
{
	unsigned int num;               /**< Entry number */
	unsigned int fields_mask;       /**< Used fields mask */
	unsigned int fields_num;        /**< Total number of fields */
	unsigned int fields_output_num; /**< Number of fields for output */
	syslog_field_t *fields;         /**< Fields list */

} syslog_entry_t;

/* ----------------------------------------------------------------------- */

/**
 * Initialize entry data structure by specified entry
 * format specification
 *
 * @param[out] entry      Pointer to the entry data structure.
 * @param[in]  entry_spec Entry format specification.
 *
 * @return 0 on success
 * @return <0 on error
 */
int syslog_entry_init(
	syslog_entry_t *entry,
	const char *entry_spec
);

/**
 * Free resources allocated for syslog entry data structure.
 *
 * @param[in] entry  Pointer to the entry data structure.
 */
void syslog_entry_destroy(syslog_entry_t *entry);

/**
 * Syslog entry line parsing function
 *
 * @param[in,out] entry   Pointer to the entry data structure.
 * @param[in]     line_n  Input line number (used only for output
 *                        in error messages).
 * @param[in,out] line    Pointer to the NULL-terminated syslog
 *                        file message. Source buffer data will be
 *                        modified by function.
 *
 * @return 0 on success
 * @return <0 on error
 */
int syslog_entry_parse(
	syslog_entry_t *entry,
	unsigned int line_n,
	char *line
);

/**
 * Check for the presence of a specified field in the entry
 *
 * The set of entry fields is determined by the entry format
 * specificator during initialization by syslog_entry_destroy()
 * function.
 *
 * @param[in] entry     Pointer to the entry data structure.
 * @param[in] field_id  Field identifier to check.
 *
 * @return 1 if field is present, 0 otherwise.
 */
static inline int syslog_entry_has_field(
	const syslog_entry_t *entry,
	const syslog_field_id_t field_id
)
{
	return !!(entry->fields_mask & (1 << field_id));
}

/* ----------------------------------------------------------------------- */

/**
 * Format field's timestamp value into buffer.
 *
 * The format is determined by the output timestamp format
 * specificator in @ref config_t::ts_output_spec of the global
 * configuiration @ref config.
 *
 * @attention
 *   ATTENTION: This function uses single static buffer for work.
 *
 * @attention
 *   DO NOT CALL this function repeatedly until you are convinced
 *   that the results of the previous call are no longer needed
 *   and are not used anywhere.
 *
 * @attention
 *   DO NOT USE this function more than in one single thread.
 *   This function is NOT threadsafe.
 *
 * @param[in] field Pointer to the field data structure.
 *
 * @return Pointer to the formatted string with timestamp.
 */
char *syslog_field_time_fmt(const syslog_field_t *field);

/* ----------------------------------------------------------------------- */

#endif /* __SYSLOG_ENTRY_H__ */
