<?crx

function get_cgi_path() /* {{{ */
{
    $crx = getenv("TEST_CRX_EXECUTABLE");
    $crx_escaped = getenv("TEST_CRX_EXECUTABLE_ESCAPED");

    $cli = false;
    $cgi = false;

    if (file_exists($crx) && is_executable($crx)) {
        $version = `$crx_escaped -n -v`;
        if (strstr($version, "(cli)")) {
            /* that's cli */
            $cli = true;
        } else if (strpos($version, "(cgi")) {
            /* that's cgi */
            return $crx;
        }
    }

    if ($cli) {
        /* trying to guess ... */
        $crx_path = $crx;
        if (defined("CRX_WINDOWS_VERSION_MAJOR")) {
            /* On Windows it should be in the same dir as crx.exe in most of the cases. */
            $cgi_path = dirname($crx) . "/crx-cgi.exe";
            if (is_executable($cgi_path)) {
                return $cgi_path;
            }
        } else {
            /* Try in the same path as crx, for the case where crx is installed. */
            $cgi_path = dirname($crx) . "/crx-cgi";
            if (is_executable($cgi_path)) {
                return $cgi_path;
            }

            /* Try sapi/cgi/crx-cgi, for the case where crx is not installed. */
            $cgi_path = dirname($crx, 3) . "/sapi/cgi/crx-cgi";
            if (is_executable($cgi_path)) {
                return $cgi_path;
            }
        }
        return false;
    }
    /* uhm? what's that then? */
    return false;
}
/* }}} */

function reset_env_vars() /* {{{ */
{
    putenv("REDIRECT_STATUS");
    putenv("QUERY_STRING");
    putenv("PATH_TRANSLATED");
    putenv("SCRIPT_FILENAME");
    putenv("SERVER_SOFTWARE");
    putenv("SERVER_NAME");
    putenv("GATEWAY_INTERFACE");
    putenv("REQUEST_METHOD");
}
/* }}} */

?>
