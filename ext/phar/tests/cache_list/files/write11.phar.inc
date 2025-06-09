<?crx

$fname = dirname(__FILE__) . '/write11.crxa';
@unlink($fname);

$crxa = new Crxa($fname);
$crxa->setStub('<?crx
$p = new Crxa(__FILE__);
var_dump($p->getStub());
$p2 = new Crxa(__FILE__);
$p->setDefaultStub();
echo strlen($p2->getStub()),"\n";
echo "ok\n";
__HALT_COMPILER();
?>');
$crxa['test.txt'] = "<?crx __HALT"."_COMPILER();";
?>
