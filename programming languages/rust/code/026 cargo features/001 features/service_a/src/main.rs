use logkit;

fn main() {
    println!("service_a have required features json only");
    println!("active features: {:?}", logkit::active_features());
    println!("{}", logkit::log("Message from service_a"));
}
