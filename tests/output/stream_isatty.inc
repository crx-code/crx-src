<?crx

function testToStdOut()
{
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
        echo "$name: "; var_dump(stream_isatty($stream));
    }

    fclose($closeMe);
}

function testToStdErr()
{
    ob_start();
    testToStdOut();
    $result = ob_get_contents();
    ob_end_clean();
    fwrite(STDERR, $result);
}
