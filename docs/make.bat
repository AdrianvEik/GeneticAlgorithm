@ECHO OFF
setlocal

pushd %~dp0

REM Command file for Sphinx documentation

set SOURCEDIR=source
set BUILDDIR=build
set REQUIREMENTS=%SOURCEDIR%\requirements.txt

if "%PYTHON%" == "" (
	set PYTHON=python
)

"%PYTHON%" -c "import sys" >NUL 2>NUL
if errorlevel 1 (
	if "%PYTHON%" == "python" (
		set PYTHON=py
		"%PYTHON%" -c "import sys" >NUL 2>NUL
	)
)

if errorlevel 1 (
	echo.
	echo.Python was not found. Install Python or set the PYTHON environment
	echo.variable to the Python executable that should build the docs.
	echo.
	goto end_error
)

if exist "%REQUIREMENTS%" (
	echo.Installing documentation requirements from %REQUIREMENTS%...
	"%PYTHON%" -m pip install -r "%REQUIREMENTS%"
	if errorlevel 1 (
		echo.
		echo.Failed to install documentation requirements.
		echo.
		goto end_error
	)
)

if "%SPHINXBUILD%" == "" (
	set SPHINXBUILD="%PYTHON%" -m sphinx
)

%SPHINXBUILD% --version >NUL 2>NUL
if errorlevel 1 (
	echo.
	echo.Sphinx could not be started after installing the documentation
	echo.requirements. Set SPHINXBUILD or PYTHON to the correct environment.
	echo.
	goto end_error
)

if "%1" == "" goto help

%SPHINXBUILD% -M %1 %SOURCEDIR% %BUILDDIR% %SPHINXOPTS% %O%
goto end

:help
%SPHINXBUILD% -M help %SOURCEDIR% %BUILDDIR% %SPHINXOPTS% %O%

:end
popd
endlocal
exit /b 0

:end_error
popd
endlocal
exit /b 1
