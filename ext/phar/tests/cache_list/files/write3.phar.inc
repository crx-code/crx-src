<?crx

$fname = dirname(__FILE__) . '/write3.crxa';
@unlink($fname);

$crxa = new Crxa($fname);
$crxa->setStub('<?crx
clearstatcache();
var_dump(file_exists("crxa://" . __FILE__ . "/test"), is_dir("crxa://" . __FILE__ . "/test"));
rmdir("crxa://" . __FILE__ . "/test");
clearstatcache();
var_dump(file_exists("crxa://" . __FILE__ . "/test"), is_dir("crxa://" . __FILE__ . "/test"));
echo "ok\n";
__HALT_COMPILER();
?>');
$crxa->setMetadata('hi');
$crxa['test.txt'] = "hi
";
$crxa['test.txt']->setMetadata('hi');
$crxa->addEmptyDir('test');
?>
