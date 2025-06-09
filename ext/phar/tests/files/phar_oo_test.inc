<?crx

ini_set('date.timezone', 'GMT');

$tname = basename(current(get_included_files()), ".crx");
$fname = dirname(__FILE__) . "/$tname.crxa.crx";
$pname = 'crxa://' . $fname;
$file = (binary)'<?crx include "' . $pname . '/a.crx"; __HALT_COMPILER(); ?>';

$files = array();

if (!isset($crxaconfig)) $crxaconfig = 0;

switch($crxaconfig)
{
    default:
    case 0:
        $files['a.crx']   = '<?crx echo "This is a.crx\n"; ?>';
        $files['b.crx']   = '<?crx echo "This is b.crx\n"; ?>';
        $files['b/c.crx'] = '<?crx echo "This is b/c.crx\n"; ?>';
        $files['b/d.crx'] = '<?crx echo "This is b/d.crx\n"; ?>';
        $files['e.crx']   = '<?crx echo "This is e.crx\n"; ?>';
        break;
    case 1:
        $files['a.csv'] =<<<EOF
1,2,3
2,a,b
3,"c","'e'"
EOF;
        break;
    case 2:
        $files['a.csv'] =<<<EOF
1,2,3
2,a,b
3,"c","'e'"
4
5,5

7,777
EOF;
        break;
    case 3:
        $files['a.crx'] = '<?crx echo new new class;';
        break;
}

$ftime = mktime(12, 0, 0, 3, 1, 2006);
include 'crxa_test.inc';

?>
