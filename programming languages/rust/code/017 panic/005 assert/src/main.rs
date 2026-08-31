use std::thread;

fn main() {
    let t0 = thread::spawn(move || {
        let x = -5;
        assert!(x > 0, "[t0] must be positive");
    });
    let _r = t0.join();

    let t1 = thread::spawn(move || {
        let x = 6;
        assert_eq!(x, -1, "[t1] must eq -1");
    });
    let _r = t1.join();

    let t2 = thread::spawn(move || {
        let x = 100;
        debug_assert!(x > 100);
    });
    let _r = t2.join();
}