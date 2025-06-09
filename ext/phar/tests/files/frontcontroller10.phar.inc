<?crx
@unlink(dirname(__FILE__) . '/frontcontroller10.crxa');
$a = new Crxa(dirname(__FILE__) . '/frontcontroller10.crxa');
$a['index.crx'] = '<?crx
var_dump($_SERVER["CRX_SELF"]);
var_dump($_SERVER["SCRIPT_NAME"]);
var_dump($_SERVER["SCRIPT_FILENAME"]);
var_dump($_SERVER["REQUEST_URI"]);
var_dump($_SERVER["CRXA_CRX_SELF"]);
var_dump($_SERVER["CRXA_SCRIPT_NAME"]);
var_dump($_SERVER["CRXA_SCRIPT_FILENAME"]);
var_dump($_SERVER["CRXA_REQUEST_URI"]);
';
$a->setStub('<?crx
Crxa::mungServer(array("CRX_SELF", "SCRIPT_NAME", "SCRIPT_FILENAME", "REQUEST_URI", "OOPSIE"));
Crxa::webCrxa();
echo "oops did not run\n";
var_dump($_ENV, $_SERVER);
__HALT_COMPILER();');
?>
