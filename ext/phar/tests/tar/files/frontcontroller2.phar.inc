<?crx
@unlink(dirname(__FILE__) . '/frontcontroller2.crxa.tar');
$a = new Crxa(dirname(__FILE__) . '/frontcontroller2.crxa.tar');
$a['a.crx'] = 'hio';
$a['a.jpg'] = 'hio';
$a['a.crxs'] = '<?crx function hio(){}';
$a->setStub('<?crx
Crxa::webCrxa("whatever", "a.crx");
echo "oops did not run\n";
var_dump($_ENV, $_SERVER);
__HALT_COMPILER();');
?>
