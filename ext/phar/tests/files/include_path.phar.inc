<?crx
// test advanced example
$crxa = new Crxa(dirname(__FILE__) . '/include_path.crxa');
$crxa2 = new Crxa(dirname(__FILE__) . '/include_path2.crxa');
$crxa2['file1.crx'] = 'file1.crx
';
$crxa2['test/file1.crx'] = 'test/file1.crx
';
$crxa['hello/test.crx'] = '<?crx
include "file1.crx";';

$crxa->setStub("<?crx
set_include_path('.' . PATH_SEPARATOR . 'crxa://' . dirname(__FILE__) . '/files/include_path2.crxa' );
include 'crxa://' . __FILE__ . '/hello/test.crx';
set_include_path('.' . PATH_SEPARATOR . 'crxa://' . dirname(__FILE__) . '/files/include_path2.crxa/test');
include 'crxa://' . __FILE__ . '/hello/test.crx';
echo \"ok\\n\";
__HALT_COMPILER();
?>");
