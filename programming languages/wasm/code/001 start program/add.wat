(module
	;; функция принимает два i32, складывает их и возвращает результат
	(func $add (export "add") (param $a i32) (param $b i32) (result i32)
	local.get $a
	local.get $b
	i32.add)
)

;; wasmtime run --invoke add add.wasm 2 3