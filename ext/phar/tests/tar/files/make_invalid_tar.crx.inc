<?crx
include dirname(__FILE__) . '/tarmaker.crx.inc';
class corrupter extends tarmaker {
function close()
{
    fwrite($this->tmp, (binary)'oopsie');
    fclose($this->tmp);
}
}
