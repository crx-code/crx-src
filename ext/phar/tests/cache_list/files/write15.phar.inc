<?crx

$fname = dirname(__FILE__) . '/write15.crxa';
@unlink($fname);

$crxa = new Crxa($fname);
$crxa->setStub('<?crx
$p = new Crxa(__FILE__);
var_dump(isset($p["copied"]));
$p->copy("test.txt","copied");
var_dump(isset($p["copied"]));
echo "ok\n";
__HALT_COMPILER();
?>');
$crxa['test.txt'] = "<?crx __HALT"."_COMPILER();";
?>
