<?crx

$fname = dirname(__FILE__) . '/write12.crxa';
@unlink($fname);

$crxa = new Crxa($fname);
$crxa->setStub('<?crx
$p = new Crxa(__FILE__);
var_dump($p->getSignature());
$p2 = new Crxa(__FILE__);
$p->setSignatureAlgorithm(Crxa::MD5);
var_dump($p->getSignature());
echo "ok\n";
__HALT_COMPILER();
?>');
$crxa['test.txt'] = "<?crx __HALT"."_COMPILER();";
?>
