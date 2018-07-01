/*
 * defs.h
 *
 *  Created on: 2018年6月23日
 *      Author: sp0917
 */

#include <stdio.h>

#ifndef SRC_COMMON_DEFS_H_
#define SRC_COMMON_DEFS_H_

static const struct table_entry {
	const char *extension;
	const char *content_type;
} content_type_table[] = {
		{ "txt", "text/plain" }, { "c", "text/plain" }, { "h", "text/plain" },
		{ "html", "text/html" }, { "htm", "text/htm" }, { "css", "text/css" },
		{ "gif", "image/gif" }, { "jpg", "image/jpg" }, { "jpeg", "image/jpeg" },
		{ "png", "images/png"}, { "pdf", "application/pdf"}, { "ps", "application/postscript"},
		{ NULL, NULL }
};

#endif /* SRC_COMMON_DEFS_H_ */
