while (1){
    $(gh pr checks).Split([Environment]::NewLine) | ForEach-Object{ 
        Write-Output $_
        if($_.contains("Decision Task (pull-request)")){
            if($_.contains("pass")){
                $url = $_.Split([char]9)[-1]
                $task_ID += $url.Replace("https://firefox-ci-tc.services.mozilla.com/tasks/","")
                Write-Output $task_ID
                Write-Output "taskID=$task_ID" | Out-File -FilePath $env:GITHUB_OUTPUT
                break;
            }
        }
        
    }
    Write-Output "Decision Task not found, sleeping 1s"
    Start-Sleep -Seconds 1 
}
