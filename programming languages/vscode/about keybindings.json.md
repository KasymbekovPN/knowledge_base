
можно использовать `workspaceFolderBasename`:

```json
{
    "key": "f5",
    "command": "python.execInTerminal",
    "when": "resourceExtname == '.py' && workspaceFolderBasename == 'knowledge_base'"
}
```

Доступные переменные для workspace в `when`:

|Переменная|Пример значения|
|---|---|
|`workspaceFolderBasename`|`knowledge_base`|
|`workspaceFolder`|`c:/projects/knowledge_base`|
|`workspaceFolderCount == 1`|число открытых папок|

`C:/Users/.../AppData/Roaming/Code/User/keybindings.json`