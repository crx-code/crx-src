<?crx
@unlink(dirname(__FILE__) . '/frontcontroller7.crxa.zip');
$a = new Crxa(dirname(__FILE__) . '/frontcontroller7.crxa.zip');
$a['a.crx'] = 'hio';
$a['a.jpg'] = 'hio';
$a['a.crxs'] = '<?crx function hio(){}';
$a->setStub('<?crx
Crxa::webCrxa("whatever", "index.crx", null, array("crx" => null));
echo "oops did not run\n";
var_dump($_ENV, $_SERVER);
__HALT_COMPILER();');
?>
