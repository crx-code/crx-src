<?crx

$fname = dirname(__FILE__) . '/write7.crxa';
@unlink($fname);

$crxa = new Crxa($fname);
$crxa->setStub('<?crx
$p = new Crxa(__FILE__);
var_dump(file_exists("crxa://" . __FILE__ . "/test.txt"));
$p->delete("test.txt");
clearstatcache();
var_dump(file_exists("crxa://" . __FILE__ . "/test.txt"));
echo "ok\n";
__HALT_COMPILER();
?>');
$crxa['test.txt'] = "hi
";
?>
