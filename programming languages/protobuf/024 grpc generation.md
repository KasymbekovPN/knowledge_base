---
tags:
  - protobuf
---
[[programming languages/protobuf/_|<=]]


**Правило именования**

protoc сопоставляет флаг `--XXX_out` с плагином по имени `protoc-gen-XXX` — имя после `protoc-gen-` должно **буквально совпадать** с частью перед `_out`. Раз мы используем `--grpc_out`, плагин обязан называться `protoc-gen-grpc`, а не `protoc-gen-grpc-cpp`. Проверка это подтвердила: с `protoc-gen-grpc-cpp` получил ошибку `protoc-gen-grpc: program not found or is not executable` (protoc искал именно `protoc-gen-grpc`, а не то имя, что я дал).

**Рабочая команда**

```bash
protoc -I. --cpp_out=. --grpc_out=. \
  --plugin=protoc-gen-grpc=$(which grpc_cpp_plugin) \
  order_service.proto
```

Синтаксис `--plugin=protoc-gen-grpc=<путь>` — это переопределение: "когда ищешь `protoc-gen-grpc`, возьми вот этот конкретный исполняемый файл" (в нашем случае — `grpc_cpp_plugin`, который физически называется иначе, но выполняет роль этого плагина).

**Альтернатива без `--plugin` вообще**

Если положить сам бинарник `grpc_cpp_plugin` в `PATH` под именем `protoc-gen-grpc` (например, через symlink), то `--plugin=` можно не указывать — protoc сам найдёт нужный исполняемый файл по стандартному соглашению об именовании. Проверил это тоже — сработало без единого флага `--plugin`:

```bash
ln -s $(which grpc_cpp_plugin) /usr/local/bin/protoc-gen-grpc
protoc -I. --cpp_out=. --grpc_out=. order_service.proto
```

```powershell
protoc -I. --cpp_out=. --grpc_out=. --plugin=protoc-gen-grpc="C:\projects\vcpkg\installed\x64-windows\tools\grpc\grpc_cpp_plugin.exe" order_service.proto
```

Именно поэтому в CMake-функции `protobuf_generate` при работе с gRPC явный `--plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>` — самый надёжный способ, не зависящий от того, что лежит в `PATH` на конкретной машине разработчика или CI.
