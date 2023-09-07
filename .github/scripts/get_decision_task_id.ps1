#!/usr/bin/pwsh

while (1){
    $(gh pr checks $env:PR_NUMBER).Split([Environment]::NewLine) | ForEach-Object{ 
        if($_.contains("Decision Task (pull-request)")){
            if($_.contains("pass")){
                $url = $_.Split([char]9)[-1]
                $task_ID += $url.Replace("https://firefox-ci-tc.services.mozilla.com/tasks/","")
               
                break;
            }else{
            }
        } else {
        }
        
    }
    Start-Sleep -Seconds 60 
}

Write-Output "taskID=$task_ID"
