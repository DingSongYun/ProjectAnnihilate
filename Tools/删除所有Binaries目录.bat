@echo off
chcp 65001 >nul
set projectDir=%~dp0..
cd /d "%projectDir%"

echo 当前工作目录: %CD%
echo 项目目录: %projectDir%

:: 删除项目根目录下的 Binaries 和 Intermediate 目录
if exist "%projectDir%\Binaries" (
    echo 正在删除: "%projectDir%\Binaries"
    rmdir /s /q "%projectDir%\Binaries"
    if not exist "%projectDir%\Binaries" (
        echo 已成功删除根目录下的 Binaries 目录
    ) else (
        echo 删除失败，可能是权限问题
    )
) else (
    echo 根目录下不存在 Binaries 目录
)

if exist "%projectDir%\Intermediate" (
    echo 正在删除: "%projectDir%\Intermediate"
    rmdir /s /q "%projectDir%\Intermediate"
    if not exist "%projectDir%\Intermediate" (
        echo 已成功删除根目录下的 Intermediate 目录
    ) else (
        echo 删除失败，可能是权限问题
    )
) else (
    echo 根目录下不存在 Intermediate 目录
)

:: 删除 Plugins 目录及其子目录下的 Binaries 和 Intermediate 目录
echo 正在检查 Plugins 目录...
if exist "%projectDir%\Plugins" (
    for /d /r "%projectDir%\Plugins" %%i in (*) do (
        if exist "%%i\Binaries" (
            echo 正在删除: "%%i\Binaries"
            rmdir /s /q "%%i\Binaries"
            if not exist "%%i\Binaries" (
                echo 已成功删除 %%~nxi 中的 Binaries 目录
            ) else (
                echo 删除失败，可能是权限问题
            )
        )
        
        if exist "%%i\Intermediate" (
            echo 正在删除: "%%i\Intermediate"
            rmdir /s /q "%%i\Intermediate"
            if not exist "%%i\Intermediate" (
                echo 已成功删除 %%~nxi 中的 Intermediate 目录
            ) else (
                echo 删除失败，可能是权限问题
            )
        )
    )
) else (
    echo Plugins 目录不存在
)

echo 清理完成！
echo 如果没有删除成功，请尝试以管理员身份运行此批处理文件
pause