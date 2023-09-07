#!/usr/bin/pwsh

$script:Task_ID=""

while (1){
    $(gh pr checks $env:PR_NUMBER).Split([Environment]::NewLine) | ForEach-Object{ 
        if(!$_.contains("Decision Task")){
            Write-Output "no task"
            return;
        }
        if(!$_.contains("pass")){
            Write-Output "not ready"
            return;
        }
        Write-Output $_
        $id = $_.Split("/")[-1]    
        Write-Output $url
        if($id -eq ""){
            Write-Output "empty"
            return;
        }
        Write-Output "taskID=${id}"
        $script:Task_ID=$id
        Write-Output "$script:Task_ID"
        break;
    }
    Start-Sleep -Seconds 30 
}

Write-Output "taskID=$script:Task_ID"
return 0
