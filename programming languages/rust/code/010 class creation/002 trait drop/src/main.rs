
struct FileHandle {
    name: String,
}

impl Drop for FileHandle {
    fn drop(&mut self) { println!("Dropping FileHandle {}", self.name); }
}

fn main() {
    {
        let _f = FileHandle { name: "file.txt".to_string() };
        println!("work with file");
    }
    println!("after drop execution");
}