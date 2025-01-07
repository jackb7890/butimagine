foreach ($drive in (Get-PSDrive)) {
    $root = $drive.root
    if (!($root)) {
        continue
    }
    $path = "${root}Program Files\Microsoft Visual Studio\2022\Community\VC\"
    if(Test-Path -Path $path) {
        echo "`"${path}Auxiliary\Build\vcvars64.bat`""
        break
    }
}
