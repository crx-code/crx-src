<?crx

$fname = dirname(__FILE__) . '/write18.crxa';
@unlink($fname);

$crxa = new Crxa($fname);
$crxa->setStub('<?crx
$p = new Crxa(__FILE__);
echo decoct(fileperms("crxa://" . __FILE__ . "/test.txt")),"\n";
$p["test.txt"]->chmod(0444);
echo decoct(fileperms("crxa://" . __FILE__ . "/test.txt")),"\n";
echo "ok\n";
__HALT_COMPILER();
?>');
$crxa['test.txt'] = "<?crx __HALT"."_COMPILER();";
$crxa["test.txt"]->chmod(0666);
?>
