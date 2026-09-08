fn main() {
    // Создание raw pointer -- БЕЗОПАСНО
    let x = 42;
    let r1: *const i32 = &x; // аналог const T* в C++
    let mut y = 10;
    let r2: *mut i32 = &mut y; // аналог T*

    // Разыменование -- уже unsafe
    unsafe {
        println!("r1: {}", *r1);
        *r2 += 5;
        println!("r2: {}", *r2);
    }

    // Несколько *const на один объект одновременно -- разрешено,
    // borrow checker на raw pointers не действует
    let a = &x as *const i32;
    let b = &x as *const i32;
    unsafe { println!("a: {}, b: {}", *a, *b); }

    // Указательная арифметика, как в C
    let arr = [10, 20, 30, 40, 50];
    let ptr = arr.as_ptr();
    unsafe {
        for i in 0..arr.len() {
            println!("arr[{i}]: {}", *ptr.add(i));
        }
    }

    // null допустим для raw pointer (недопустим для &T)
    let null_ptr: *const i32 = std::ptr::null();
    println!("is_null: {:?}", null_ptr.is_null());

    // unsafe fn -- функция с непроверяемым контрактом
    unsafe fn read_at(ptr: *const i32, index: isize) -> i32 {
        *ptr.offset(index)
    }
    unsafe { println!("{}", read_at(arr.as_ptr(), 2)); }
}
