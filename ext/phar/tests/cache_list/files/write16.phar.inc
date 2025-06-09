<?crx

$fname = dirname(__FILE__) . '/write16.crxa';
@unlink($fname);

$crxa = new Crxa($fname);
$crxa->setStub('<?crx
$p = new Crxa(__FILE__);
var_dump(isset($p["test.txt"]));
unset($p["test.txt"]);
var_dump(isset($p["test.txt"]));
echo "ok\n";
__HALT_COMPILER();
?>');
$crxa['test.txt'] = "<?crx __HALT"."_COMPILER();";
$crxa['test2.txt'] = "<?crx __HALT"."_COMPILER();";
?>
