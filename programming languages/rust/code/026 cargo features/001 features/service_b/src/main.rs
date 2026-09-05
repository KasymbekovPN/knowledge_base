use logkit;

fn main() {
    println!("service_b have required features timestamps only");
    println!("active features: {:?}", logkit::active_features());
    println!("{}", logkit::log("Message from service_b"));
}
