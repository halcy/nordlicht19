$citra =  'C:\Users\halcy\AppData\Local\citra\app-0.1.463\citra-qt.exe'

$watcher = New-Object System.IO.FileSystemWatcher
$watcher.Path = get-location
$watcher.Filter = "*.3dsx"
$watcher.IncludeSubdirectories = $false
$watcher.EnableRaisingEvents = $true
$watcher.NotifyFilter = [System.IO.NotifyFilters]::LastWrite -bor [System.IO.NotifyFilters]::FileName

while($TRUE){
    $result = $watcher.WaitForChanged([System.IO.WatcherChangeTypes]::Changed -bor [System.IO.WatcherChangeTypes]::Renamed -bOr [System.IO.WatcherChangeTypes]::Created, 1000);
    if($result.TimedOut){
        continue;
    }
    write-host "ROM updated:" $result.Name
    Start-Process $citra $result.Name -Wait
}
