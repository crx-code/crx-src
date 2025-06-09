<?crx

/**
 * @file crxacommand.inc
 * @ingroup Crxa
 * @brief class CLICommand
 * @author  Marcus Boerger
 * @date    2007 - 2008
 *
 * Crxa Command
 */
// {{{ class CrxaCommand extends CLICommand
/**
 * CrxaCommand class
 *
 * This class handles the handling of the crxa
 * commands. It will be used from command line/console
 * in order to retrieve and execute crxa functions.
 *
 * @ingroup Crxa
 * @brief   Crxa console command implementation
 * @author  Marcus Boerger
 * @version 1.0
 */
class CrxaCommand extends CLICommand
{
    // {{{ public function cli_get_SP2
    public function cli_get_SP2($l1, $arg_inf)
    {
        return str_repeat(' ', $l1 + 2 + 4 + 9);
    }
    // }}}
    // {{{ public function cli_get_SP3
    /**
     * Cli Get SP3
     *
     * @param string $l1      Eleven
     * @param string $l2      Twelve
     * @param string $arg_inf
     * @return string  The repeated string.
     */
    function cli_get_SP3($l1, $l2, $arg_inf)
    {
        return str_repeat(' ', $l1 + 2 + 4 + 9 + 2 + $l2 + 2);
    }
    // }}}
    // {{{ static function crxa_args
    /**
     * Crxa arguments
     *
     * This function contains all the crxa commands
     *
     * @param  string $which    Which argument is chosen.
     * @param  string $crxatype The type of crxa, specific file to work on
     * @return unknown
     */
    static function crxa_args($which, $crxatype)
    {
        $crxa_args = array(
            'a' => array(
                'typ' => 'alias',
                'val' => NULL,
                'inf' => '<alias>  Provide an alias name for the crxa file.'
            ),
            'b' => array(
                'typ' => 'any',
                'val' => NULL,
                'inf' => '<bang>   Hash-bang line to start the archive (e.g. #!/usr/bin/crx). The hash '
                         .'         mark itself \'#!\' and the newline character are optional.'
            ),
            'c' => array(
                'typ' => 'compalg',
                'val' => NULL,
                'inf' => '<algo>   Compression algorithm.',
                'select' => array(
                    '0'    => 'No compression',
                    'none' => 'No compression',
                    'auto' => 'Automatically select compression algorithm'
                )
            ),
            'e' => array(
                'typ' => 'entry',
                'val' => NULL,
                'inf' => '<entry>  Name of entry to work on (must include CRXA internal directory name if any).'
            ),
            'f' => array(
                'typ' => $crxatype,
                'val' => NULL,
                'inf' => '<file>   Specifies the crxa file to work on.'
            ),
            'h' => array(
                'typ' => 'select',
                'val' => NULL,
                'inf' => '<method> Selects the hash algorithm.',
                'select' => ['md5' => 'MD5','sha1' => 'SHA1', 'sha256' => 'SHA256', 'sha512' => 'SHA512', 'openssl' => 'OPENSSL', 'openssl_sha256' => 'OPENSSL_SHA256', 'openssl_sha512' => 'OPENSSL_SHA512']
            ),
            'i' => array(
                'typ' => 'regex',
                'val' => NULL,
                'inf' => '<regex>  Specifies a regular expression for input files.'
            ),
            'k' => array(
                'typ' => 'any',
                'val' => NULL,
                'inf' => '<index>  Subscription index to work on.',
            ),
            'l' => array(
                'typ' => 'int',
                'val' => 0,
                'inf' => '<level>  Number of preceding subdirectories to strip from file entries',
            ),
            'm' => array(
                'typ' => 'any',
                'val' => NULL,
                'inf' => '<meta>   Meta data to store with entry (serialized crx data).'
            ),
            'p' => array(
                'typ' => 'loader',
                'val' => NULL,
                'inf' => '<loader> Location of CRX_Archive class file (pear list-files CRX_Archive).'
                         .'You can use \'0\' or \'1\' to locate it automatically using the mentioned '
                         .'pear command. When using \'0\' the command does not error out when the '
                         .'class file cannot be located. This switch also adds some code around the '
                         .'stub so that class CRX_Archive gets registered as crxa:// stream wrapper '
                         .'if necessary. And finally this switch will add the file crxa.inc from '
                         .'this package and load it to ensure class Crxa is present.'
                         ,
            ),
            's' => array(
                'typ' => 'file',
                'val' => NULL,
                'inf' => '<stub>   Select the stub file.'
            ),
            'x' => array(
                'typ' => 'regex',
                'val' => NULL,
                'inf' => '<regex>  Regular expression for input files to exclude.'
            ),
            'y' => array(
                'typ' => 'privkey',
                'val' => NULL,
                'inf' => '<key>    Private key for OpenSSL signing.',
            ),
        );

        if (extension_loaded('zlib')) {
            $crxa_args['c']['select']['gz']    = 'GZip compression';
            $crxa_args['c']['select']['gzip']  = 'GZip compression';
        }

        if (extension_loaded('bz2')) {
            $crxa_args['c']['select']['bz2']   = 'BZip2 compression';
            $crxa_args['c']['select']['bzip2'] = 'BZip2 compression';
        }

        $hash_avail = Crxa::getSupportedSignatures();
        $hash_optional = array('SHA-256' => 'SHA256',
                               'SHA-512' => 'SHA512',
                               'OpenSSL_sha256' => 'OpenSSL_SHA256',
                               'OpenSSL_sha512' => 'OpenSSL_SHA512',
                               'OpenSSL' => 'OpenSSL');
        if (!in_array('OpenSSL', $hash_avail)) {
            unset($crxa_args['y']);
        }

        foreach($hash_optional as $key => $name) {
            if (in_array($key, $hash_avail)) {
                $crxa_args['h']['select'][strtolower($name)] = $name;
            }
        }

        $args = array();

        foreach($crxa_args as $lkey => $cfg) {
            $ukey     = strtoupper($lkey);
            $required = strpos($which, $ukey) !== false;
            $optional = strpos($which, $lkey) !== false;

            if ($required || $optional) {
                $args[$lkey] = $cfg;
                $args[$lkey]['required'] = $required;
            }
        }
        return $args;
    }
    // }}}
    // {{{ static function strEndsWith
    /**
     * String Ends With
     *
     * Whether a string ends with another needle.
     *
     * @param string $haystack  The haystack
     * @param string $needle    The needle.
     * @return mixed false if doesn't end with anything, the string
     *               substr'ed if the string ends with the needle.
     */
    static function strEndsWith($haystack, $needle)
    {
        return substr($haystack, -strlen($needle)) == $needle;
    }
    // }}}
    // {{{ static function cli_arg_typ_loader
    /**
     * Argument type loader
     *
     * @param string $arg   Either 'auto', 'optional' or an filename that
     *                      contains class CRX_Archive
     * @param  string $cfg  Configuration to pass to a new file
     * @param  string $key  The key
     * @return string $arg  The argument.
     */
    static function cli_arg_typ_loader($arg, $cfg, $key)
    {
        if (($arg == '0' || $arg == '1') && !file_exists($arg) && substr(CRX_OS, 0, 3) != 'WIN') {
            $found = NULL;
            $apiver = false;
            $path = explode(PATH_SEPARATOR, $_ENV['PATH']);
            $pear = false;
            foreach ($path as $component) {
                if (file_exists($component . DIRECTORY_SEPARATOR . 'pear')
                    && is_executable($component . DIRECTORY_SEPARATOR . 'pear')) {
                    $pear = true;
                    break;
                }
            }
            if ($pear) {
                $apiver = (string) `pear -q info CRX_Archive 2>/dev/null|grep 'API Version'`;
                $apiver = trim(substr($apiver, strlen('API Version')));
            }
            if ($apiver) {
                self::notice("PEAR package CRX_Archive: API Version: $apiver.\n");
                $files  = explode("\n", (string) `pear list-files CRX_Archive`);
                $crxdir = (string) `pear config-get crx_dir 2>/dev/null`;
                $crxdir = trim($crxdir);
                self::notice("PEAR package CRX_Archive: $crxdir.\n");
                if (is_dir($crxdir)) {
                    foreach($files as $ent) {
                        $matches = NULL;
                        if (preg_match(",^crx[ \t]+([^ \t].*[\\\\/]CRX[\\\\/]Archive\.crx)$,", $ent, $matches)) {
                            $sub = $matches[1];
                            if (strpos($sub, $crxdir) !== 0) {
                                $found = NULL;
                                break;
                            }
                            $found = $sub;
                            break;
                        }
                    }
                } else {
                    self::notice("PEAR package CRX_Archive: corrupt or inaccessible base dir: $crxdir.\n");
                }
            }
            if (isset($found)) {
                self::notice("PEAR package CRX_Archive: $found.\n");
            } else {
                $msg = "PEAR package CRX_Archive not installed: generated crxa will require CRX's crxa extension be enabled.\n";
                if ($arg == '0') {
                    self::notice($msg);
                } else {
                    self::error($msg);
                }
            }
            return null;
        }
        return self::cli_arg_typ_file($arg);
    }
    // }}}
    // {{{ static function cli_arg_typ_crxanew
    /**
     * Argument type new crxa
     *
     * @param  string $arg  The new crxa component.
     * @param  string $cfg  Configuration to pass to a new file
     * @param  string $key  The key
     * @return string $arg  The new argument file.
     */
    static function cli_arg_typ_crxanew($arg, $cfg, $key)
    {
        $arg = self::cli_arg_typ_filenew($arg, $cfg, $key);
        if (!Crxa::isValidCrxaFilename($arg)) {
            self::error("Crxa files must have file extension '.crxa', '.crxa.crx', '.crxa.bz2' or '.crxa.gz'.\n");
        }
        return $arg;
    }
    // }}}
    // {{{ static function cli_arg_typ_crxafile
    /**
     * Argument type existing Crxa file
     *
     * Return filename of an existing Crxa.
     *
     * @param  string $arg      The file in the crxa to open.
     * @param  string $cfg      The configuration information
     * @param  string $key      The key information.
     * @return string $crxafile The name of the loaded Crxa file.
     * @note The Crxa will be loaded
     */
    static function cli_arg_typ_crxafile($arg, $cfg, $key)
    {
        try {
            $crxafile = self::cli_arg_typ_file($arg, $cfg, $key);

            if (!Crxa::loadCrxa($crxafile)) {
                self::error("Unable to open crxa '$arg'\n");
            }

            return $crxafile;
        } catch(Exception $e) {
            self::error("Exception while opening crxa '$arg':\n" . $e->getMessage() . "\n");
        }
    }
    // }}}
    // {{{ static function cli_arg_typ_crxaurl
    /**
     * Argument type Crxa url-like
     *
     * Check the argument as cli_arg_Typ_crxa and return its name prefixed
     * with crxa://
     *
     * Ex:
     * <code>
     *  $arg = 'crxachive.crxa/file.crx';
     *  cli_arg_typ_crxaurl($arg)
     * </code>
     *
     * @param  string $arg The url-like crxa archive to retrieve.
     * @return string The crxa file-archive.
     */
    static function cli_arg_typ_crxaurl($arg, $cfg, $key)
    {
        return 'crxa://' . self::cli_arg_typ_crxafile($arg, $cfg, $key);
    }
    // }}}
    // {{{ static function cli_arg_typ_crxa
    /**
     * Cli argument type crxa
     *
     * @param  string $arg  The crxa archive to use.
     * @return object new Crxa of the passed argument.
     */
    static function cli_arg_typ_crxa($arg, $cfg, $key)
    {
        try {
            return new Crxa(self::cli_arg_typ_crxafile($arg, $cfg, $key));
        } catch(Exception $e) {
            self::error("Exception while opening crxa '$argv':\n" . $e->getMessage() . "\n");
        }
    }
    // }}}
    // {{{ static function cli_arg_typ_entry
    /**
     * Argument type Entry name
     *
     * @param  string $arg The argument (the entry)
     * @return string $arg The entry itself.
     */
    static function cli_arg_typ_entry($arg, $cfg, $key)
    {
        // no further check atm, maybe check for no '/' at beginning
        return $arg;
    }
    // }}}
    // {{{ static function cli_arg_typ_compalg
    /**
     * Argument type compression algorithm
     *
     * @param  string $arg  The crxa selection
     * @param  string $cfg  The config option.
     * @param  string $key  The key information.
     * @return string $arg  The selected algorithm
     */
    static function cli_arg_typ_compalg($arg, $cfg, $key)
    {
        $arg = self::cli_arg_typ_select($arg, $cfg, $key);

        switch($arg) {
            case 'auto':
                if (extension_loaded('zlib')) {
                    $arg = 'gz';
                } elseif (extension_loaded('bz2')) {
                    $arg = 'bz2';
                } else {
                    $arg = '0';
                }
                break;
        }
        return $arg;
    }
    // }}}
    // {{{ static function cli_arg_typ_privkey
    /**
     * Argument type private key (for OpenSSL signing)
     *
     * @param  string $arg  The crxa selection
     * @param  string $cfg  The config option.
     * @param  string $key  The key information.
     * @return string $arg  The private key.
     */
    static function cli_arg_typ_privkey($arg, $cfg, $key)
    {
        $arg = self::cli_arg_typ_filecont($arg, $cfg, $key);

        $hash_avail = Crxa::getSupportedSignatures();
        if ($arg && !in_array('OpenSSL', $hash_avail))
        {
            self::error("Cannot specify private key without OpenSSL support.\n");
        }
        return $arg;
    }
    // }}}
    // {{{ static function crxa_check_hash
    /**
     * Check whether hash method is valid.
     *
     * @return Hash constant to be used.
     */
    function crxa_check_hash($hash, $privkey)
    {
        switch($hash) {
            case 'md5':
                return Crxa::MD5;
            case 'sha1':
                return Crxa::SHA1;
            case 'sha256':
                return Crxa::SHA256;
            case 'sha512':
                return Crxa::SHA512;
            case 'openssl':
                if (!$privkey) {
                    self::error("Cannot use OpenSSL signing without key.\n");
                }
                return Crxa::OPENSSL;
            case 'openssl_sha256':
                if (!$privkey) {
                    self::error("Cannot use OpenSSL signing without key.\n");
                }
                return Crxa::OPENSSL_SHA256;
            case 'openssl_sha512':
                if (!$privkey) {
                    self::error("Cannot use OpenSSL signing without key.\n");
                }
                return Crxa::OPENSSL_SHA512;
        }
    }
    // }}}
    // {{{ static function cli_cmd_inf_pack
    /**
     * Information pack
     *
     * @return string A description about packing files into a Crxa archive.
     */
    static function cli_cmd_inf_pack()
    {
        return "Pack files into a CRXA archive.\n" .
               "When using -s <stub>, then the stub file is being " .
               "excluded from the list of input files/dirs." .
               "To create an archive that contains PEAR class CRX_Archive " .
               "then point -p argument to CRX/Archive.crx.\n";
    }
    // }}}
    // {{{ static function cli_cmd_arg_pack
    /**
     * Pack a new crxa infos
     *
     * @return array  $args  The arguments for a new Crxa archive.
     */
    static function cli_cmd_arg_pack()
    {
        $args = self::crxa_args('abcFhilpsxy', 'crxanew');

        $args[''] = array(
            'typ'     => 'any',
            'val'      => NULL,
            'required' => 1,
            'inf'      => '         Any number of input files and directories. If -i is in use then ONLY files and matching the given regular expression are being packed. If -x is given then files matching that regular expression are NOT being packed.',
        );

        return $args;
    }
    // }}}
    // {{{ function crxa_set_stub_begin
    /**
     * Set the stub
     */
    public function crxa_set_stub_begin(Crxa $crxa, $stub, $loader = NULL, $hashbang = NULL)
    {
        if (isset($stub)) {
            $c = file_get_contents($stub);

            if (substr($c, 0, 2) == '#!') {
                if (strpos($c, "\n") !== false) {
                    if (!isset($hashbang)) {
                        $hashbang = substr($c, 0, strpos($c, "\n") + 1);
                    }
                    $c = substr($c, strpos($c, "\n") + 1);
                } else {
                    if (!isset($hashbang)) {
                        $hashbang = $c;
                    }
                    $c = NULL;
                }
            }

            if (isset($hashbang)) {
                if (substr($hashbang, 0, 2) != '#!') {
                    $hashbang = '#!' . $hashbang;
                }
                if (substr($hashbang, -1) != "\n") {
                    $hashbang .= "\n";
                }
            } else {
                $hashbang = "";
            }

            if (isset($loader)) {
                $s = "<?crx if (!class_exists('CRX_Archive')) {\n?>";
                if (is_file($loader)) {
                    $s .= file_get_contents($loader);
                }
                $s .= "<?crx\n";
                $s .= "}\n";
                $s .= "if (!in_array('crxa', stream_get_wrappers())) {\n";
                $s .= "\tstream_wrapper_register('crxa', 'CRX_Archive');\n";
                $s .= "}\n";
                $s .= "if (!class_exists('Crxa',0)) {\n";
                $s .= "\tinclude 'crxa://'.__FILE__.'/crxa.inc';\n";
                $s .= "}\n";
                $s .= '?>';
                $s .= $c;

                $crxa->setStub($hashbang . $s);
            } else {
                $crxa->setStub($hashbang . $c);
            }
            return new SplFileInfo($stub);
        }
        return NULL;
    }
    // }}}
    // {{{ function crxa_set_stub_end
    /**
     * Set stub end
     */
    public function crxa_set_stub_end(Crxa $crxa, $stub, $loader = NULL)
    {
        if (isset($stub) && isset($loader)) {
            if (substr(__FILE__, -15) == 'crxacommand.inc') {
                self::crxa_add_file($crxa, 0, 'crxa.inc', 'crxa://'.__FILE__.'/crxa.inc', NULL);
            } else {
                self::crxa_add_file($crxa, 0, 'crxa.inc', dirname(__FILE__).'/crxa/crxa.inc', NULL);
            }
        }
    }
    // }}}
    // {{{ function cli_cmd_run_pack
    /**
     * Pack a new Crxa
     *
     * This function will try to pack a new Crxa archive.
     *
     * @see Exit to make sure that we are done.
     */
    public function cli_cmd_run_pack()
    {
        if (ini_get('crxa.readonly')) {
            self::error("Creating crxa files is disabled by ini setting 'crxa.readonly'.\n");
        }

        if (!Crxa::canWrite()) {
            self::error("Creating crxa files is disabled, Crxa::canWrite() returned false.\n");
        }

        $alias    = $this->args['a']['val'];
        $hashbang = $this->args['b']['val'];
        $archive  = $this->args['f']['val'];
        $hash     = $this->args['h']['val'];
        $privkey  = $this->args['y']['val'] ?? null;
        $regex    = $this->args['i']['val'];
        $level    = $this->args['l']['val'];
        $loader   = $this->args['p']['val'];
        $stub     = $this->args['s']['val'];
        $invregex = $this->args['x']['val'];
        $input    = $this->args['']['val'];

        $hash = self::crxa_check_hash($hash, $privkey);

        $crxa  = new Crxa($archive, 0, $alias);

        $crxa->startBuffering();

        $stub = $this->crxa_set_stub_begin($crxa, $stub, $loader, $hashbang);

        if (!is_array($input)) {
            $this->crxa_add($crxa, $level, $input, $regex, $invregex, $stub, NULL, isset($loader));
        } else {
            foreach($input as $i) {
                $this->crxa_add($crxa, $level, $i, $regex, $invregex, $stub, NULL, isset($loader));
            }
        }

        $this->crxa_set_stub_end($crxa, $stub, $loader);

        switch($this->args['c']['val']) {
            case 'gz':
            case 'gzip':
                $crxa->compressFiles(Crxa::GZ);
                break;
            case 'bz2':
            case 'bzip2':
                $crxa->compressFiles(Crxa::BZ2);
                break;
            default:
                $crxa->decompressFiles();
                break;
        }

        if ($hash) {
            $crxa->setSignatureAlgorithm($hash, $privkey);
        }

        $crxa->stopBuffering();
        exit(0);
    }
    // }}}
    // {{{ static function crxa_add
    /**
     * Add files to a crxa archive.
     *
     * This function will take a directory and iterate through
     * it and get the files to insert into the Crxa archive.
     *
     * @param Crxa        $crxa      The crxa object.
     * @param string      $input     The input directory
     * @param string      $regex     The regex used in RegexIterator.
     * @param string      $invregex  The InvertedRegexIterator expression.
     * @param SplFileInfo $stub Stub file object
     * @param mixed       $compress  Compression algorithm or NULL
     * @param boolean     $noloader  Whether to prevent adding the loader
     */
    static function crxa_add(Crxa $crxa, $level, $input, $regex, $invregex, SplFileInfo $stub = NULL, $compress = NULL, $noloader = false)
    {
        if ($input && is_file($input) && !is_dir($input)) {
            return self::crxa_add_file($crxa, $level, $input, $input, $compress);
        }
        $dir   = new RecursiveDirectoryIterator($input);
        $dir   = new RecursiveIteratorIterator($dir);

        if (isset($regex)) {
            $dir = new RegexIterator($dir, $regex);
        }

        if (isset($invregex)) {
            $dir = new InvertedRegexIterator($dir, $invregex);
        }

        try {
            foreach($dir as $file) {
                if ((empty($stub) || $file->getRealPath() != $stub->getRealPath()) && !is_dir($file)) {
                    self::crxa_add_file($crxa, $level, $dir->getSubPathName(), $file, $compress, $noloader);
                }
            }
        } catch(Exception $e) {
            self::error("Unable to complete operation on file '$file'\n" . $e->getMessage() . "\n");
        }
    }
    // }}}
    // {{{ static function crxa_add_file
    /**
     * Add a crxa file
     *
     * This function adds a file to a crxa archive.
     *
     * @param Crxa    $crxa      The crxa object
     * @param string  $level     The level of the file.
     * @param string  $entry     The entry point
     * @param string  $file      The file to add to the archive
     * @param string  $compress  The compression scheme for the file.
     * @param boolean $noloader  Whether to prevent adding the loader
     */
    static function crxa_add_file(Crxa $crxa, $level, $entry, $file, $compress, $noloader = false)
    {
        $entry = str_replace('//', '/', $entry);
        while($level-- > 0 && ($p = strpos($entry, '/')) !== false) {
            $entry = substr($entry, $p+1);
        }

    if ($noloader && $entry == 'crxa.inc') {
        return;
    }

        echo "$entry\n";

        $crxa[$entry] = file_get_contents($file);
        switch($compress) {
            case 'gz':
            case 'gzip':
                $crxa[$entry]->compress(Crxa::GZ);
                break;
            case 'bz2':
            case 'bzip2':
                $crxa[$entry]->compress(Crxa::BZ2);
                break;
            case '0':
                $crxa[$entry]->decompress();
                break;
            default:
                break;
        }
    }
    // }}}
    // {{{ public function crxa_dir_echo
    /**
     * Echo directory
     *
     * @param string $pn
     * @param unknown_type $f
     */
    public function crxa_dir_echo($pn, $f)
    {
        echo "$f\n";
    }
    // }}}
    // {{{ public function crxa_dir_operation
    /**
     * Directory operations
     *
     * Crxa directory operations.
     *
     * @param RecursiveIteratorIterator $dir  The recursiveIteratorIterator object.
     * @param string                    $func Function to call on the iterations
     * @param array                     $args Function arguments.
     */
    public function crxa_dir_operation(RecursiveIteratorIterator $dir, $func, array $args = array())
    {
        $regex   = $this->args['i']['val'];
        $invregex= $this->args['x']['val'];

        if (isset($regex)) {
            $dir = new RegexIterator($dir, $regex);
        }

        if (isset($invregex)) {
            $dir = new InvertedRegexIterator($dir, $invregex);
        }

        $any = false;
        foreach($dir as $pn => $f) {
            $any = true;
            call_user_func($func, $pn, $f, $args);
        }
        return $any;
    }
    // {{{ static function cli_cmd_inf_list
    /**
     * Cli Command Info List
     *
     * @return string What inf does
     */
    static function cli_cmd_inf_list()
    {
        return "List contents of a CRXA archive.";
    }
    // }}}
    // {{{ static function cli_cmd_arg_list
    /**
     * Cli Command Argument List
     *
     * @return arguments list
     */
    static function cli_cmd_arg_list()
    {
        return self::crxa_args('Fix', 'crxaurl');
    }
    // }}}
    // {{{ public function cli_cmd_run_list
    /**
     * Cli Command Run List
     *
     * @see $this->crxa_dir_operation
     */
    public function cli_cmd_run_list()
    {
        $this->crxa_dir_operation(
            new DirectoryTreeIterator(
                $this->args['f']['val']),
                array($this, 'crxa_dir_echo')
            );
    }
    // }}}
    // {{{ static function cli_command_inf_tree
    /**
     * Cli Command Inf Tree
     *
     * @return string  The description of a directory tree for a Crxa archive.
     */
    static function cli_cmd_inf_tree()
    {
        return "Get a directory tree for a CRXA archive.";
    }
    // }}}
    // {{{ static function cli_cmd_arg_tree
    /**
     * Cli Command Argument Tree
     *
     * @return string Arguments in URL format.
     */
    static function cli_cmd_arg_tree()
    {
        return self::crxa_args('Fix', 'crxaurl');
    }
    // }}}
    // {{{ public function cli_cmd_run_tree
    /**
     * Cli Command Run Tree
     *
     * Set the crxa_dir_operation with a directorygraphiterator.
     *
     * @see DirectoryGraphIterator
     * @see $this->crxa_dir_operation
     *
     */
    public function cli_cmd_run_tree()
    {
        $a = $this->crxa_dir_operation(
            new DirectoryGraphIterator(
                $this->args['f']['val']),
                array($this, 'crxa_dir_echo')
            );
        if (!$a) {
            echo "|-<root directory>\n";
        }
    }
    // }}}
    // {{{ cli_cmd_inf_extract
    /**
     * Cli Command Inf Extract
     *
     * @return string The description of the command extra to a directory.
     */
    static function cli_cmd_inf_extract()
    {
        return "Extract a CRXA package to a directory.";
    }
    // }}}
    // {{{ static function cli_cmd_arg_extract
    /**
     * Cli Command Arguments Extract
     *
     * The arguments for the extract function.
     *
     * @return array  The arguments for the extraction.
     */
    static function cli_cmd_arg_extract()
    {
        $args = self::crxa_args('Fix', 'crxa');

        $args[''] = array(
            'type' => 'dir',
            'val' => '.',
            'inf' => '         Directory to extract to (defaults to \'.\').',
        );

        return $args;
    }
    // }}}
    // {{{ public function cli_cmd_run_extract
    /**
     * Run Extract
     *
     * Run the extraction of a crxa Archive.
     *
     * @see $this->crxa_dir_operation
     */
    public function cli_cmd_run_extract()
    {
        $dir = $this->args['']['val'];

        if (is_array($dir)) {
            if (count($dir) != 1) {
                self::error("Only one target directory allowed.\n");
            } else {
                $dir = $dir[0];
            }
        }

        $crxa = $this->args['f']['val'];
        $base = $crxa->getPathname();
        $bend = strpos($base, '.crxa');
        $bend = strpos($base, '/', $bend);
        $base = substr($base, 0, $bend + 1);
        $blen = strlen($base);

        $this->crxa_dir_operation(
            new RecursiveIteratorIterator($crxa),
            array($this, 'crxa_dir_extract'),
            array($blen, $dir)
        );
    }
    // }}}
    // {{{ public function crxa_dir_extract
    /**
     * Extract to a directory
     *
     * This function will extract the content of a Crxa
     * to a directory and create new files and directories
     * depending on the permissions on that folder.
     *
     * @param string $pn
     * @param string $f     The file name
     * @param array $args   The directory and Blen information
     */
    public function crxa_dir_extract($pn, $f, $args)
    {
        $blen   = $args[0];
        $dir    = $args[1];
        $sub    = substr($pn, $blen);
        $target = $dir . '/' . $sub;

        if (!file_exists(dirname($target))) {
            @mkdir(dirname($target), 0777, true);
        }
        if (!file_exists(dirname($target))) {
            self::error("Operation could not be completed\n");
        }

        echo "$sub";

        if (!@copy($f, $target)) {
            echo " ...error\n";
        } else {
            echo " ...ok\n";
        }
    }
    // }}}
    // {{{ static function cli_cmd_inf_delete
    /**
     * Delete an entry from a crxa information.
     *
     * @return string The information
     */
    static function cli_cmd_inf_delete()
    {
        return 'Delete entry from a CRXA archive';
    }
    // }}}
    // {{{ static function cli_cmd_arg_delete
    /**
     * The cli command argument for deleting.
     *
     * @return array information about the arguments to use.
     */
    static function cli_cmd_arg_delete()
    {
        return self::crxa_args('FE', 'crxa');
    }
    // }}}
    // {{{ public function cli_cmd_run_delete
    /**
     * Deleting execution
     *
     * Execute the deleting of the file from the crxa archive.
     */
    public function cli_cmd_run_delete()
    {
        $crxa  = $this->args['f']['val'];
        $entry = $this->args['e']['val'];

        $crxa->startBuffering();
        unset($crxa[$entry]);
        $crxa->stopBuffering();
    }
    // }}}
    // {{{ static function cli_cmd_inf_add
    /**
     * Client comment add file information
     *
     * @return string The description of the feature
     */
    static function cli_cmd_inf_add()
    {
        return "Add entries to a CRXA package.";
    }
    // }}}
    // {{{ static function cli_cmd_arg_add
    /**
     * Add a file arguments
     */
    static function cli_cmd_arg_add()
    {
        $args = self::crxa_args('acFilx', 'crxa');
        $args[''] = array(
            'type'     => 'any',
            'val'      => NULL,
            'required' => 1,
            'inf'      => '         Any number of input files and directories. If -i is in use then ONLY files and matching the given regular expression are being packed. If -x is given then files matching that regular expression are NOT being packed.',
        );
        return $args;
    }
    // }}}
    // {{{ public functio cli_cmd_run_add
    /**
     * Add a file
     *
     * Run the action of adding a file to
     * a crxa archive.
     */
    public function cli_cmd_run_add()
    {
        $compress= $this->args['c']['val'];
        $crxa    = $this->args['f']['val'];
        $regex   = $this->args['i']['val'];
        $level   = $this->args['l']['val'];
        $invregex= $this->args['x']['val'];
        $input   = $this->args['']['val'];

        $crxa->startBuffering();

        if (!is_array($input)) {
            $this->crxa_add($crxa, $level, $input, $regex, $invregex, NULL, $compress);
        } else {
            foreach($input as $i) {
                $this->crxa_add($crxa, $level, $i, $regex, $invregex, NULL, $compress);
            }
        }
        $crxa->stopBuffering();
        exit(0);
    }
    // }}}
    // {{{ public function cli_cmd_inf_stub_set
    /**
     * Set the stup of a crxa file.
     *
     * @return string The stub set description.
     */
    public function cli_cmd_inf_stub_set()
    {
        return "Set the stub of a CRXA file. " .
               "If no input file is specified as stub then stdin is being used.";
    }
    // }}}
    // {{{ public function cli_cmd_arg_stub_set
    /**
     * Set the argument stub
     *
     * @return string arguments for a stub
     */
    public function cli_cmd_arg_stub_set()
    {
        $args = self::crxa_args('bFps', 'crxa');
        $args['s']['val'] = 'crx://stdin';
        return $args;
    }
    // }}}
    // {{{ public function cli_cmd_run_stub_set
    /**
     * Cli Command run stub set
     *
     * @see   $crxa->setStub()
     */
    public function cli_cmd_run_stub_set()
    {
        $hashbang = $this->args['b']['val'];
        $crxa     = $this->args['f']['val'];
        $stub     = $this->args['s']['val'];
        $loader   = $this->args['p']['val'];

        $this->crxa_set_stub_begin($crxa, $stub, $loader, $hashbang);
        $this->crxa_set_stub_end($crxa, $stub, $loader);
    }
    // }}}
    // {{{ public function cli_cmd_inf_stub_get
    /**
     * Get the command stub infos.
     *
     * @return string a description of the stub of a Crxa file.
     */
    public function cli_cmd_inf_stub_get()
    {
        return "Get the stub of a CRXA file. " .
               "If no output file is specified as stub then stdout is being used.";
    }
    // }}}
    // {{{ public function cli_cmd_arg_stub_get
    /**
     * Get the argument stub
     *
     * @return array $args The arguments passed to the stub.
     */
    public function cli_cmd_arg_stub_get()
    {
        $args = self::crxa_args('Fs', 'crxa');
        $args['s']['val'] = 'crx://stdin';
        return $args;
    }
    // }}}
    // {{{ public function cli_cmd_run_stub_get
    /**
     * Cli Command Run Stub
     *
     * Get arguments and store them into a stub.
     *
     * @param arguments $args
     * @see   $this->args
     */
    public function cli_cmd_run_stub_get($args)
    {
        $crxa = $this->args['f']['val'];
        $stub = $this->args['s']['val'];

        file_put_contents($stub, $crxa->getStub());
    }
    // }}}
    // {{{ public function cli_cmd_inf_compress
    /**
     * Cli Command Inf Compress
     *
     * Cli Command compress information
     *
     * @return string A description of the command.
     */
    public function cli_cmd_inf_compress()
    {
        return "Compress or uncompress all files or a selected entry.";
    }
    // }}}
    // {{{ public function cli_cmd_arg_cmpress
    /**
     * Cli Command Arg Compress
     *
     * @return array The arguments for compress
     */
    public function cli_cmd_arg_compress()
    {
        return self::crxa_args('FCe', 'crxa');
    }
    // }}}
    // {{{ public function cli_cmd_run_compress
    /**
     * Cli Command Run Compress
     *
     * @see $this->args
     */
    public function cli_cmd_run_compress()
    {
        $crxa  = $this->args['f']['val'];
        $entry = $this->args['e']['val'];

        switch($this->args['c']['val']) {
            case 'gz':
            case 'gzip':
                if (isset($entry)) {
                    $crxa[$entry]->compress(Crxa::GZ);
                } else {
                    $crxa->compressFiles(Crxa::GZ);
                }
                break;
            case 'bz2':
            case 'bzip2':
                if (isset($entry)) {
                    $crxa[$entry]->compress(Crxa::BZ2);
                } else {
                    $crxa->compressFiles(Crxa::BZ2);
                }
                break;
            default:
                if (isset($entry)) {
                    $crxa[$entry]->decompress();
                } else {
                    $crxa->decompressFiles();
                }
                break;
        }
    }
    // }}}
    // {{{ public function cli_cmd_inf_sign
    /**
     * Cli Command Info Signature
     *
     * @return string A description of the signature arguments.
     */
    public function cli_cmd_inf_sign()
    {
        return "Set signature hash algorithm.";
    }
    // }}}
    // {{{ public function cli_cmd_arg_sign
    /**
     * Cli Command Argument Sign
     *
     * @return array Arguments for Signature
     */
    public function cli_cmd_arg_sign()
    {
        return self::crxa_args('FHy', 'crxa');
    }
    // }}}
    // {{{ public function cli_cmd_run_sign
    /**
     * Cli Command Run Signature
     *
     * @see $crxa->setSignaturealgorithm
     */
    public function cli_cmd_run_sign()
    {
        $crxa     = $this->args['f']['val'];
        $hash     = $this->args['h']['val'];
        $privkey  = $this->args['y']['val'];

        $hash = self::crxa_check_hash($hash, $privkey);

        $crxa->setSignatureAlgorithm($hash, $privkey);
    }
    // }}}
    // {{{ public function cli_cmd_inf_meta_set
    /**
     * Cli Command Inf Meta Set
     *
     * @return string A description
     */
    public function cli_cmd_inf_meta_set()
    {
        return "Set meta data of a CRXA entry or a CRXA package using serialized input. " .
               "If no input file is specified for meta data then stdin is being used." .
               "You can also specify a particular index using -k. In that case the metadata is " .
               "expected to be an array and the value of the given index is being set. If " .
               "the metadata is not present or empty a new array will be created. If the " .
               "metadata is present and a flat value then the return value is 1. Also using -k " .
               "the input is been taken directly rather then being serialized.";
    }
    // }}}
    // {{{ public function cli_cmd_arg_meta_set
    /**
     * Cli Command Argument Meta Set
     *
     * @return array  The arguments for meta set
     */
    public function cli_cmd_arg_meta_set()
    {
        return self::crxa_args('FekM', 'crxa');
    }
    // }}}
    // {{{ public function cli_cmd_run_met_set
    /**
     * Cli Command Run Metaset
     *
     * @see $crxa->startBuffering
     * @see $crxa->setMetadata
     * @see $crxa->stopBuffering
     */
    public function cli_cmd_run_meta_set()
    {
        $crxa  = $this->args['f']['val'];
        $entry = $this->args['e']['val'];
        $index = $this->args['k']['val'];
        $meta  = $this->args['m']['val'];

        $crxa->startBuffering();

        if (isset($index)) {
            if (isset($entry)) {
                if ($crxa[$entry]->hasMetadata()) {
                    $old = $crxa[$entry]->getMetadata();
                } else {
                    $old = array();
                }
            } else {
                if ($crxa->hasMetadata()) {
                    $old = $crxa->getMetadata();
                } else {
                    $old = array();
                }
            }

            if (!is_array($old)) {
                self::error('Metadata is a flat value while an index operation was issued.');
            }

            $old[$index] = $meta;
            $meta = $old;
        } else {
            $meta = unserialize($meta);
        }

        if (isset($entry)) {
            $crxa[$entry]->setMetadata($meta);
        } else {
            $crxa->setMetadata($meta);
        }
        $crxa->stopBuffering();
    }
    // }}}
    // {{{ public function cli_cmd_inf_met_get
    /**
     * Cli Command Inf Metaget
     *
     * @return string A description of the metaget arguments
     */
    public function cli_cmd_inf_meta_get()
    {
        return "Get meta information of a CRXA entry or a CRXA package in serialized form. " .
               "If no output file is specified for meta data then stdout is being used.\n" .
               "You can also specify a particular index using -k. In that case the metadata is " .
               "expected to be an array and the value of the given index is returned using echo " .
               "rather than using serialize. If that index does not exist or no meta data is " .
               "present then the return value is 1.";
    }
    // }}}
    // {{{ public function cli_cmd_arg_meta_get
    /**
     * Cli Command arg metaget
     *
     * @return array  The arguments for meta get.
     */
    public function cli_cmd_arg_meta_get()
    {
        return self::crxa_args('Fek', 'crxa');
    }
    // }}}
    // {{{ public function cli_cmd_run_meta_get
    /**
     * Cli Command Run Metaget
     *
     * @see $this->args
     * @see $crxa[$x]->hasMetadata()
     * @see $crxa->getMetadata()
     */
    public function cli_cmd_run_meta_get()
    {
        $crxa  = $this->args['f']['val'];
        $entry = $this->args['e']['val'];
        $index = $this->args['k']['val'];

        if (isset($entry)) {
            if (!$crxa[$entry]->hasMetadata()) {
                echo "No Metadata\n";
                exit(1);
            }
            echo serialize($crxa[$entry]->getMetadata());
        } else {
            if (!$crxa->hasMetadata()) {
                echo "No Metadata\n";
                exit(1);
            }
            $meta = $crxa->getMetadata();
        }

        if (isset($index)) {
            if (isset($index)) {
                if (isset($meta[$index])) {
                    echo $meta[$index];
                    exit(0);
                } else {
                    echo "No Metadata\n";
                    exit(1);
                }
            } else {
                echo serialize($meta);
            }
        }
    }
    // }}}
    // {{{ public function cli_cmd_inf_meta_del
    /**
     * Cli Command Inf Metadel
     *
     * @return string A description of the metadel function
     */
    public function cli_cmd_inf_meta_del()
    {
        return "Delete meta information of a CRXA entry or a CRXA package.\n" .
               "If -k is given then the metadata is expected to be an array " .
               "and the given index is being deleted.\n" .
               "If something was deleted the return value is 0 otherwise it is 1.";
    }
    // }}}
    // {{{ public function cli_cmd_arg_meta_del
    /**
     * CliC ommand Arg Metadelete
     *
     * @return array The arguments for metadel
     */
    public function cli_cmd_arg_meta_del()
    {
        return self::crxa_args('Fek', 'crxa');
    }
    // }}}
    // {{{ public function cli_cmd_run_meta_del
    /**
     * Cli Command Run MetaDel
     *
     * @see $crxa[$x]->delMetadata()
     * @see $crxa->delMetadata()
     */
    public function cli_cmd_run_meta_del()
    {
        $crxa  = $this->args['f']['val'];
        $entry = $this->args['e']['val'];
        $index = $this->args['k']['val'];

        if (isset($entry)) {
            if (isset($index)) {
                if (!$crxa[$entry]->hasMetadata()) {
                    exit(1);
                }
                $meta = $crxa[$entry]->getMetadata();

                // @todo add error message here.
                if (!is_array($meta)) {
                    exit(1);
                }

                unset($meta[$index]);
                $crxa[$entry]->setMetadata($meta);
            } else {
                exit($crxa[$entry]->delMetadata() ? 0 : 1);
            }
        } else {
            if (isset($index)) {
                if (!$crxa->hasMetadata()) {
                    exit(1);
                }

                $meta = $crxa->getMetadata();

                // @todo Add error message
                if (!is_array($meta)) {
                    exit(1);
                }

                unset($meta[$index]);
                $crxa->setMetadata($meta);
            } else {
                exit($crxa->delMetadata() ? 0 : 1);
            }
        }
    }
    // }}}
    // {{{ public function cli_cmd_inf_info
    /**
     * CLi Command Inf Info
     *
     * @return string A description about the info commands.
     */
    public function cli_cmd_inf_info()
    {
        return "Get information about a CRXA package.\n" .
               "By using -k it is possible to return a single value.";
    }
    // }}}
    // {{{ public function cli_cmd_arg_info
    /**
     * Cli Command Arg Infos
     *
     * @return array The arguments for info command.
     */
    public function cli_cmd_arg_info()
    {
        return self::crxa_args('Fk', 'crxa');
    }
    // }}}
    // {{{ public function cli_cmd_run_info
    /**
     * Cli Command Run Info
     *
     * @param args $args
     */
    public function cli_cmd_run_info()
    {
        $crxa  = $this->args['f']['val'];
        $index = $this->args['k']['val'];

        $hash  = $crxa->getSignature();
        $infos = array();

        if ($crxa->getAlias()) {
            $infos['Alias'] = $crxa->getAlias();
        }

        if (!$hash) {
            $infos['Hash-type'] = 'NONE';
        } else {
            $infos['Hash-type'] = $hash['hash_type'];
            $infos['Hash'] = $hash['hash'];
        }

        $csize   = 0;
        $usize   = 0;
        $count   = 0;
        $ccount  = 0;
        $ucount  = 0;
        $mcount  = 0;
        $compalg = array('GZ'=>0, 'BZ2'=>0);

        foreach(new RecursiveIteratorIterator($crxa) as $ent) {
            $count++;
            if ($ent->isCompressed()) {
                $ccount++;
                $csize += $ent->getCompressedSize();
                if ($ent->isCompressed(Crxa::GZ)) {
                    $compalg['GZ']++;
                } elseif ($ent->isCompressed(Crxa::BZ2)) {
                    $compalg['BZ2']++;
                }
            } else {
                $ucount++;
                $csize += $ent->getSize();
            }

            $usize += $ent->getSize();

            if ($ent->hasMetadata()) {
                $mcount++;
            }
        }

        $infos['Entries']            = $count;
        $infos['Uncompressed-files'] = $ucount;
        $infos['Compressed-files']   = $ccount;
        $infos['Compressed-gz']      = $compalg['GZ'];
        $infos['Compressed-bz2']     = $compalg['BZ2'];
        $infos['Uncompressed-size']  = $usize;
        $infos['Compressed-size']    = $csize;
        $infos['Compression-ratio']  = sprintf('%.3g%%', $usize ? ($csize * 100) / $usize : 100);
        $infos['Metadata-global']    = $crxa->hasMetadata() * 1;
        $infos['Metadata-files']     = $mcount;
        $infos['Stub-size']          = strlen($crxa->getStub());

        if (isset($index)) {
            if (!isset($infos[$index])) {
                self::error("Requested value does not exist.\n");
            }

            echo $infos[$index];
            exit(0);
        }

        $l = 0;
        foreach($infos as $which => $val) {
            $l = max(strlen($which), $l);
        }

        foreach($infos as $which => $val) {
            echo $which . ':' . str_repeat(' ', $l + 1 - strlen($which)) . $val . "\n";
        }
    }
    // }}}
    // {{{ public function cli_cmd_inf_version
    /**
     * CLi Command Inf Version
     *
     * @return string A description about the info commands.
     */
    public function cli_cmd_inf_version()
    {
        return "Get information about the CRXA environment and the tool version.";
    }
    // }}}
    // {{{ public function cli_cmd_arg_version
    /**
     * Cli Command Arg Version
     *
     * @return array The arguments for version command.
     */
    public function cli_cmd_arg_version()
    {
        return self::crxa_args('', NULL);
    }
    // }}}
    // {{{ public function cli_cmd_run_info
    /**
     * Cli Command Run Info
     *
     * @param args $args
     */
    public function cli_cmd_run_version()
    {
        $use_ext = extension_loaded('crxa');
        $version = array(
            'CRX Version' => crxversion(),
            'crxa.crxa version' => '$Id: 17c5c6051fafe1e675ff9f6b7fc98afd448b8521 $',
            'Crxa EXT version' => $use_ext ? crxversion('crxa') : 'Not available',
            'Crxa API version' => Crxa::apiVersion(),
            'Crxa-based crxa archives' => true,
            'Tar-based crxa archives' => $use_ext,
            'ZIP-based crxa archives' => $use_ext,
            'gzip compression' => extension_loaded('zlib'),
            'bzip2 compression' => extension_loaded('bz2'),
            'supported signatures' => $use_ext ? join(', ', Crxa::getSupportedSignatures()) : '',
            );
        $klen = 0;
        foreach($version as $k => $v)
        {
            $klen = max($klen, strlen($k));
        }
        ++$klen;
        foreach($version as $k => $v) {
            if (is_bool($v)) {
                $v = $v ? 'enabled' : 'disabled';
            }
            printf("%-{$klen}s  %s\n", $k.':', $v);
        }
    }
    // }}}
}
// }}}
?>
