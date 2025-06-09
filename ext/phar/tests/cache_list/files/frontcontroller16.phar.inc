<?crx
@unlink(dirname(__FILE__) . '/frontcontroller16.crxa');
$a = new Crxa(dirname(__FILE__) . '/frontcontroller16.crxa');
$a['index.crx'] = '<?crx
echo "hi";
';
$a->setStub('<?crx
try {
Crxa::webCrxa("test.crxa", "/index.crx", null, array(), array("fail", "here"));
} catch (TypeError|Exception $e) {
die($e->getMessage() . "\n");
}
echo "oops did not run\n";
var_dump($_ENV, $_SERVER);
__HALT_COMPILER();');
?>
