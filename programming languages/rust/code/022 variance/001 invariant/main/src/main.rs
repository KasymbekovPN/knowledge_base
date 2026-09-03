
fn evil_swap<'a>(important_place: &mut &'a str, cheap_trick: &'a str) {
    *important_place = cheap_trick;
}

fn main() {
    let mut main_ptr: &'static str = "Important static string";

    {
        let short_string = String::from("Short string");

        // ОШИБКА КОМПИЛЯЦИИ: main_ptr имеет тип &'static str.
        // evil_swap пытается временно "сжать" 'static до времени жизни этого блока.
        // Из-за инвариантности по T, Rust говорит: "Нет, &mut &'static str
        // нельзя превратить в &mut &'block str".
        // evil_swap(&mut main_ptr, &short_string);
    }

    // Если бы код скомпилировался, здесь main_ptr указывал бы на
    // уже удаленную short_string! Был бы краш.
    println!("{}", main_ptr);
}
