<?crx

$fname = dirname(__FILE__) . '/write25.crxa';
@unlink($fname);

$crxa = new Crxa($fname);
$crxa->setStub('<?crx
$p = new Crxa(__FILE__);
var_dump(isset($p["mounted"]));
Crxa::mount("mounted", "crxa://" . __FILE__ . "/test.txt");
var_dump(isset($p["mounted"]));
echo $p["mounted"]->getContent(),"\nok\n";
__HALT_COMPILER();
?>');
$crxa['test.txt'] = "<?crx __HALT"."_COMPILER();";
?>
