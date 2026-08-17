from wasmtime import Store, Module, Instance

store = Store()
module = Module.from_file(store.engine, "plugin_str.wasm")
instance = Instance(store, module, [])
exports = instance.exports(store)

malloc = exports["malloc"]
free = exports["free"]
to_upper = exports["to_upper"]
memory = exports["memory"]

text = b"hello from the host!"
length = len(text)

# 1) ХОСТ ПРОСИТ ГОСТЯ выделить буфер нужного размера -- НЕ выбирает смещение сам.
#    Гость сам знает, какие участки его linear memory свободны.
guest_ptr = malloc(store, length)
print(f"The guest allocated a buffer at its own offset: {guest_ptr}")

# 2) ХОСТ ЗАПИСЫВАЕТ байты в память гостя по этому смещению через API рантайма
#    (это ровно то же самое, что runtime делает под капотом при host_ptr + offset,
#     но с проверкой границ -- напрямую руками так не пишут).
memory.write(store, text, guest_ptr)

# 3) ХОСТ ВЫЗЫВАЕТ функцию плагина, передавая только (указатель, длина) -- как и
#    договорились в контракте. Плагин работает in-place внутри СВОЕЙ памяти.
to_upper(store, guest_ptr, length)

# 4) ХОСТ ЧИТАЕТ результат обратно из той же области памяти гостя.
result = memory.read(store, guest_ptr, guest_ptr + length)
print(f"Result after plugin calling: {result.decode()!r}")

# 5) ХОСТ ОБЯЗАН попросить гостя освободить буфер -- иначе при повторных вызовах
#    куча гостя будет расти бесконечно (в госте нет сборщика мусора).
free(store, guest_ptr)
print("Buffer free by guest (free-method called form host).")
