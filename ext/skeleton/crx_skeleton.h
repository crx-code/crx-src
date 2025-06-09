%HEADER%

#ifndef CRX_%EXTNAMECAPS%_H
# define CRX_%EXTNAMECAPS%_H

extern crex_module_entry %EXTNAME%_module_entry;
# define crxext_%EXTNAME%_ptr &%EXTNAME%_module_entry

# define CRX_%EXTNAMECAPS%_VERSION "0.1.0"

# if defined(ZTS) && defined(COMPILE_DL_%EXTNAMECAPS%)
CREX_TSRMLS_CACHE_EXTERN()
# endif

#endif	/* CRX_%EXTNAMECAPS%_H */
