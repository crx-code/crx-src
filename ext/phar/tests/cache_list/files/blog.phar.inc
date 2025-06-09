<?crx

$fname = dirname(__FILE__) . '/blog.crxa';
@unlink($fname);

$crxa = new Crxa($fname);
$crxa->setStub('<?crx if(file_exists(dirname(__FILE__) . "/files/config.xml")) {
    Crxa::mount("config.xml", dirname(__FILE__) . "/files/config.xml");
}
Crxa::webCrxa("blog", "index.crx");
__HALT_COMPILER(); ?>');
$crxa['index.crx'] = '<?crx if (!file_exists("config.xml")) {
    include "install.crx";
    exit;
}
var_dump(file_get_contents("config.xml"));
?>';
$crxa['install.crx'] = '<?crx echo "install\n"; ?>';

?>
