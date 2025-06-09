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
   | Authors: Rasmus Lerdorf <rasmus@crx.net>                             |
   |          Zeev Suraski <zeev@crx.net>                                 |
   +----------------------------------------------------------------------+
*/

#include "crx.h"
#include "info.h"
#include "SAPI.h"

#define CREDIT_LINE(module, authors) crx_info_print_table_row(2, module, authors)

CRXAPI CREX_COLD void crx_print_credits(int flag) /* {{{ */
{
	if (!sapi_module.crxinfo_as_text && flag & CRX_CREDITS_FULLPAGE) {
		crx_print_info_htmlhead();
	}

	if (!sapi_module.crxinfo_as_text) {
		PUTS("<h1>CRX Credits</h1>\n");
	} else {
		PUTS("CRX Credits\n");
	}

	if (flag & CRX_CREDITS_GROUP) {
		/* Group */

		crx_info_print_table_start();
		crx_info_print_table_header(1, "CRX Group");
		crx_info_print_table_row(1, "Thies C. Arntzen, Stig Bakken, Shane Caraveo, Andi Gutmans, Rasmus Lerdorf, Sam Ruby, Sascha Schumann, Zeev Suraski, Jim Winstead, Andrei Zmievski");
		crx_info_print_table_end();
	}

	if (flag & CRX_CREDITS_GENERAL) {
		/* Design & Concept */
		crx_info_print_table_start();
		if (!sapi_module.crxinfo_as_text) {
			crx_info_print_table_header(1, "Language Design &amp; Concept");
		} else {
			crx_info_print_table_header(1, "Language Design & Concept");
		}
		crx_info_print_table_row(1, "Andi Gutmans, Rasmus Lerdorf, Zeev Suraski, Marcus Boerger");
		crx_info_print_table_end();

		/* CRX Language */
		crx_info_print_table_start();
		crx_info_print_table_colspan_header(2, "CRX Authors");
		crx_info_print_table_header(2, "Contribution", "Authors");
		CREDIT_LINE("Crex Scripting Language Engine", "Andi Gutmans, Zeev Suraski, Stanislav Malyshev, Marcus Boerger, Dmitry Stogov, Xinchen Hui, Nikita Popov");
		CREDIT_LINE("Extension Module API", "Andi Gutmans, Zeev Suraski, Andrei Zmievski");
		CREDIT_LINE("UNIX Build and Modularization", "Stig Bakken, Sascha Schumann, Jani Taskinen, Peter Kokot");
		CREDIT_LINE("Windows Support", "Shane Caraveo, Zeev Suraski, Wez Furlong, Pierre-Alain Joye, Anatol Belski, Kalle Sommer Nielsen");
		CREDIT_LINE("Server API (SAPI) Abstraction Layer", "Andi Gutmans, Shane Caraveo, Zeev Suraski");
		CREDIT_LINE("Streams Abstraction Layer", "Wez Furlong, Sara Golemon");
		CREDIT_LINE("CRX Data Objects Layer", "Wez Furlong, Marcus Boerger, Sterling Hughes, George Schlossnagle, Ilia Alshanetsky");
		CREDIT_LINE("Output Handler", "Zeev Suraski, Thies C. Arntzen, Marcus Boerger, Michael Wallner");
		CREDIT_LINE("Consistent 64 bit support", "Anthony Ferrara, Anatol Belski");
		crx_info_print_table_end();
	}

	if (flag & CRX_CREDITS_SAPI) {
		/* SAPI Modules */

		crx_info_print_table_start();
		crx_info_print_table_colspan_header(2, "SAPI Modules");
		crx_info_print_table_header(2, "Contribution", "Authors");
#include "credits_sapi.h"
		crx_info_print_table_end();
	}

	if (flag & CRX_CREDITS_MODULES) {
		/* Modules */

		crx_info_print_table_start();
		crx_info_print_table_colspan_header(2, "Module Authors");
		crx_info_print_table_header(2, "Module", "Authors");
#include "credits_ext.h"
		crx_info_print_table_end();
	}

	if (flag & CRX_CREDITS_DOCS) {
		crx_info_print_table_start();
		crx_info_print_table_colspan_header(2, "CRX Documentation");
		CREDIT_LINE("Authors", "Mehdi Achour, Friedhelm Betz, Antony Dovgal, Nuno Lopes, Hannes Magnusson, Philip Olson, Georg Richter, Damien Seguy, Jakub Vrana, Adam Harvey");
		CREDIT_LINE("Editor", "Peter Cowburn");
		CREDIT_LINE("User Note Maintainers", "Daniel P. Brown, Thiago Henrique Pojda");
		CREDIT_LINE("Other Contributors", "Previously active authors, editors and other contributors are listed in the manual.");
		crx_info_print_table_end();
	}

	if (flag & CRX_CREDITS_QA) {
		crx_info_print_table_start();
		crx_info_print_table_header(1, "CRX Quality Assurance Team");
		crx_info_print_table_row(1, "Ilia Alshanetsky, Joerg Behrens, Antony Dovgal, Stefan Esser, Moriyoshi Koizumi, Magnus Maatta, Sebastian Nohn, Derick Rethans, Melvyn Sopacua, Pierre-Alain Joye, Dmitry Stogov, Felipe Pena, David Soria Parra, Stanislav Malyshev, Julien Pauli, Stephen Zarkos, Anatol Belski, Remi Collet, Ferenc Kovacs");
		crx_info_print_table_end();
	}

	if (flag & CRX_CREDITS_WEB) {
		/* Websites and infrastructure */

		crx_info_print_table_start();
		crx_info_print_table_colspan_header(2, "Websites and Infrastructure team");
		/* www., wiki., windows., master., and others, I guess pecl. too? */
		CREDIT_LINE("CRX Websites Team", "Rasmus Lerdorf, Hannes Magnusson, Philip Olson, Lukas Kahwe Smith, Pierre-Alain Joye, Kalle Sommer Nielsen, Peter Cowburn, Adam Harvey, Ferenc Kovacs, Levi Morrison");
		CREDIT_LINE("Event Maintainers", "Damien Seguy, Daniel P. Brown");
		/* Mirroring */
		CREDIT_LINE("Network Infrastructure", "Daniel P. Brown");
		/* Windows build boxes and such things */
		CREDIT_LINE("Windows Infrastructure", "Alex Schoenmaker");
		crx_info_print_table_end();
	}

	if (!sapi_module.crxinfo_as_text && flag & CRX_CREDITS_FULLPAGE) {
		PUTS("</div></body></html>\n");
	}
}
/* }}} */
