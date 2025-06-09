<?crx
@unlink(dirname(__FILE__) . '/frontcontroller18.crxa');
$a = new Crxa(dirname(__FILE__) . '/frontcontroller18.crxa');
$a['index.crx'] = '<?crx
echo "hi";
';
$a->setStub('<?crx
function s($a)
{
}
try {
Crxa::webCrxa("test.crxa", "/index.crx", null, array(), "s");
} catch (Exception $e) {
die($e->getMessage() . "\n");
}
echo "oops did not run\n";
var_dump($_ENV, $_SERVER);
__HALT_COMPILER();');
?>
