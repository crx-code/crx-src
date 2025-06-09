<?crx

/**
 * @file crxa.inc
 * @ingroup Crxa
 * @brief class Crxa
 * @author  Marcus Boerger
 * @date    2007 - 2008
 *
 * Crxa Command
 */
// {{{ class Crxa extends CRX_Archive
/**
 * Crxa class
 *
 * @ingroup Crxa
 * @brief   Crxa implementation
 * @author  Marcus Boerger
 * @version 1.0
 */
class Crxa extends CRX_Archive implements RecursiveIterator
{
    function getSignature()
    {
        return false;
    }

    function getAlias()
    {
        return false;
    }

    function rewind()
    {
    }

    function valid()
    {
        return false;
    }

    function current()
    {
    }

    function key()
    {
    }

    function next()
    {
    }

    function hasChildren()
    {
        return false;
    }

    function getChildren()
    {
    }

    function hasMetadata()
    {
    }

    function getMetadata()
    {
    }

    function getStub()
    {
    }

    function setStub()
    {
    }
}

?>
