; Stands in for CPack's AddToPath and un.RemoveFromPath (see
; nsis_template.cmake). Those edit PATH in NSIS strings, which top out at 1024
; characters, so on any machine with a longer PATH they give up with "PATH too
; long". Here PowerShell edits the raw registry value instead: the PATH never
; passes through an NSIS string, and %VAR% references and REG_EXPAND_SZ are kept.
; The directory and scope go in through environment variables, so a path is
; never quoted into the script.

!define BPQ_PATH_PS `"$SYSDIR\WindowsPowerShell\v1.0\powershell.exe" -NoProfile -NonInteractive -ExecutionPolicy Bypass -Command "$$ErrorActionPreference='Stop'; $$d=$$env:BPQ_DIR.TrimEnd('\'); $$k=if($$env:BPQ_SCOPE -eq 'Machine'){[Microsoft.Win32.Registry]::LocalMachine.OpenSubKey('SYSTEM\CurrentControlSet\Control\Session Manager\Environment',$$true)}else{[Microsoft.Win32.Registry]::CurrentUser.CreateSubKey('Environment')}; $$p=@($$k.GetValue('Path','','DoNotExpandEnvironmentNames') -split ';' | ?{$$_}); $$q=@($$p | ?{$$_.TrimEnd('\') -ne $$d}); `

; Runs the script above, with TAIL appended, against the directory in $0.
; Clobbers $1.
!macro BPQ_EDIT_PATH TAIL
  StrCpy $1 "User"
  StrCmp $ADD_TO_PATH_ALL_USERS "1" 0 +2
    StrCpy $1 "Machine"
  System::Call 'Kernel32::SetEnvironmentVariable(t "BPQ_DIR", t "$0")'
  System::Call 'Kernel32::SetEnvironmentVariable(t "BPQ_SCOPE", t "$1")'
  DetailPrint "Updating the $1 PATH for $0"
  nsExec::ExecToLog `${BPQ_PATH_PS}${TAIL}"`
  Pop $1
  StrCmp $1 "0" +2
    MessageBox MB_OK|MB_ICONEXCLAMATION "Could not update PATH (PowerShell returned $1). Add $0 to PATH by hand to use the command line." /SD IDOK
  SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000
!macroend

; AddToPath - appends the directory on the stack to PATH, once.
Function AddToPath
  Exch $0
  Push $1

  # don't add if the path doesn't exist
  IfFileExists "$0\*.*" 0 AddToPath_done
  !insertmacro BPQ_EDIT_PATH "$$k.SetValue('Path',(($$q+$$d) -join ';'),'ExpandString')"

  AddToPath_done:
  Pop $1
  Pop $0
FunctionEnd

; un.RemoveFromPath - removes the directory on the stack from PATH.
Function un.RemoveFromPath
  Exch $0
  Push $1

  !insertmacro BPQ_EDIT_PATH "if($$q.Count -lt $$p.Count){$$k.SetValue('Path',($$q -join ';'),'ExpandString')}"

  Pop $1
  Pop $0
FunctionEnd
