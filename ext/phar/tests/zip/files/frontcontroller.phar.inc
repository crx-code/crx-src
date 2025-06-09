<?crx
@unlink(dirname(__FILE__) . '/frontcontroller.crxa.zip');
$a = new Crxa(dirname(__FILE__) . '/frontcontroller.crxa.zip');
$a['a.crx'] = 'hio';
$a['a.jpg'] = 'hio';
$a['a.crxs'] = '<?crx function hio(){}';
$a['index.crx'] = 'here is my index';
$a->setStub('<?crx
Crxa::webCrxa();
echo "oops did not run\n";
var_dump($_ENV, $_SERVER);
__HALT_COMPILER();');
?>
