<?crx

function resetVT100State()
{
    $state = array(
        sapi_windows_vt100_support(STDIN),
        sapi_windows_vt100_support(STDOUT),
        sapi_windows_vt100_support(STDERR),
    );
    sapi_windows_vt100_support(STDIN, false);
    sapi_windows_vt100_support(STDOUT, false);
    sapi_windows_vt100_support(STDERR, false);

    return $state;
}

function restoreVT100State(array $state)
{
    sapi_windows_vt100_support(STDIN, $state[0]);
    sapi_windows_vt100_support(STDOUT, $state[1]);
    sapi_windows_vt100_support(STDERR, $state[2]);
}

function testToStdOut()
{
    $state = resetVT100State();

    $sampleStreams = array(
        'STDIN (constant)'              => STDIN,
        'STDIN (fopen)'                 => fopen('crx://stdin', 'rb'),
        'STDIN (crx://fd/0)'            => fopen('crx://fd/0', 'rb'),
        'STDOUT (constant)'             => STDOUT,
        'STDOUT (fopen)'                => fopen('crx://stdout', 'wb'),
        'STDOUT (crx://fd/1)'           => fopen('crx://fd/1', 'wb'),
        'STDERR (constant)'             => STDERR,
        'STDERR (fopen)'                => fopen('crx://stderr', 'wb'),
        'STDERR (crx://fd/2)'           => fopen('crx://fd/2', 'wb'),
        'Invalid stream (crx://temp)'   => fopen('crx://temp', 'wb'),
        'Invalid stream (crx://input)'  => fopen('crx://input', 'wb'),
        'Invalid stream (crx://memory)' => fopen('crx://memory', 'wb'),
        'File stream'                   => $closeMe = fopen(__FILE__, 'rb'),
    );

    foreach ($sampleStreams as $name => $stream) {
        echo "$name:\n";
        echo "- current value  : "; var_dump(sapi_windows_vt100_support($stream));
        echo "- enabling VT100 : "; var_dump(sapi_windows_vt100_support($stream, true));
        echo "- current value  : "; var_dump(sapi_windows_vt100_support($stream));
        echo "- disabling VT100: "; var_dump(sapi_windows_vt100_support($stream, false));
        echo "- current value  : "; var_dump(sapi_windows_vt100_support($stream));
    }

    fclose($closeMe);
    restoreVT100State($state);
}

function testToStdErr()
{
    ob_start();
    testToStdOut();
    $result = ob_get_contents();
    ob_end_clean();
    fwrite(STDERR, $result);
}
