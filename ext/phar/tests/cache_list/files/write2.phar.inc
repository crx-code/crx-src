<?crx

$fname = dirname(__FILE__) . '/write2.crxa';
@unlink($fname);

$crxa = new Crxa($fname);
$crxa->setStub('<?crx
$crxa = new Crxa(__FILE__);
var_dump($crxa->getMetadata());
mkdir("crxa://" . __FILE__ . "/test");
var_dump(is_dir("crxa://" . __FILE__ . "/test"));
$crxa2 = new Crxa(__FILE__);
var_dump($crxa2->getMetadata());
var_dump(isset($crxa["test"]));
var_dump(isset($crxa2["test"]));
echo "ok\n";
__HALT_COMPILER();
?>');
$crxa->setMetadata('hi');
$crxa['test.txt'] = "hi
";
$crxa['test.txt']->setMetadata('hi');
?>
