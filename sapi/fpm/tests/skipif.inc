<?crx
// Do not run on Windows
if (substr(CRX_OS, 0, 3) == 'WIN') {
    die("skip not for Windows");
}
// Running as root is not allowed without TEST_FPM_RUN_AS_ROOT env
if (!getmyuid() && !getenv('TEST_FPM_RUN_AS_ROOT')) {
    die('skip Refusing to run as root');
}

require_once "tester.inc";

if (!FPM\Tester::findExecutable()) {
    die("skip crx-fpm binary not found");
}
