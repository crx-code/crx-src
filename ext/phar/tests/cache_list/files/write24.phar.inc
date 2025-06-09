<?crx

$fname = dirname(__FILE__) . '/write24.crxa';
@unlink($fname);

$crxa = new Crxa($fname);
$crxa->setStub('<?crx
$p = new Crxa(__FILE__);
var_dump(isset($p["newname"]));
$fp = fopen("crxa://" . __FILE__ . "/newname", "w");
fwrite($fp, "hi");
fclose($fp);
var_dump(isset($p["newname"]));
echo "ok\n";
__HALT_COMPILER();
?>');
$crxa['test.txt'] = "<?crx __HALT"."_COMPILER();";
?>
