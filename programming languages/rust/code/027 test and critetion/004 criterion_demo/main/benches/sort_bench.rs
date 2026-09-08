use std::hint::black_box;
use criterion::{criterion_group, criterion_main, BenchmarkId, Criterion, Throughput};
use criterion_demo::{generate_data, insertion_sort, std_sort};

fn bench_single_size(c: &mut Criterion) {
    let data = generate_data(1000, 42);

    c.bench_function("insertion_sort 1000", |b| {
        b.iter(|| {
            let mut d = data.clone();
            insertion_sort(black_box(&mut d));
        });
    });

    c.bench_function("std_sort", |b| {
        b.iter(|| {
            let mut d = data.clone();
            std_sort(black_box(&mut d));
        });
    });
}

fn bench_across_sizes(c: &mut Criterion) {
    let mut group = c.benchmark_group("sorting");

    for size in [10, 100, 1_000, 5_000] {
        let data = generate_data(size, 42);
        group.throughput(Throughput::Elements(size as u64));

        group.bench_with_input(BenchmarkId::new("insertion_sort", size), &data, |b, data| {
            b.iter(|| {
                let mut d = data.clone();
                insertion_sort(black_box(&mut d));
            });
        });

        group.bench_with_input(BenchmarkId::new("std_sort", size), &data, |b, data| {
            b.iter(|| {
                let mut d = data.clone();
                std_sort(&mut d);
            });
        });
    }

    group.finish();
}

criterion_group!(benches, bench_single_size, bench_across_sizes);
criterion_main!(benches);
