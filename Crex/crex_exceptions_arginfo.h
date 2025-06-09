/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 4cf2c620393f468968a219b5bd12a2b5f6b03ecc */

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Throwable_getMessage, 0, 0, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_Throwable_getCode, 0, 0, 0)
CREX_END_ARG_INFO()

#define arginfo_class_Throwable_getFile arginfo_class_Throwable_getMessage

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Throwable_getLine, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Throwable_getTrace, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Throwable_getPrevious, 0, 0, Throwable, 1)
CREX_END_ARG_INFO()

#define arginfo_class_Throwable_getTraceAsString arginfo_class_Throwable_getMessage

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Exception___clone, 0, 0, IS_VOID, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_Exception___main, 0, 0, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, message, IS_STRING, 0, "\"\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, code, IS_LONG, 0, "0")
	CREX_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, previous, Throwable, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_Exception___wakeup, 0, 0, IS_VOID, 0)
CREX_END_ARG_INFO()

#define arginfo_class_Exception_getMessage arginfo_class_Throwable_getMessage

#define arginfo_class_Exception_getCode arginfo_class_Throwable_getCode

#define arginfo_class_Exception_getFile arginfo_class_Throwable_getMessage

#define arginfo_class_Exception_getLine arginfo_class_Throwable_getLine

#define arginfo_class_Exception_getTrace arginfo_class_Throwable_getTrace

#define arginfo_class_Exception_getPrevious arginfo_class_Throwable_getPrevious

#define arginfo_class_Exception_getTraceAsString arginfo_class_Throwable_getMessage

#define arginfo_class_Exception___toString arginfo_class_Throwable_getMessage

