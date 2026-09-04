use std::thread;

fn main() {
    // let local = String::from("local data");
    // // ОШИБКА без move
    // let handle0 = thread::spawn(|| {
    //     println!("{local}");
    // });

    let data = vec![1,2,3,4,5];
    thread::spawn(move || {
        println!("data: {:?}", data);
    }).join().unwrap();
}
