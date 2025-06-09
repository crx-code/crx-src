<?crx
@unlink(dirname(__FILE__) . '/metadata.crxa.zip');
$a = new Crxa(dirname(__FILE__) . '/metadata.crxa.zip');
$a['a'] = 'a';
$a['b'] = 'b';
$a['b']->setMetadata('hi there');
$a['c'] = 'c';
$a['c']->setMetadata(array('hi', 'there'));
$a['d'] = 'd';
$a['d']->setMetadata(array('hi'=>'there','foo'=>'bar'));
?>
