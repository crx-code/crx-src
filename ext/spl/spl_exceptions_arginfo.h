/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 07475caecc81ab3b38a04905f874615af1126289 */




static const crex_function_entry class_LogicException_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_BadFunctionCallException_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_BadMethodCallException_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_DomainException_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_InvalidArgumentException_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_LengthException_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_OutOfRangeException_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_RuntimeException_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_OutOfBoundsException_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_OverflowException_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_RangeException_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_UnderflowException_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_UnexpectedValueException_methods[] = {
	CREX_FE_END
};

static crex_class_entry *register_class_LogicException(crex_class_entry *class_entry_Exception)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "LogicException", class_LogicException_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_Exception);

	return class_entry;
}

static crex_class_entry *register_class_BadFunctionCallException(crex_class_entry *class_entry_LogicException)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "BadFunctionCallException", class_BadFunctionCallException_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_LogicException);

	return class_entry;
}

static crex_class_entry *register_class_BadMethodCallException(crex_class_entry *class_entry_BadFunctionCallException)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "BadMethodCallException", class_BadMethodCallException_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_BadFunctionCallException);

	return class_entry;
}

static crex_class_entry *register_class_DomainException(crex_class_entry *class_entry_LogicException)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DomainException", class_DomainException_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_LogicException);

	return class_entry;
}

static crex_class_entry *register_class_InvalidArgumentException(crex_class_entry *class_entry_LogicException)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "InvalidArgumentException", class_InvalidArgumentException_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_LogicException);

	return class_entry;
}

static crex_class_entry *register_class_LengthException(crex_class_entry *class_entry_LogicException)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "LengthException", class_LengthException_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_LogicException);

	return class_entry;
}

static crex_class_entry *register_class_OutOfRangeException(crex_class_entry *class_entry_LogicException)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "OutOfRangeException", class_OutOfRangeException_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_LogicException);

	return class_entry;
}

static crex_class_entry *register_class_RuntimeException(crex_class_entry *class_entry_Exception)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "RuntimeException", class_RuntimeException_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_Exception);

	return class_entry;
}

static crex_class_entry *register_class_OutOfBoundsException(crex_class_entry *class_entry_RuntimeException)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "OutOfBoundsException", class_OutOfBoundsException_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_RuntimeException);

	return class_entry;
}

static crex_class_entry *register_class_OverflowException(crex_class_entry *class_entry_RuntimeException)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "OverflowException", class_OverflowException_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_RuntimeException);

	return class_entry;
}

static crex_class_entry *register_class_RangeException(crex_class_entry *class_entry_RuntimeException)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "RangeException", class_RangeException_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_RuntimeException);

	return class_entry;
}

static crex_class_entry *register_class_UnderflowException(crex_class_entry *class_entry_RuntimeException)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "UnderflowException", class_UnderflowException_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_RuntimeException);

	return class_entry;
}

static crex_class_entry *register_class_UnexpectedValueException(crex_class_entry *class_entry_RuntimeException)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "UnexpectedValueException", class_UnexpectedValueException_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_RuntimeException);

	return class_entry;
}
