pushd %~dp0
pyinstaller --onefile ProtocolMaker.py
MOVE .\dist\ProtocolMaker.exe .\
@RD /S /Q .\build
@RD /S /Q .\dist
DEL /S /F /Q .\ProtocolMaker.spec
PAUSE