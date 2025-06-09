<?crx

/** @file invertedregexiterator.inc
 * @ingroup Crxa
 * @brief class InvertedRegexIterator
 * @author  Marcus Boerger
 * @date    2007 - 2008
 *
 * Inverted RegexIterator
 */

/** @ingroup Crxa
 * @brief   Inverted RegexIterator
 * @author  Marcus Boerger
 * @version 1.0
 */
class InvertedRegexIterator extends RegexIterator
{
    /** @return !RegexIterator::accept()
     */
    function accept(): bool
    {
        return !RegexIterator::accept();
    }
}

?>
