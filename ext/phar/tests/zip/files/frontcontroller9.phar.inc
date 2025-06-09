<?crx
@unlink(dirname(__FILE__) . '/frontcontroller9.crxa.zip');
$a = new Crxa(dirname(__FILE__) . '/frontcontroller9.crxa.zip');
$a['a.crxs'] = 'hio1';
$a['a.jpg'] = 'hio2';
$a['a.crx'] = '<?crx function hio(){}';
$a['fronk.gronk'] = 'hio3';
$a->setStub('<?crx
Crxa::mungServer(array());
Crxa::webCrxa("whatever", "index.crx", null, array("jpg" => "foo/bar", "crxs" => Crxa::CRX, "crx" => Crxa::CRXS));
echo "oops did not run\n";
var_dump($_ENV, $_SERVER);
__HALT_COMPILER();');
?>
