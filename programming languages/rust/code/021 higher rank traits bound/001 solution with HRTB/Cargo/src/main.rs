
// Читается так: "F реализует Fn(&str) ДЛЯ ВСЕХ ('for') времен жизни 'a"
fn process_data<F>(f: F)
where
    F: for<'a> Fn(&'a str)
{
    let local_string = String::from("Hello world");
    f(&local_string);
}

fn main() {
    // Замыкание просто печатает то, что ему дали
    process_data(|text| println!("Gotten: {}", text));
}
