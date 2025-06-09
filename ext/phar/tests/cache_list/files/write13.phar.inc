<?crx

$fname = dirname(__FILE__) . '/write13.crxa';
@unlink($fname);

$crxa = new Crxa($fname);
$crxa->setStub('<?crx
$p = new Crxa(__FILE__);
var_dump($p["test.txt"]->isCompressed());
$p2 = new Crxa(__FILE__);
$p->compressFiles(Crxa::GZ);
var_dump($p["test.txt"]->isCompressed());
echo "ok\n";
__HALT_COMPILER();
?>');
$crxa['test.txt'] = "<?crx __HALT"."_COMPILER();";
?>
