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
   | Authors: Colin Viebrock <colin@viebrock.ca>                          |
   +----------------------------------------------------------------------+
*/

#include "crx.h"
#include "info.h"

CRXAPI CREX_COLD void crx_info_print_css(void) /* {{{ */
{
	PUTS("body {background-color: #fff; color: #222; font-family: sans-serif;}\n");
	PUTS("pre {margin: 0; font-family: monospace;}\n");
	PUTS("a:link {color: #009; text-decoration: none; background-color: #fff;}\n");
	PUTS("a:hover {text-decoration: underline;}\n");
	PUTS("table {border-collapse: collapse; border: 0; width: 934px; box-shadow: 1px 2px 3px rgba(0, 0, 0, 0.2);}\n");
	PUTS(".center {text-align: center;}\n");
	PUTS(".center table {margin: 1em auto; text-align: left;}\n");
	PUTS(".center th {text-align: center !important;}\n");
	PUTS("td, th {border: 1px solid #666; font-size: 75%; vertical-align: baseline; padding: 4px 5px;}\n");
	PUTS("th {position: sticky; top: 0; background: inherit;}\n");
	PUTS("h1 {font-size: 150%;}\n");
	PUTS("h2 {font-size: 125%;}\n");
	PUTS("h2 a:link, h2 a:visited{color: inherit; background: inherit;}\n");
	PUTS(".p {text-align: left;}\n");
	PUTS(".e {background-color: #ccf; width: 300px; font-weight: bold;}\n");
	PUTS(".h {background-color: #99c; font-weight: bold;}\n");
	PUTS(".v {background-color: #ddd; max-width: 300px; overflow-x: auto; word-wrap: break-word;}\n");
	PUTS(".v i {color: #999;}\n");
	PUTS("img {float: right; border: 0;}\n");
	PUTS("hr {width: 934px; background-color: #ccc; border: 0; height: 1px;}\n");
	PUTS(":root {--crx-dark-grey: #333; --crx-dark-blue: #4F5B93; --crx-medium-blue: #8892BF; --crx-light-blue: #E2E4EF; --crx-accent-purple: #793862}");
	PUTS(
		"@media (prefers-color-scheme: dark) {\n"
		"  body {background: var(--crx-dark-grey); color: var(--crx-light-blue)}\n"
		"  .h td, td.e, th {border-color: #606A90}\n"
		"  td {border-color: #505153}\n"
		"  .e {background-color: #404A77}\n"
		"  .h {background-color: var(--crx-dark-blue)}\n"
		"  .v {background-color: var(--crx-dark-grey)}\n"
		"  hr {background-color: #505153}\n"
		"}\n"
	);
}
/* }}} */
