

### .vscode/c_cpp_properties.json
```json
{
    "configurations": [
        {
            "name": "Clang",
            "compilerPath": "clang++",
            "cppStandard": "c++23",
            "intelliSenseMode": "windows-clang-x64"
        }
    ],
    "version": 4
}
```

### .vscode/tasks.json
```cpp
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "Build C++23",
            "type": "shell",
            "command": "C:/Program Files/LLVM/bin/clang++.exe",
            "args": [
                "-std=c++23",
                "-g",
                "${file}",
                "-o",
                "${fileDirname}/${fileBasenameNoExtension}.exe"
            ],
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "problemMatcher": [
                "$gcc"
            ]
        }
    ]
}
```

Устанавливает расширение [[https://marketplace.visualstudio.com/items?itemName=vadimcn.vscode-lldb|CodeLLDB]]
### .vscode/launch.json
```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug with LLDB",
            "type": "lldb",
            "request": "launch",
            "program": "${fileDirname}/${fileBasenameNoExtension}.exe",
            "cwd": "${fileDirname}",
            "preLaunchTask": "Build C++23"
        }
    ]
}
```

### .vscode/settings.json
```json
{
    "cmake.generator": "Ninja",
    "cmake.configureSettings": {
        "CMAKE_EXPORT_COMPILE_COMMANDS": "ON"
    },
}
```