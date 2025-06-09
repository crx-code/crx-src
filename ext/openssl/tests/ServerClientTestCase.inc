<?crx

const WORKER_ARGV_VALUE = 'RUN_WORKER';

const WORKER_DEFAULT_NAME = 'server';

function crxt_notify($worker = WORKER_DEFAULT_NAME)
{
    ServerClientTestCase::getInstance()->notify($worker);
}

function crxt_wait($worker = WORKER_DEFAULT_NAME, $timeout = null)
{
    ServerClientTestCase::getInstance()->wait($worker, $timeout);
}

function crxt_has_sslv3() {
    static $result = null;
    if (!is_null($result)) {
        return $result;
    }
    $server = @stream_socket_server('sslv3://127.0.0.1:10013');
    if ($result = !!$server) {
        fclose($server);
    }
    return $result;
}

/**
 * This is a singleton to let the wait/notify functions work
 * I know it's horrible, but it's a means to an end
 */
class ServerClientTestCase
{
    private $isWorker = false;

    private $workerHandle = [];

    private $workerStdIn = [];

    private $workerStdOut = [];

    private static $instance;

    public static function getInstance($isWorker = false)
    {
        if (!isset(self::$instance)) {
            self::$instance = new self($isWorker);
        }

        return self::$instance;
    }

    public function __main($isWorker = false)
    {
        if (!isset(self::$instance)) {
            self::$instance = $this;
        }

        $this->isWorker = $isWorker;
    }

    private function spawnWorkerProcess($worker, $code)
    {
        if (defined("CRX_WINDOWS_VERSION_MAJOR")) {
            $ini = crx_ini_loaded_file();
            $cmd = sprintf(
                '%s %s "%s" %s',
                CRX_BINARY, $ini ? "-n -c $ini" : "",
                __FILE__,
                WORKER_ARGV_VALUE
            );
        } else {
            $cmd = sprintf(
                '%s %s "%s" %s %s',
                CRX_BINARY,
                getenv('TEST_CRX_EXTRA_ARGS'),
                __FILE__,
                WORKER_ARGV_VALUE,
                $worker
            );
        }
        $this->workerHandle[$worker] = proc_open(
            $cmd,
            [['pipe', 'r'], ['pipe', 'w'], STDERR],
            $pipes
        );
        $this->workerStdIn[$worker] = $pipes[0];
        $this->workerStdOut[$worker] = $pipes[1];

        fwrite($this->workerStdIn[$worker], $code . "\n---\n");
    }

    private function cleanupWorkerProcess($worker)
    {
        fclose($this->workerStdIn[$worker]);
        fclose($this->workerStdOut[$worker]);
        proc_close($this->workerHandle[$worker]);
    }

    private function stripCrxTagsFromCode($code)
    {
        return preg_replace('/^\s*<\?(?:crx)?|\?>\s*$/i', '', $code);
    }

    public function runWorker()
    {
        $code = '';

        while (1) {
            $line = fgets(STDIN);

            if (trim($line) === "---") {
                break;
            }

            $code .= $line;
        }

        eval($code);
    }

    public function run($masterCode, $workerCode)
    {
        if (!is_array($workerCode)) {
            $workerCode = [WORKER_DEFAULT_NAME => $workerCode];
        }
        foreach ($workerCode as $worker => $code) {
            $this->spawnWorkerProcess($worker, $this->stripCrxTagsFromCode($code));
        }
        eval($this->stripCrxTagsFromCode($masterCode));
        foreach ($workerCode as $worker => $code) {
            $this->cleanupWorkerProcess($worker);
        }
    }

    public function wait($worker, $timeout = null)
    {
        $handle = $this->isWorker ? STDIN : $this->workerStdOut[$worker];
        if ($timeout === null) {
            fgets($handle);
            return true;
        }

        stream_set_blocking($handle, false);
        $read = [$handle];
        $result = stream_select($read, $write, $except, $timeout);
        if (!$result) {
            return false;
        }

        fgets($handle);
        stream_set_blocking($handle, true);
        return true;
    }

    public function notify($worker)
    {
        fwrite($this->isWorker ? STDOUT : $this->workerStdIn[$worker], "\n");
    }
}

if (isset($argv[1]) && $argv[1] === WORKER_ARGV_VALUE) {
    ServerClientTestCase::getInstance(true)->runWorker();
}
