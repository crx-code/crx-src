#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#include "crx_config.h"
#endif

#include "gd_compat.h"
#include "crx.h"

int overflow2(int a, int b)
{

	if(a <= 0 || b <= 0) {
		crx_error_docref(NULL, E_WARNING, "One parameter to a memory allocation multiplication is negative or zero, failing operation gracefully\n");
		return 1;
	}
	if(a > INT_MAX / b) {
		crx_error_docref(NULL, E_WARNING, "Product of memory allocation multiplication would exceed INT_MAX, failing operation gracefully\n");
		return 1;
	}
	return 0;
}
