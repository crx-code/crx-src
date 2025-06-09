<?crx
@unlink(dirname(__FILE__) . '/frontcontroller11.crxa.tar');
$a = new Crxa(dirname(__FILE__) . '/frontcontroller11.crxa.tar');
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
Crxa::mungServer(array(array(), "SCRIPT_NAME", "SCRIPT_FILENAME", "REQUEST_URI"));
Crxa::webCrxa();
echo "oops did not run\n";
var_dump($_ENV, $_SERVER);
__HALT_COMPILER();');
?>
