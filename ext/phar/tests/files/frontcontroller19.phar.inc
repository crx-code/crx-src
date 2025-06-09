<?crx
@unlink(dirname(__FILE__) . '/frontcontroller19.crxa');
$a = new Crxa(dirname(__FILE__) . '/frontcontroller19.crxa');
$a['start/index.crx'] = '<?crx
echo "start/index.crx\n";
include "./another.crx";
';
$a['start/another.crx'] = '<?crx
echo "start/another.crx\n";
include "../another.crx";
?>';
$a['another.crx'] = '<?crx
echo "another.crx\n";
?>';
$a->setStub('<?crx
set_include_path("crxa://" . __FILE__);
try {
Crxa::webCrxa("test.crxa", "/start/index.crx");
} catch (Exception $e) {
die($e->getMessage() . "\n");
}
echo "oops did not run\n";
var_dump($_ENV, $_SERVER);
__HALT_COMPILER();');
?>
