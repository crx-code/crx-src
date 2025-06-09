/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: cbf308f532d4a28da8a9cde94b726faba9d8c7a4 */

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_IntlBreakIterator_createCharacterInstance, 0, 0, IntlBreakIterator, 1)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, locale, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_IntlBreakIterator_createCodePointInstance, 0, 0, IntlCodePointBreakIterator, 0)
CREX_END_ARG_INFO()

#define arginfo_class_IntlBreakIterator_createLineInstance arginfo_class_IntlBreakIterator_createCharacterInstance

#define arginfo_class_IntlBreakIterator_createSentenceInstance arginfo_class_IntlBreakIterator_createCharacterInstance

#define arginfo_class_IntlBreakIterator_createTitleInstance arginfo_class_IntlBreakIterator_createCharacterInstance

#define arginfo_class_IntlBreakIterator_createWordInstance arginfo_class_IntlBreakIterator_createCharacterInstance

CREX_BEGIN_ARG_INFO_EX(arginfo_class_IntlBreakIterator___main, 0, 0, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlBreakIterator_current, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_class_IntlBreakIterator_first arginfo_class_IntlBreakIterator_current

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlBreakIterator_following, 0, 1, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_class_IntlBreakIterator_getErrorCode arginfo_class_IntlBreakIterator_current

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlBreakIterator_getErrorMessage, 0, 0, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_IntlBreakIterator_getLocale, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, type, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_IntlBreakIterator_getPartsIterator, 0, 0, IntlPartsIterator, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, type, IS_STRING, 0, "IntlPartsIterator::KEY_SEQUENTIAL")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlBreakIterator_getText, 0, 0, IS_STRING, 1)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlBreakIterator_isBoundary, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_class_IntlBreakIterator_last arginfo_class_IntlBreakIterator_current

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlBreakIterator_next, 0, 0, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, offset, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_class_IntlBreakIterator_preceding arginfo_class_IntlBreakIterator_following

#define arginfo_class_IntlBreakIterator_previous arginfo_class_IntlBreakIterator_current

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlBreakIterator_setText, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, text, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_IntlBreakIterator_getIterator, 0, 0, Iterator, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_IntlRuleBasedBreakIterator___main, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, rules, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, compiled, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_IntlRuleBasedBreakIterator_getBinaryRules, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
CREX_END_ARG_INFO()

#define arginfo_class_IntlRuleBasedBreakIterator_getRules arginfo_class_IntlRuleBasedBreakIterator_getBinaryRules

#define arginfo_class_IntlRuleBasedBreakIterator_getRuleStatus arginfo_class_IntlBreakIterator_current

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_IntlRuleBasedBreakIterator_getRuleStatusVec, 0, 0, MAY_BE_ARRAY|MAY_BE_FALSE)
CREX_END_ARG_INFO()

#define arginfo_class_IntlCodePointBreakIterator_getLastCodePoint arginfo_class_IntlBreakIterator_current


CREX_METHOD(IntlBreakIterator, createCharacterInstance);
CREX_METHOD(IntlBreakIterator, createCodePointInstance);
CREX_METHOD(IntlBreakIterator, createLineInstance);
CREX_METHOD(IntlBreakIterator, createSentenceInstance);
CREX_METHOD(IntlBreakIterator, createTitleInstance);
CREX_METHOD(IntlBreakIterator, createWordInstance);
CREX_METHOD(IntlBreakIterator, __main);
CREX_METHOD(IntlBreakIterator, current);
CREX_METHOD(IntlBreakIterator, first);
CREX_METHOD(IntlBreakIterator, following);
CREX_METHOD(IntlBreakIterator, getErrorCode);
CREX_METHOD(IntlBreakIterator, getErrorMessage);
CREX_METHOD(IntlBreakIterator, getLocale);
CREX_METHOD(IntlBreakIterator, getPartsIterator);
CREX_METHOD(IntlBreakIterator, getText);
CREX_METHOD(IntlBreakIterator, isBoundary);
CREX_METHOD(IntlBreakIterator, last);
CREX_METHOD(IntlBreakIterator, next);
CREX_METHOD(IntlBreakIterator, preceding);
CREX_METHOD(IntlBreakIterator, previous);
CREX_METHOD(IntlBreakIterator, setText);
CREX_METHOD(IntlBreakIterator, getIterator);
CREX_METHOD(IntlRuleBasedBreakIterator, __main);
CREX_METHOD(IntlRuleBasedBreakIterator, getBinaryRules);
CREX_METHOD(IntlRuleBasedBreakIterator, getRules);
CREX_METHOD(IntlRuleBasedBreakIterator, getRuleStatus);
CREX_METHOD(IntlRuleBasedBreakIterator, getRuleStatusVec);
CREX_METHOD(IntlCodePointBreakIterator, getLastCodePoint);


