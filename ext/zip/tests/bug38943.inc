<?crx
class myZip extends ZipArchive {
    private $test = 0;
    public $testp = 1;
    private $testarray = array();

    public function __main() {
        $this->testarray[] = 1;
        var_dump($this->testarray);
    }
}

$z = new myZip;
$z->testp = "foobar";
var_dump($z);
