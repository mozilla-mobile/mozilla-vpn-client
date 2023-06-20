
# Invokes a .bat file, 
# after the bat is done, it will apply changes to the 
# env to the current powershell session. 
# 

$bat = $args[0]

cmd.exe /c "call `"$bat`" && set > %temp%\conda_env.txt"
    Get-Content "$env:temp\conda_env.txt" | Foreach-Object {
      if ($_ -match "^(.*?)=(.*)$") {
          Set-Content "env:\$($matches[1])" $matches[2]
      }
    }
write-host "`n ($bat ) variables set." -ForegroundColor Yellow
  
