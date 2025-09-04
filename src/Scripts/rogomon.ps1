# When running Rog-O-Matic in a loop, sometimes the game hangs.
# This script terminates the process if it's been idle for too long.

$targetImageName = "RogueCollection.exe"
$checkInterval = 5
$maxStagnantTime = 10
$processStats = @{}

while ($true) {
    Write-Host ""
    Write-Host "--- Iteration at " + (Get-Date -Format 'HH:mm:ss') + " ---"

    # Get all processes matching the image name
    $processes = Get-WmiObject Win32_Process | Where-Object {
        $_.Name -ieq $targetImageName
    }

    foreach ($proc in $processes) {
        $procId = $proc.ProcessId
        $readBytes = [int64]$proc.ReadTransferCount

        if ($processStats.ContainsKey($procId)) {
            $prev = $processStats[$procId]
            if ($prev.ReadBytes -eq $readBytes) {
                $prev.StagnantTime += $checkInterval
            } else {
                $prev.ReadBytes = $readBytes
                $prev.StagnantTime = 0
            }

            Write-Host ("Process {0}: TotalReadBytes = {1}, StagnantTime = {2}s" -f $procId, $readBytes, $prev.StagnantTime)

            if ($prev.StagnantTime -ge $maxStagnantTime) {
                Write-Host ("WARNING: Process {0} has been stagnant for {1}s - terminating..." -f $procId, $prev.StagnantTime)
                #Stop-Process -Id $procId -Force
                $x = Get-WmiObject Win32_Process -Filter "ProcessId = $procId"
                $x.Terminate()
            }
        } else {
            $processStats[$procId] = @{
                ReadBytes = $readBytes
                StagnantTime = 0
            }
            Write-Host ("Process {0}: First seen, TotalReadBytes = {1}" -f $procId, $readBytes)
        }
    }

    # Clean up stats for processes that no longer exist
    $activeProcIds = $processes | ForEach-Object { $_.ProcessId }
    $trackedIds = @($processStats.Keys)  # Clone keys to avoid modification during iteration

    foreach ($trackedId in $trackedIds) {
        if (-not ($activeProcIds -contains $trackedId)) {
            Write-Host ("Process {0} no longer active - removing from tracking" -f $trackedId)
            $processStats.Remove($trackedId)
        }
    }

    Start-Sleep -Seconds $checkInterval
}