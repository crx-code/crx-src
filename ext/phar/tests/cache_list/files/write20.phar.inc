<?crx

$fname = dirname(__FILE__) . '/write20.crxa';
@unlink($fname);

$crxa = new Crxa($fname);
$crxa->setStub('<?crx
$p = new Crxa(__FILE__);
var_dump($p["test.txt"]->getMetadata());
$p["test.txt"]->delMetadata();
var_dump($p["test.txt"]->getMetadata());
echo "ok\n";
__HALT_COMPILER();
?>');
$crxa['test.txt'] = "<?crx __HALT"."_COMPILER();";
$crxa["test.txt"]->setMetadata("hi");
?>
