<?crx

$fname = dirname(__FILE__) . '/write8.crxa';
@unlink($fname);

$crxa = new Crxa($fname);
$crxa->setStub('<?crx
$p = new Crxa(__FILE__);
var_dump($p->getAlias());
$p2 = new Crxa(__FILE__);
$p->setAlias("hi");
echo $p2->getAlias(),"\n";
echo "ok\n";
__HALT_COMPILER();
?>');
$crxa['test.txt'] = "hi
";
?>
