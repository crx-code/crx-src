/*
   +----------------------------------------------------------------------+
   | Copyright (c) The CRX Group                                          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the CRX license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | https://www.crx.net/license/3_01.txt                                 |
   | If you did not receive a copy of the CRX license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@crx.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Wez Furlong <wez@thebrainroom.com>                           |
   +----------------------------------------------------------------------+
 */

/* Stream context and status notification related definitions */

/* callback for status notifications */
typedef void (*crx_stream_notification_func)(crx_stream_context *context,
		int notifycode, int severity,
		char *xmsg, int xcode,
		size_t bytes_sofar, size_t bytes_max,
		void * ptr);

#define CRX_STREAM_NOTIFIER_PROGRESS	1

/* Attempt to fetch context from the zval passed,
   If no context was passed, use the default context
   The default context has not yet been created, do it now. */
#define crx_stream_context_from_zval(zcontext, nocontext) ( \
		(zcontext) ? crex_fetch_resource_ex(zcontext, "Stream-Context", crx_le_stream_context()) : \
		(nocontext) ? NULL : \
		FG(default_context) ? FG(default_context) : \
		(FG(default_context) = crx_stream_context_alloc()) )

#define crx_stream_context_to_zval(context, zval) { ZVAL_RES(zval, (context)->res); GC_ADDREF((context)->res); }

typedef struct _crx_stream_notifier crx_stream_notifier;

struct _crx_stream_notifier {
	crx_stream_notification_func func;
	void (*dtor)(crx_stream_notifier *notifier);
	zval ptr;
	int mask;
	size_t progress, progress_max; /* position for progress notification */
};

struct _crx_stream_context {
	crx_stream_notifier *notifier;
	zval options;	/* hash keyed by wrapper family or specific wrapper */
	crex_resource *res;	/* used for auto-cleanup */
};

BEGIN_EXTERN_C()
CRXAPI void crx_stream_context_free(crx_stream_context *context);
CRXAPI crx_stream_context *crx_stream_context_alloc(void);
CRXAPI zval *crx_stream_context_get_option(crx_stream_context *context,
		const char *wrappername, const char *optionname);
CRXAPI void crx_stream_context_set_option(crx_stream_context *context,
		const char *wrappername, const char *optionname, zval *optionvalue);

CRXAPI crx_stream_notifier *crx_stream_notification_alloc(void);
CRXAPI void crx_stream_notification_free(crx_stream_notifier *notifier);
END_EXTERN_C()

/* not all notification codes are implemented */
#define CRX_STREAM_NOTIFY_RESOLVE		1
#define CRX_STREAM_NOTIFY_CONNECT		2
#define CRX_STREAM_NOTIFY_AUTH_REQUIRED		3
#define CRX_STREAM_NOTIFY_MIME_TYPE_IS	4
#define CRX_STREAM_NOTIFY_FILE_SIZE_IS	5
#define CRX_STREAM_NOTIFY_REDIRECTED	6
#define CRX_STREAM_NOTIFY_PROGRESS		7
#define CRX_STREAM_NOTIFY_COMPLETED		8
#define CRX_STREAM_NOTIFY_FAILURE		9
#define CRX_STREAM_NOTIFY_AUTH_RESULT	10

#define CRX_STREAM_NOTIFY_SEVERITY_INFO	0
#define CRX_STREAM_NOTIFY_SEVERITY_WARN	1
#define CRX_STREAM_NOTIFY_SEVERITY_ERR	2

BEGIN_EXTERN_C()
CRXAPI void crx_stream_notification_notify(crx_stream_context *context, int notifycode, int severity,
		char *xmsg, int xcode, size_t bytes_sofar, size_t bytes_max, void * ptr);
CRXAPI crx_stream_context *crx_stream_context_set(crx_stream *stream, crx_stream_context *context);
END_EXTERN_C()

#define crx_stream_notify_info(context, code, xmsg, xcode)	do { if ((context) && (context)->notifier) { \
	crx_stream_notification_notify((context), (code), CRX_STREAM_NOTIFY_SEVERITY_INFO, \
				(xmsg), (xcode), 0, 0, NULL); } } while (0)

#define crx_stream_notify_progress(context, bsofar, bmax) do { if ((context) && (context)->notifier) { \
	crx_stream_notification_notify((context), CRX_STREAM_NOTIFY_PROGRESS, CRX_STREAM_NOTIFY_SEVERITY_INFO, \
			NULL, 0, (bsofar), (bmax), NULL); } } while(0)

#define crx_stream_notify_completed(context) do { if ((context) && (context)->notifier) { \
	crx_stream_notification_notify((context), CRX_STREAM_NOTIFY_COMPLETED, CRX_STREAM_NOTIFY_SEVERITY_INFO, \
			NULL, 0, (context)->notifier->progress, (context)->notifier->progress_max, NULL); } } while(0)

#define crx_stream_notify_progress_init(context, sofar, bmax) do { if ((context) && (context)->notifier) { \
	(context)->notifier->progress = (sofar); \
	(context)->notifier->progress_max = (bmax); \
	(context)->notifier->mask |= CRX_STREAM_NOTIFIER_PROGRESS; \
	crx_stream_notify_progress((context), (sofar), (bmax)); } } while (0)

#define crx_stream_notify_progress_increment(context, dsofar, dmax) do { if ((context) && (context)->notifier && ((context)->notifier->mask & CRX_STREAM_NOTIFIER_PROGRESS)) { \
	(context)->notifier->progress += (dsofar); \
	(context)->notifier->progress_max += (dmax); \
	crx_stream_notify_progress((context), (context)->notifier->progress, (context)->notifier->progress_max); } } while (0)

#define crx_stream_notify_file_size(context, file_size, xmsg, xcode) do { if ((context) && (context)->notifier) { \
	crx_stream_notification_notify((context), CRX_STREAM_NOTIFY_FILE_SIZE_IS, CRX_STREAM_NOTIFY_SEVERITY_INFO, \
			(xmsg), (xcode), 0, (file_size), NULL); } } while(0)

#define crx_stream_notify_error(context, code, xmsg, xcode) do { if ((context) && (context)->notifier) {\
	crx_stream_notification_notify((context), (code), CRX_STREAM_NOTIFY_SEVERITY_ERR, \
			(xmsg), (xcode), 0, 0, NULL); } } while(0)
