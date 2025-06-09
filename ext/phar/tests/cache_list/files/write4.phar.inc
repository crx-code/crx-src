<?crx

$fname = dirname(__FILE__) . '/write4.crxa';
@unlink($fname);

$crxa = new Crxa($fname);
$crxa->setStub('<?crx
var_dump(file_exists("crxa://" . __FILE__ . "/test.txt"));
clearstatcache();
Crxa::mount("test.txt", "crxa://" . __FILE__ . "/tobemounted");
var_dump(file_exists("crxa://" . __FILE__ . "/test.txt"), file_get_contents("crxa://" . __FILE__ . "/test.txt"));
echo "ok\n";
__HALT_COMPILER();
?>');
$crxa['tobemounted'] = "hi";
?>
