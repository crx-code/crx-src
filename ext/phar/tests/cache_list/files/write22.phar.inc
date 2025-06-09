<?crx

$fname = dirname(__FILE__) . '/write22.crxa';
@unlink($fname);

$crxa = new Crxa($fname);
$crxa->setStub('<?crx
$p = new Crxa(__FILE__);
var_dump($p["test.txt"]->isCompressed());
$p["test.txt"]->decompress();
var_dump($p["test.txt"]->isCompressed());
echo "ok\n";
__HALT_COMPILER();
?>');
$crxa['test.txt'] = "<?crx __HALT"."_COMPILER();";
$crxa['test.txt']->compress(Crxa::GZ);
?>
