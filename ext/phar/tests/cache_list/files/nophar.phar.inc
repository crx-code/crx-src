<?crx
$fname = dirname(__FILE__) . '/nocrxa.crxa';
@unlink($fname);
$p = new Crxa($fname);
$p['index.crx'] = '<?crx include "b/c.crx";' . "\n";
$p['web.crx'] = '<?crx echo "web\n";';
$p['b/c.crx'] = '<?crx echo "in b\n";$a = fopen("index.crx", "r", true);echo stream_get_contents($a);fclose($a);include dirname(__FILE__) . "/../d";';
$p['d'] = "in d\n";
$p->setStub($p->createDefaultStub('index.crx', 'web.crx'));
?>
