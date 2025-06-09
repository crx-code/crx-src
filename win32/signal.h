#ifndef CRX_WIN32_SIGNAL_H
#define CRX_WIN32_SIGNAL_H

#include <signal.h>

#include "win32/winutil.h"

#define SIGALRM 13
#define	SIGVTALRM 26			/* virtual time alarm */
#define	SIGPROF	27				/* profiling time alarm */

CRX_WINUTIL_API void crx_win32_signal_ctrl_handler_init(void);
CRX_WINUTIL_API void crx_win32_signal_ctrl_handler_shutdown(void);

#endif /* CRX_WIN32_SIGNAL_H */
