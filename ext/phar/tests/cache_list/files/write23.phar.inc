<?crx

$fname = dirname(__FILE__) . '/write23.crxa';
@unlink($fname);

$crxa = new Crxa($fname);
$crxa->setStub('<?crx
$p = new Crxa(__FILE__);
var_dump(isset($p["test.txt"]), isset($p["newname"]));
rename("crxa://" . __FILE__ . "/test.txt", "crxa://" . __FILE__ . "/newname");
var_dump(isset($p["test.txt"]), isset($p["newname"]));
echo "ok\n";
__HALT_COMPILER();
?>');
$crxa['test.txt'] = "<?crx __HALT"."_COMPILER();";
?>
