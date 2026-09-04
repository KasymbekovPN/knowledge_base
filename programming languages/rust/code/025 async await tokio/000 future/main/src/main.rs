use std::future::Future;
use std::pin::Pin;
use std::task::{Context, Poll, Waker};
use std::sync::{Arc, Mutex};
use std::thread;
use std::time::Duration;

struct Delay {
    shared: Arc<Mutex<SharedState>>,
}

struct SharedState {
    completed: bool,
    waker: Option<Waker>,
}

impl Future for Delay {
    type Output = String;

    fn poll(self: Pin<&mut Self>, cx: &mut Context<'_>) -> Poll<Self::Output> {
        let mut shared = self.shared.lock().unwrap();
        if shared.completed {
            Poll::Ready("Done!".to_string())
        } else {
            // сохраняем continuation
            shared.waker = Some(cx.waker().clone());
            Poll::Pending
        }
    }
}

impl Delay {
    fn new(duration: Duration) -> Self {
        let shared = Arc::new(Mutex::new(SharedState { completed: false, waker: None }));
        let thread_shared = Arc::clone(&shared);
        thread::spawn(move || {
            thread::sleep(duration);
            let mut s = thread_shared.lock().unwrap();
            s.completed = true;
            if let Some(waker) = s.waker.take() {
                // "разбуди меня" -- сигнал исполнителю перепроверить
                waker.wake();
            }
        });
        Delay { shared }
    }
}


#[tokio::main]
async fn main() {
    println!("before await");
    let result = Delay::new(Duration::from_millis(200)).await;
    println!("result: {:?}", result);
}
