@echo off
if exist hopsangui.exe (
  hopsangui.exe
) else if exist hopsangui_d.exe (
  hopsangui_d.exe
)


echo.
echo HopsanGUI has terminated
pause
