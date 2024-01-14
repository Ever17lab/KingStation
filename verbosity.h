/*  KingStation - A frontend for libks.
 *  Copyright (C) 2011-2017 - Daniel De Matteis
 *
 *  KingStation is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  KingStation is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with KingStation.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __KINGSN_VERBOSITY_H
#define __KINGSN_VERBOSITY_H

#include <stdarg.h>
#include <stdlib.h>

#include <boolean.h>
#include <ks_common_api.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

KS_BEGIN_DECLS

#define FILE_PATH_LOG_DBG   "[VERBOSE]"
#define FILE_PATH_LOG_INFO  "[INFO]"
#define FILE_PATH_LOG_ERROR "[ERROR]"
#define FILE_PATH_LOG_WARN  "[WARN]"

bool verbosity_is_enabled(void);

void verbosity_enable(void);

void verbosity_disable(void);

void verbosity_set_log_level(unsigned level);

bool *verbosity_get_ptr(void);

void ks_main_log_file_deinit(void);

void ks_main_log_file_init(const char *path, bool append);

bool is_logging_to_file(void);

#if defined(HAVE_LOGGER)

void logger_init (void);
void logger_shutdown (void);
void logger_send (const char *__format,...);
void logger_send_v(const char *__format, va_list args);

#ifdef IS_SALAMANDER

#define KINGSN_DBG(...) do { \
   logger_send("KingStation Salamander: " __VA_ARGS__); \
} while (0)

#define KINGSN_LOG(...) do { \
   logger_send("KingStation Salamander: " __VA_ARGS__); \
} while (0)

#define KINGSN_LOG_V(tag, fmt, vp) do { \
   logger_send("KingStation Salamander: " tag); \
   logger_send_v(fmt, vp); \
} while (0)

#define KINGSN_LOG_OUTPUT(...) do { \
   logger_send("[OUTPUT] " __VA_ARGS__); \
} while (0)

#define KINGSN_LOG_OUTPUT_V(tag, fmt, vp) do { \
   logger_send("[OUTPUT] " tag); \
   logger_send_v(fmt, vp); \
} while (0)

#define KINGSN_ERR(...) do { \
   logger_send("[ERROR] " __VA_ARGS__); \
} while (0)

#define KINGSN_ERR_V(tag, fmt, vp) do { \
   logger_send("[ERROR] " tag); \
   logger_send_v(fmt, vp); \
} while (0)

#define KINGSN_WARN(...) do { \
   logger_send("[WARN] " __VA_ARGS__); \
} while (0)

#define KINGSN_WARN_V(tag, fmt, vp) do { \
   logger_send("[WARN] " tag); \
   logger_send_v(fmt, vp); \
} while (0)

#else /* IS_SALAMANDER */

#define KINGSN_DBG(...) do { \
   logger_send("" __VA_ARGS__); \
} while (0)

#define KINGSN_LOG(...) do { \
   logger_send("" __VA_ARGS__); \
} while (0)

#define KINGSN_LOG_V(tag, fmt, vp) do { \
   logger_send("" tag); \
   logger_send_v(fmt, vp); \
} while (0)

#define KINGSN_ERR(...) do { \
   logger_send("[ERROR] " __VA_ARGS__); \
} while (0)

#define KINGSN_ERR_V(tag, fmt, vp) do { \
   logger_send("[ERROR] " tag); \
   logger_send_v(fmt, vp); \
} while (0)

#define KINGSN_WARN(...) do { \
   logger_send("[WARN] " __VA_ARGS__); \
} while (0)

#define KINGSN_WARN_V(tag, fmt, vp) do { \
   logger_send("[WARN] :: " tag); \
   logger_send_v(fmt, vp); \
} while (0)

#define KINGSN_LOG_OUTPUT(...) do { \
   logger_send("[OUTPUT] " __VA_ARGS__); \
} while (0)

#define KINGSN_LOG_OUTPUT_V(tag, fmt, vp) do { \
   logger_send("[OUTPUT] " tag); \
   logger_send_v(fmt, vp); \
} while (0)
#endif

#define KINGSN_LOG_BUFFER(...) do { } while (0)

#else /* HAVE_LOGGER */
void KINGSN_LOG_V(const char *tag, const char *fmt, va_list ap);
void KINGSN_DBG(const char *fmt, ...);
void KINGSN_LOG(const char *fmt, ...);
void KINGSN_LOG_BUFFER(uint8_t *buffer, size_t size);
void KINGSN_LOG_OUTPUT(const char *msg, ...);
void KINGSN_WARN(const char *fmt, ...);
void KINGSN_ERR(const char *fmt, ...);

#define KINGSN_LOG_OUTPUT_V KINGSN_LOG_V
#define KINGSN_WARN_V KINGSN_LOG_V
#define KINGSN_ERR_V KINGSN_LOG_V
#endif /* HAVE_LOGGER */

void kingsn_log_file_init(
      bool log_to_file,
      bool log_to_file_timestamp,
      const char *log_dir);

void kingsn_log_file_deinit(void);

void kingsn_log_file_set_override(const char *path);


KS_END_DECLS

#endif
