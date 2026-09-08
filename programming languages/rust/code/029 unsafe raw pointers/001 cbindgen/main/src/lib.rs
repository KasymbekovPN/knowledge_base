
#[unsafe(no_mangle)]
pub extern "C" fn rust_add(a: i32, b: i32) -> i32 { a + b }

#[repr(C)]
pub struct Vector3 {
    pub x: f64,
    pub y: f64,
    pub z: f64,
}

#[unsafe(no_mangle)]
pub extern "C" fn vector3_length(v: Vector3) -> f64 {
    (v.x * v.x + v.y * v.y + v.z * v.z).sqrt()
}

#[unsafe(no_mangle)]
pub extern "C" fn vector3_add(a: Vector3, b: Vector3) -> Vector3 {
    Vector3 { x: a.x + b.x, y: a.y + b.y, z: a.z + b.z }
}

pub struct Accumulator {
    total: f64,
}

#[unsafe(no_mangle)]
pub extern "C" fn accumulator_new() -> *mut Accumulator {
    Box::into_raw(Box::new(Accumulator { total: 0.0 }))
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn accumulator_add(ptr: *mut Accumulator, value: f64) {
    if ptr.is_null() {
        return;
    }
    (*ptr).total += value;
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn accumulator_total(ptr: *const Accumulator) -> f64 {
    if ptr.is_null() {
        return 0.0;
    }
    (*ptr).total
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn accumulator_free(ptr: *mut Accumulator) {
    if !ptr.is_null() {
        drop(Box::from_raw(ptr));
    }
}
