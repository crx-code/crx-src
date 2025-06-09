<?crx

$fname = dirname(__FILE__) . '/write17.crxa';
@unlink($fname);

$crxa = new Crxa($fname);
$crxa->setStub('<?crx
$p = new Crxa(__FILE__);
var_dump($p->getMetadata());
$p->setMetadata("hi");
var_dump($p->getMetadata());
echo "ok\n";
__HALT_COMPILER();
?>');
$crxa['test.txt'] = "<?crx __HALT"."_COMPILER();";
?>
