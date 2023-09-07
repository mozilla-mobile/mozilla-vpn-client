#!/usr/bin/pwsh

$script:Task_ID=""

while (1){
    $(gh pr checks $env:PR_NUMBER).Split([Environment]::NewLine) | ForEach-Object{ 
        if(!$_.contains("Decision Task")){
            return;
        }
        if(!$_.contains("pass")){
            return;
        }
        $url = $_.Split([char]9)[-1]
        $script:Task_ID = $url.Replace("https://firefox-ci-tc.services.mozilla.com/tasks/","")
        if($script:Task_ID -eq ""){
            return;
        }
        break;
    }
    Start-Sleep -Seconds 60 
}

Write-Output "taskID=$script:Task_ID"