CREX_BEGIN_ARG_INFO_EX(arginfo_class_ErrorException___main, 0, 0, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, message, IS_STRING, 0, "\"\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, code, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, severity, IS_LONG, 0, "E_ERROR")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, filename, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, line, IS_LONG, 1, "null")
	CREX_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, previous, Throwable, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_class_ErrorException_getSeverity arginfo_class_Throwable_getLine

#define arginfo_class_Error___clone arginfo_class_Exception___clone

#define arginfo_class_Error___main arginfo_class_Exception___main

#define arginfo_class_Error___wakeup arginfo_class_Exception___wakeup

#define arginfo_class_Error_getMessage arginfo_class_Throwable_getMessage

#define arginfo_class_Error_getCode arginfo_class_Throwable_getCode

#define arginfo_class_Error_getFile arginfo_class_Throwable_getMessage

#define arginfo_class_Error_getLine arginfo_class_Throwable_getLine

#define arginfo_class_Error_getTrace arginfo_class_Throwable_getTrace

#define arginfo_class_Error_getPrevious arginfo_class_Throwable_getPrevious

#define arginfo_class_Error_getTraceAsString arginfo_class_Throwable_getMessage

#define arginfo_class_Error___toString arginfo_class_Throwable_getMessage


CREX_METHOD(Exception, __clone);
CREX_METHOD(Exception, __main);
CREX_METHOD(Exception, __wakeup);
CREX_METHOD(Exception, getMessage);
CREX_METHOD(Exception, getCode);
CREX_METHOD(Exception, getFile);
CREX_METHOD(Exception, getLine);
CREX_METHOD(Exception, getTrace);
CREX_METHOD(Exception, getPrevious);
CREX_METHOD(Exception, getTraceAsString);
CREX_METHOD(Exception, __toString);
CREX_METHOD(ErrorException, __main);
CREX_METHOD(ErrorException, getSeverity);


static const crex_function_entry class_Throwable_methods[] = {
	CREX_ABSTRACT_ME_WITH_FLAGS(Throwable, getMessage, arginfo_class_Throwable_getMessage, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_ABSTRACT_ME_WITH_FLAGS(Throwable, getCode, arginfo_class_Throwable_getCode, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_ABSTRACT_ME_WITH_FLAGS(Throwable, getFile, arginfo_class_Throwable_getFile, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_ABSTRACT_ME_WITH_FLAGS(Throwable, getLine, arginfo_class_Throwable_getLine, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_ABSTRACT_ME_WITH_FLAGS(Throwable, getTrace, arginfo_class_Throwable_getTrace, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_ABSTRACT_ME_WITH_FLAGS(Throwable, getPrevious, arginfo_class_Throwable_getPrevious, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_ABSTRACT_ME_WITH_FLAGS(Throwable, getTraceAsString, arginfo_class_Throwable_getTraceAsString, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_FE_END
};


static const crex_function_entry class_Exception_methods[] = {
	CREX_ME(Exception, __clone, arginfo_class_Exception___clone, CREX_ACC_PRIVATE)
	CREX_ME(Exception, __main, arginfo_class_Exception___main, CREX_ACC_PUBLIC)
	CREX_ME(Exception, __wakeup, arginfo_class_Exception___wakeup, CREX_ACC_PUBLIC)
	CREX_ME(Exception, getMessage, arginfo_class_Exception_getMessage, CREX_ACC_PUBLIC|CREX_ACC_FINAL)
	CREX_ME(Exception, getCode, arginfo_class_Exception_getCode, CREX_ACC_PUBLIC|CREX_ACC_FINAL)
	CREX_ME(Exception, getFile, arginfo_class_Exception_getFile, CREX_ACC_PUBLIC|CREX_ACC_FINAL)
	CREX_ME(Exception, getLine, arginfo_class_Exception_getLine, CREX_ACC_PUBLIC|CREX_ACC_FINAL)
	CREX_ME(Exception, getTrace, arginfo_class_Exception_getTrace, CREX_ACC_PUBLIC|CREX_ACC_FINAL)
	CREX_ME(Exception, getPrevious, arginfo_class_Exception_getPrevious, CREX_ACC_PUBLIC|CREX_ACC_FINAL)
	CREX_ME(Exception, getTraceAsString, arginfo_class_Exception_getTraceAsString, CREX_ACC_PUBLIC|CREX_ACC_FINAL)
	CREX_ME(Exception, __toString, arginfo_class_Exception___toString, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_ErrorException_methods[] = {
	CREX_ME(ErrorException, __main, arginfo_class_ErrorException___main, CREX_ACC_PUBLIC)
	CREX_ME(ErrorException, getSeverity, arginfo_class_ErrorException_getSeverity, CREX_ACC_PUBLIC|CREX_ACC_FINAL)
	CREX_FE_END
};


static const crex_function_entry class_Error_methods[] = {
	CREX_MALIAS(Exception, __clone, __clone, arginfo_class_Error___clone, CREX_ACC_PRIVATE)
	CREX_MALIAS(Exception, __main, __main, arginfo_class_Error___main, CREX_ACC_PUBLIC)
	CREX_MALIAS(Exception, __wakeup, __wakeup, arginfo_class_Error___wakeup, CREX_ACC_PUBLIC)
	CREX_MALIAS(Exception, getMessage, getMessage, arginfo_class_Error_getMessage, CREX_ACC_PUBLIC|CREX_ACC_FINAL)
	CREX_MALIAS(Exception, getCode, getCode, arginfo_class_Error_getCode, CREX_ACC_PUBLIC|CREX_ACC_FINAL)
	CREX_MALIAS(Exception, getFile, getFile, arginfo_class_Error_getFile, CREX_ACC_PUBLIC|CREX_ACC_FINAL)
	CREX_MALIAS(Exception, getLine, getLine, arginfo_class_Error_getLine, CREX_ACC_PUBLIC|CREX_ACC_FINAL)
	CREX_MALIAS(Exception, getTrace, getTrace, arginfo_class_Error_getTrace, CREX_ACC_PUBLIC|CREX_ACC_FINAL)
	CREX_MALIAS(Exception, getPrevious, getPrevious, arginfo_class_Error_getPrevious, CREX_ACC_PUBLIC|CREX_ACC_FINAL)
	CREX_MALIAS(Exception, getTraceAsString, getTraceAsString, arginfo_class_Error_getTraceAsString, CREX_ACC_PUBLIC|CREX_ACC_FINAL)
	CREX_MALIAS(Exception, __toString, __toString, arginfo_class_Error___toString, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_CompileError_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_ParseError_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_TypeError_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_ArgumentCountError_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_ValueError_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_ArithmeticError_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_DivisionByZeroError_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_UnhandledMatchError_methods[] = {
	CREX_FE_END
};

static crex_class_entry *register_class_Throwable(crex_class_entry *class_entry_Stringable)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "Throwable", class_Throwable_methods);
	class_entry = crex_register_internal_interface(&ce);
	crex_class_implements(class_entry, 1, class_entry_Stringable);

	return class_entry;
}

static crex_class_entry *register_class_Exception(crex_class_entry *class_entry_Throwable)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "Exception", class_Exception_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	crex_class_implements(class_entry, 1, class_entry_Throwable);

	zval property_message_default_value;
	ZVAL_EMPTY_STRING(&property_message_default_value);
	crex_string *property_message_name = crex_string_init("message", sizeof("message") - 1, 1);
	crex_declare_typed_property(class_entry, property_message_name, &property_message_default_value, CREX_ACC_PROTECTED, NULL, (crex_type) CREX_TYPE_INIT_NONE(0));
	crex_string_release(property_message_name);

	zval property_string_default_value;
	ZVAL_EMPTY_STRING(&property_string_default_value);
	crex_string *property_string_name = crex_string_init("string", sizeof("string") - 1, 1);
	crex_declare_typed_property(class_entry, property_string_name, &property_string_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_string_name);

	zval property_code_default_value;
	ZVAL_LONG(&property_code_default_value, 0);
	crex_string *property_code_name = crex_string_init("code", sizeof("code") - 1, 1);
	crex_declare_typed_property(class_entry, property_code_name, &property_code_default_value, CREX_ACC_PROTECTED, NULL, (crex_type) CREX_TYPE_INIT_NONE(0));
	crex_string_release(property_code_name);

	zval property_file_default_value;
	ZVAL_EMPTY_STRING(&property_file_default_value);
	crex_string *property_file_name = crex_string_init("file", sizeof("file") - 1, 1);
	crex_declare_typed_property(class_entry, property_file_name, &property_file_default_value, CREX_ACC_PROTECTED, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_file_name);

	zval property_line_default_value;
	ZVAL_LONG(&property_line_default_value, 0);
	crex_string *property_line_name = crex_string_init("line", sizeof("line") - 1, 1);
	crex_declare_typed_property(class_entry, property_line_name, &property_line_default_value, CREX_ACC_PROTECTED, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(property_line_name);

	zval property_trace_default_value;
	ZVAL_EMPTY_ARRAY(&property_trace_default_value);
	crex_string *property_trace_name = crex_string_init("trace", sizeof("trace") - 1, 1);
	crex_declare_typed_property(class_entry, property_trace_name, &property_trace_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_ARRAY));
	crex_string_release(property_trace_name);

	zval property_previous_default_value;
	ZVAL_NULL(&property_previous_default_value);
	crex_string *property_previous_name = crex_string_init("previous", sizeof("previous") - 1, 1);
	crex_string *property_previous_class_Throwable = crex_string_init("Throwable", sizeof("Throwable")-1, 1);
	crex_declare_typed_property(class_entry, property_previous_name, &property_previous_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property_previous_class_Throwable, 0, MAY_BE_NULL));
	crex_string_release(property_previous_name);

	return class_entry;
}

static crex_class_entry *register_class_ErrorException(crex_class_entry *class_entry_Exception)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "ErrorException", class_ErrorException_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_Exception);

	zval property_severity_default_value;
	ZVAL_LONG(&property_severity_default_value, E_ERROR);
	crex_string *property_severity_name = crex_string_init("severity", sizeof("severity") - 1, 1);
	crex_declare_typed_property(class_entry, property_severity_name, &property_severity_default_value, CREX_ACC_PROTECTED, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(property_severity_name);

	return class_entry;
}

static crex_class_entry *register_class_Error(crex_class_entry *class_entry_Throwable)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "Error", class_Error_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	crex_class_implements(class_entry, 1, class_entry_Throwable);

	zval property_message_default_value;
	ZVAL_EMPTY_STRING(&property_message_default_value);
	crex_string *property_message_name = crex_string_init("message", sizeof("message") - 1, 1);
	crex_declare_typed_property(class_entry, property_message_name, &property_message_default_value, CREX_ACC_PROTECTED, NULL, (crex_type) CREX_TYPE_INIT_NONE(0));
	crex_string_release(property_message_name);

	zval property_string_default_value;
	ZVAL_EMPTY_STRING(&property_string_default_value);
	crex_string *property_string_name = crex_string_init("string", sizeof("string") - 1, 1);
	crex_declare_typed_property(class_entry, property_string_name, &property_string_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_string_name);

	zval property_code_default_value;
	ZVAL_LONG(&property_code_default_value, 0);
	crex_string *property_code_name = crex_string_init("code", sizeof("code") - 1, 1);
	crex_declare_typed_property(class_entry, property_code_name, &property_code_default_value, CREX_ACC_PROTECTED, NULL, (crex_type) CREX_TYPE_INIT_NONE(0));
	crex_string_release(property_code_name);

	zval property_file_default_value;
	ZVAL_EMPTY_STRING(&property_file_default_value);
	crex_string *property_file_name = crex_string_init("file", sizeof("file") - 1, 1);
	crex_declare_typed_property(class_entry, property_file_name, &property_file_default_value, CREX_ACC_PROTECTED, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_file_name);

	zval property_line_default_value;
	ZVAL_UNDEF(&property_line_default_value);
	crex_string *property_line_name = crex_string_init("line", sizeof("line") - 1, 1);
	crex_declare_typed_property(class_entry, property_line_name, &property_line_default_value, CREX_ACC_PROTECTED, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(property_line_name);

	zval property_trace_default_value;
	ZVAL_EMPTY_ARRAY(&property_trace_default_value);
	crex_string *property_trace_name = crex_string_init("trace", sizeof("trace") - 1, 1);
	crex_declare_typed_property(class_entry, property_trace_name, &property_trace_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_ARRAY));
	crex_string_release(property_trace_name);

	zval property_previous_default_value;
	ZVAL_NULL(&property_previous_default_value);
	crex_string *property_previous_name = crex_string_init("previous", sizeof("previous") - 1, 1);
	crex_string *property_previous_class_Throwable = crex_string_init("Throwable", sizeof("Throwable")-1, 1);
	crex_declare_typed_property(class_entry, property_previous_name, &property_previous_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property_previous_class_Throwable, 0, MAY_BE_NULL));
	crex_string_release(property_previous_name);

	return class_entry;
}

static crex_class_entry *register_class_CompileError(crex_class_entry *class_entry_Error)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "CompileError", class_CompileError_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_Error);

	return class_entry;
}

static crex_class_entry *register_class_ParseError(crex_class_entry *class_entry_CompileError)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "ParseError", class_ParseError_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_CompileError);

	return class_entry;
}

static crex_class_entry *register_class_TypeError(crex_class_entry *class_entry_Error)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "TypeError", class_TypeError_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_Error);

	return class_entry;
}

static crex_class_entry *register_class_ArgumentCountError(crex_class_entry *class_entry_TypeError)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "ArgumentCountError", class_ArgumentCountError_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_TypeError);

	return class_entry;
}

static crex_class_entry *register_class_ValueError(crex_class_entry *class_entry_Error)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "ValueError", class_ValueError_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_Error);

	return class_entry;
}

static crex_class_entry *register_class_ArithmeticError(crex_class_entry *class_entry_Error)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "ArithmeticError", class_ArithmeticError_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_Error);

	return class_entry;
}

static crex_class_entry *register_class_DivisionByZeroError(crex_class_entry *class_entry_ArithmeticError)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DivisionByZeroError", class_DivisionByZeroError_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_ArithmeticError);

	return class_entry;
}

static crex_class_entry *register_class_UnhandledMatchError(crex_class_entry *class_entry_Error)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "UnhandledMatchError", class_UnhandledMatchError_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_Error);

	return class_entry;
}
