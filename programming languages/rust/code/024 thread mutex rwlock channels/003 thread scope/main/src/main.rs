use std::thread;

fn main() {
    let data = vec![1,2,3,4,5];

    thread::scope(|s| {
        s.spawn(|| { println!("thread 1 {:?}", &data[..2]); });
        s.spawn(|| { println!("thread 2 {:?}", &data[2..]); });
    }); // здесь гарантированно ВСЕ scoped-потоки уже завершены

    println!("main thread continues work: {data:?}");
}
