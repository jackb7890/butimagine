Param(
    $timeout = 10
)

# launch the server

# assume server.exe is on our path

$process start-process -filepath server.exe

write-out $process.Id