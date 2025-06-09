<?crx
@unlink(dirname(__FILE__) . '/frontcontroller3.crxa');
$a = new Crxa(dirname(__FILE__) . '/frontcontroller3.crxa');
$a['a.crx'] = 'hio';
$a['a.jpg'] = 'hio';
$a['a.crxs'] = '<?crx function hio(){}';
$a->setStub('<?crx
function s($a)
{
    static $b = array("/hi" => "a.crxs");
    if (isset($b[$a])) return $b[$a];
    return $a;
}
Crxa::webCrxa("whatever", "/index.crx", null, array(), "s");
echo "oops did not run\n";
var_dump($_ENV, $_SERVER);
__HALT_COMPILER();');
?>