static const crex_function_entry class_IntlBreakIterator_methods[] = {
	CREX_ME(IntlBreakIterator, createCharacterInstance, arginfo_class_IntlBreakIterator_createCharacterInstance, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlBreakIterator, createCodePointInstance, arginfo_class_IntlBreakIterator_createCodePointInstance, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlBreakIterator, createLineInstance, arginfo_class_IntlBreakIterator_createLineInstance, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlBreakIterator, createSentenceInstance, arginfo_class_IntlBreakIterator_createSentenceInstance, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlBreakIterator, createTitleInstance, arginfo_class_IntlBreakIterator_createTitleInstance, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlBreakIterator, createWordInstance, arginfo_class_IntlBreakIterator_createWordInstance, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlBreakIterator, __main, arginfo_class_IntlBreakIterator___main, CREX_ACC_PRIVATE)
	CREX_ME(IntlBreakIterator, current, arginfo_class_IntlBreakIterator_current, CREX_ACC_PUBLIC)
	CREX_ME(IntlBreakIterator, first, arginfo_class_IntlBreakIterator_first, CREX_ACC_PUBLIC)
	CREX_ME(IntlBreakIterator, following, arginfo_class_IntlBreakIterator_following, CREX_ACC_PUBLIC)
	CREX_ME(IntlBreakIterator, getErrorCode, arginfo_class_IntlBreakIterator_getErrorCode, CREX_ACC_PUBLIC)
	CREX_ME(IntlBreakIterator, getErrorMessage, arginfo_class_IntlBreakIterator_getErrorMessage, CREX_ACC_PUBLIC)
	CREX_ME(IntlBreakIterator, getLocale, arginfo_class_IntlBreakIterator_getLocale, CREX_ACC_PUBLIC)
	CREX_ME(IntlBreakIterator, getPartsIterator, arginfo_class_IntlBreakIterator_getPartsIterator, CREX_ACC_PUBLIC)
	CREX_ME(IntlBreakIterator, getText, arginfo_class_IntlBreakIterator_getText, CREX_ACC_PUBLIC)
	CREX_ME(IntlBreakIterator, isBoundary, arginfo_class_IntlBreakIterator_isBoundary, CREX_ACC_PUBLIC)
	CREX_ME(IntlBreakIterator, last, arginfo_class_IntlBreakIterator_last, CREX_ACC_PUBLIC)
	CREX_ME(IntlBreakIterator, next, arginfo_class_IntlBreakIterator_next, CREX_ACC_PUBLIC)
	CREX_ME(IntlBreakIterator, preceding, arginfo_class_IntlBreakIterator_preceding, CREX_ACC_PUBLIC)
	CREX_ME(IntlBreakIterator, previous, arginfo_class_IntlBreakIterator_previous, CREX_ACC_PUBLIC)
	CREX_ME(IntlBreakIterator, setText, arginfo_class_IntlBreakIterator_setText, CREX_ACC_PUBLIC)
	CREX_ME(IntlBreakIterator, getIterator, arginfo_class_IntlBreakIterator_getIterator, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_IntlRuleBasedBreakIterator_methods[] = {
	CREX_ME(IntlRuleBasedBreakIterator, __main, arginfo_class_IntlRuleBasedBreakIterator___main, CREX_ACC_PUBLIC)
	CREX_ME(IntlRuleBasedBreakIterator, getBinaryRules, arginfo_class_IntlRuleBasedBreakIterator_getBinaryRules, CREX_ACC_PUBLIC)
	CREX_ME(IntlRuleBasedBreakIterator, getRules, arginfo_class_IntlRuleBasedBreakIterator_getRules, CREX_ACC_PUBLIC)
	CREX_ME(IntlRuleBasedBreakIterator, getRuleStatus, arginfo_class_IntlRuleBasedBreakIterator_getRuleStatus, CREX_ACC_PUBLIC)
	CREX_ME(IntlRuleBasedBreakIterator, getRuleStatusVec, arginfo_class_IntlRuleBasedBreakIterator_getRuleStatusVec, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_IntlCodePointBreakIterator_methods[] = {
	CREX_ME(IntlCodePointBreakIterator, getLastCodePoint, arginfo_class_IntlCodePointBreakIterator_getLastCodePoint, CREX_ACC_PUBLIC)
	CREX_FE_END
};

static crex_class_entry *register_class_IntlBreakIterator(crex_class_entry *class_entry_IteratorAggregate)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "IntlBreakIterator", class_IntlBreakIterator_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_NOT_SERIALIZABLE;
	crex_class_implements(class_entry, 1, class_entry_IteratorAggregate);

	zval const_DONE_value;
	ZVAL_LONG(&const_DONE_value, BreakIterator::DONE);
	crex_string *const_DONE_name = crex_string_init_interned("DONE", sizeof("DONE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DONE_name, &const_DONE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DONE_name);

	zval const_WORD_NONE_value;
	ZVAL_LONG(&const_WORD_NONE_value, UBRK_WORD_NONE);
	crex_string *const_WORD_NONE_name = crex_string_init_interned("WORD_NONE", sizeof("WORD_NONE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_WORD_NONE_name, &const_WORD_NONE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_WORD_NONE_name);

	zval const_WORD_NONE_LIMIT_value;
	ZVAL_LONG(&const_WORD_NONE_LIMIT_value, UBRK_WORD_NONE_LIMIT);
	crex_string *const_WORD_NONE_LIMIT_name = crex_string_init_interned("WORD_NONE_LIMIT", sizeof("WORD_NONE_LIMIT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_WORD_NONE_LIMIT_name, &const_WORD_NONE_LIMIT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_WORD_NONE_LIMIT_name);

	zval const_WORD_NUMBER_value;
	ZVAL_LONG(&const_WORD_NUMBER_value, UBRK_WORD_NUMBER);
	crex_string *const_WORD_NUMBER_name = crex_string_init_interned("WORD_NUMBER", sizeof("WORD_NUMBER") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_WORD_NUMBER_name, &const_WORD_NUMBER_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_WORD_NUMBER_name);

	zval const_WORD_NUMBER_LIMIT_value;
	ZVAL_LONG(&const_WORD_NUMBER_LIMIT_value, UBRK_WORD_NUMBER_LIMIT);
	crex_string *const_WORD_NUMBER_LIMIT_name = crex_string_init_interned("WORD_NUMBER_LIMIT", sizeof("WORD_NUMBER_LIMIT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_WORD_NUMBER_LIMIT_name, &const_WORD_NUMBER_LIMIT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_WORD_NUMBER_LIMIT_name);

	zval const_WORD_LETTER_value;
	ZVAL_LONG(&const_WORD_LETTER_value, UBRK_WORD_LETTER);
	crex_string *const_WORD_LETTER_name = crex_string_init_interned("WORD_LETTER", sizeof("WORD_LETTER") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_WORD_LETTER_name, &const_WORD_LETTER_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_WORD_LETTER_name);

	zval const_WORD_LETTER_LIMIT_value;
	ZVAL_LONG(&const_WORD_LETTER_LIMIT_value, UBRK_WORD_LETTER_LIMIT);
	crex_string *const_WORD_LETTER_LIMIT_name = crex_string_init_interned("WORD_LETTER_LIMIT", sizeof("WORD_LETTER_LIMIT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_WORD_LETTER_LIMIT_name, &const_WORD_LETTER_LIMIT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_WORD_LETTER_LIMIT_name);

	zval const_WORD_KANA_value;
	ZVAL_LONG(&const_WORD_KANA_value, UBRK_WORD_KANA);
	crex_string *const_WORD_KANA_name = crex_string_init_interned("WORD_KANA", sizeof("WORD_KANA") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_WORD_KANA_name, &const_WORD_KANA_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_WORD_KANA_name);

	zval const_WORD_KANA_LIMIT_value;
	ZVAL_LONG(&const_WORD_KANA_LIMIT_value, UBRK_WORD_KANA_LIMIT);
	crex_string *const_WORD_KANA_LIMIT_name = crex_string_init_interned("WORD_KANA_LIMIT", sizeof("WORD_KANA_LIMIT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_WORD_KANA_LIMIT_name, &const_WORD_KANA_LIMIT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_WORD_KANA_LIMIT_name);

	zval const_WORD_IDEO_value;
	ZVAL_LONG(&const_WORD_IDEO_value, UBRK_WORD_IDEO);
	crex_string *const_WORD_IDEO_name = crex_string_init_interned("WORD_IDEO", sizeof("WORD_IDEO") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_WORD_IDEO_name, &const_WORD_IDEO_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_WORD_IDEO_name);

	zval const_WORD_IDEO_LIMIT_value;
	ZVAL_LONG(&const_WORD_IDEO_LIMIT_value, UBRK_WORD_IDEO_LIMIT);
	crex_string *const_WORD_IDEO_LIMIT_name = crex_string_init_interned("WORD_IDEO_LIMIT", sizeof("WORD_IDEO_LIMIT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_WORD_IDEO_LIMIT_name, &const_WORD_IDEO_LIMIT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_WORD_IDEO_LIMIT_name);

	zval const_LINE_SOFT_value;
	ZVAL_LONG(&const_LINE_SOFT_value, UBRK_LINE_SOFT);
	crex_string *const_LINE_SOFT_name = crex_string_init_interned("LINE_SOFT", sizeof("LINE_SOFT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LINE_SOFT_name, &const_LINE_SOFT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LINE_SOFT_name);

	zval const_LINE_SOFT_LIMIT_value;
	ZVAL_LONG(&const_LINE_SOFT_LIMIT_value, UBRK_LINE_SOFT_LIMIT);
	crex_string *const_LINE_SOFT_LIMIT_name = crex_string_init_interned("LINE_SOFT_LIMIT", sizeof("LINE_SOFT_LIMIT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LINE_SOFT_LIMIT_name, &const_LINE_SOFT_LIMIT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LINE_SOFT_LIMIT_name);

	zval const_LINE_HARD_value;
	ZVAL_LONG(&const_LINE_HARD_value, UBRK_LINE_HARD);
	crex_string *const_LINE_HARD_name = crex_string_init_interned("LINE_HARD", sizeof("LINE_HARD") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LINE_HARD_name, &const_LINE_HARD_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LINE_HARD_name);

	zval const_LINE_HARD_LIMIT_value;
	ZVAL_LONG(&const_LINE_HARD_LIMIT_value, UBRK_LINE_HARD_LIMIT);
	crex_string *const_LINE_HARD_LIMIT_name = crex_string_init_interned("LINE_HARD_LIMIT", sizeof("LINE_HARD_LIMIT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LINE_HARD_LIMIT_name, &const_LINE_HARD_LIMIT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LINE_HARD_LIMIT_name);

	zval const_SENTENCE_TERM_value;
	ZVAL_LONG(&const_SENTENCE_TERM_value, UBRK_SENTENCE_TERM);
	crex_string *const_SENTENCE_TERM_name = crex_string_init_interned("SENTENCE_TERM", sizeof("SENTENCE_TERM") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SENTENCE_TERM_name, &const_SENTENCE_TERM_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SENTENCE_TERM_name);

	zval const_SENTENCE_TERM_LIMIT_value;
	ZVAL_LONG(&const_SENTENCE_TERM_LIMIT_value, UBRK_SENTENCE_TERM_LIMIT);
	crex_string *const_SENTENCE_TERM_LIMIT_name = crex_string_init_interned("SENTENCE_TERM_LIMIT", sizeof("SENTENCE_TERM_LIMIT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SENTENCE_TERM_LIMIT_name, &const_SENTENCE_TERM_LIMIT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SENTENCE_TERM_LIMIT_name);

	zval const_SENTENCE_SEP_value;
	ZVAL_LONG(&const_SENTENCE_SEP_value, UBRK_SENTENCE_SEP);
	crex_string *const_SENTENCE_SEP_name = crex_string_init_interned("SENTENCE_SEP", sizeof("SENTENCE_SEP") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SENTENCE_SEP_name, &const_SENTENCE_SEP_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SENTENCE_SEP_name);

	zval const_SENTENCE_SEP_LIMIT_value;
	ZVAL_LONG(&const_SENTENCE_SEP_LIMIT_value, UBRK_SENTENCE_SEP_LIMIT);
	crex_string *const_SENTENCE_SEP_LIMIT_name = crex_string_init_interned("SENTENCE_SEP_LIMIT", sizeof("SENTENCE_SEP_LIMIT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SENTENCE_SEP_LIMIT_name, &const_SENTENCE_SEP_LIMIT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SENTENCE_SEP_LIMIT_name);

	return class_entry;
}

static crex_class_entry *register_class_IntlRuleBasedBreakIterator(crex_class_entry *class_entry_IntlBreakIterator)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "IntlRuleBasedBreakIterator", class_IntlRuleBasedBreakIterator_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_IntlBreakIterator);
	class_entry->ce_flags |= CREX_ACC_NOT_SERIALIZABLE;

	return class_entry;
}

static crex_class_entry *register_class_IntlCodePointBreakIterator(crex_class_entry *class_entry_IntlBreakIterator)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "IntlCodePointBreakIterator", class_IntlCodePointBreakIterator_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_IntlBreakIterator);
	class_entry->ce_flags |= CREX_ACC_NOT_SERIALIZABLE;

	return class_entry;
}
