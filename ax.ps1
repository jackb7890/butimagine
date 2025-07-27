$codeCont = Get-Content -path ".\c"
$namesCont = Get-Content -path ".\b"
$count = 0
$bigStr = ""
foreach ($line in $codeCont) {
    $str = $codeline -replace "#" $namesCont[$count]
    $bigStr += $str + "\n"
    $count = $count + 1
}

$bigStr | Out-file -path "output.txt"