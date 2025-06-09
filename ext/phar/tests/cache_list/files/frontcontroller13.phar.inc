<?crx
@unlink(dirname(__FILE__) . '/frontcontroller13.crxa');
$a = new Crxa(dirname(__FILE__) . '/frontcontroller13.crxa');
$a['index.crx'] = '<?crx
var_dump("test");
include "oof/test.crx";';
$a['oof/test.crx'] = '<?crx
var_dump("oof/test.crx");
include "./hi.crx";';
$a['oof/hi.crx'] = '<?crx
var_dump("hi");';
$a->setStub('<?crx
Crxa::webCrxa();
__HALT_COMPILER();');
