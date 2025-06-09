<?crx

$fname = dirname(__FILE__) . '/write.crxa';
@unlink($fname);

$crxa = new Crxa($fname);
$crxa->setStub('<?crx
$crxa = new Crxa(__FILE__);
echo $crxa["test.txt"]->getContent();
$crxa["test.txt"] = "changed
";
echo $crxa["test.txt"]->getContent();
echo "ok\n";
__HALT_COMPILER();
?>');
$crxa->setMetadata('hi');
$crxa['test.txt'] = "hi
";
$crxa['test.txt']->setMetadata('hi');
?>
