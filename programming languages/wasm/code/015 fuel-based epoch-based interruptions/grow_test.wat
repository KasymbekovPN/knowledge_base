(module
  (memory (export "memory") 2)
  (func (export "grow_from_guest") (result i32)
    i32.const 1
    memory.grow)
)

;; wat2wasm grow_test.wat -o grow_test.wasm