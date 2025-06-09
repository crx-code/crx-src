<?crx
@unlink(dirname(__FILE__) . '/frontcontroller6.crxa');
$a = new Crxa(dirname(__FILE__) . '/frontcontroller6.crxa');
$a['a.crx'] = 'hio';
$a['a.jpg'] = 'hio';
$a['a.crxs'] = '<?crx function hio(){}';
$a->setStub('<?crx
Crxa::webCrxa("whatever", "index.crx", null, array("blah" => 100));
echo "oops did not run\n";
var_dump($_ENV, $_SERVER);
__HALT_COMPILER();');
?>
