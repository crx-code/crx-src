<?crx
@unlink(dirname(__FILE__) . '/frontcontroller14.crxa');
$a = new Crxa(dirname(__FILE__) . '/frontcontroller14.crxa');
$a['html/index.crx'] = '<?crx
var_dump($_SERVER["PATH_INFO"]);
var_dump($_SERVER["PATH_TRANSLATED"]);
';
$a->setStub('<?crx
function s($a)
{
    return "/html/index.crx";
}
Crxa::interceptFileFuncs();
Crxa::mungServer(array("CRX_SELF", "REQUEST_URI"));
Crxa::webCrxa("whatever", "/html/index.crx", null, array(), "s");
echo "oops did not run\n";
var_dump($_ENV, $_SERVER);
__HALT_COMPILER();');
?>
