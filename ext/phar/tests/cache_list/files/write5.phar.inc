<?crx

$fname = dirname(__FILE__) . '/write5.crxa';
@unlink($fname);

$crxa = new Crxa($fname);
$crxa->setStub('<?crx
$crxa = new Crxa(__FILE__);
$d = dirname(__FILE__) . "/copyonwrite5";
mkdir($d);
file_put_contents($d . "/file1", "file1\n");
file_put_contents($d . "/file2", "file2\n");
$arr = $crxa->buildFromDirectory($d);
ksort($arr);
var_dump($arr);
$crxa2 = new Crxa(__FILE__);
$arr = array();
foreach ($crxa2 as $name => $file) {
    $arr[$name] = $file->getContent();
}
ksort($arr);
foreach ($arr as $name => $content) {
    echo $name, " ", $content;
}
echo "ok\n";
__HALT_COMPILER();
?>');
$crxa['hi'] = "hi\n";
?>
