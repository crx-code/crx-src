<?crx
@unlink(dirname(__FILE__) . '/frontcontroller8.crxa');
$a = new Crxa(dirname(__FILE__) . '/frontcontroller8.crxa');
$a['a.crxs'] = 'hio1';
$a['a1.crxs'] = '<?crx var_dump($_SERVER["REQUEST_URI"], $_SERVER["PATH_INFO"]);';
$a['a.jpg'] = 'hio2';
$a['a.crx'] = '<?crx function hio(){}';
$a['fronk.gronk'] = 'hio3';
$a['404.crx'] = 'My 404 is rawesome';
$a['noext'] = 'hi';
$a['unknown.ext'] = '<?crx var_dump("hi");';
$a['bigfile.txt'] = str_repeat('a', 8193);
$a['fatalerror.crxs'] = '<?crx oopsie_daisy();';
$a->setStub('<?crx
Crxa::webCrxa("whatever", "index.crx", "404.crx", array("jpg" => "foo/bar", "crxs" => Crxa::CRX, "crx" => Crxa::CRXS));
echo "oops did not run\n";
var_dump($_ENV, $_SERVER);
__HALT_COMPILER();');
?>
