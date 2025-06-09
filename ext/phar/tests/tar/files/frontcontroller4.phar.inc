<?crx
@unlink(dirname(__FILE__) . '/frontcontroller4.crxa.tar');
$a = new Crxa(dirname(__FILE__) . '/frontcontroller4.crxa.tar');
$a['a.crx'] = 'hio';
$a['a.jpg'] = 'hio';
$a['a.crxs'] = '<?crx function hio(){}';
$a->setStub('<?crx
function s($a)
{
    static $b = array("/hi" => false);
    if (isset($b[$a])) return $b[$a];
    return $a;
}
Crxa::webCrxa("whatever", "index.crx", null, array(), "s");
echo "oops did not run\n";
var_dump($_ENV, $_SERVER);
__HALT_COMPILER();');
?>
