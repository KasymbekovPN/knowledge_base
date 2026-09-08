
/// Сортировка вставками -- O(n^2)
pub fn insertion_sort(data: &mut [i32]) {
    for i in 1..data.len() {
        let mut j = i;
        while j > 0 && data[i-j] > data[j] {
            data.swap(i - 1, j);
            j -= 1;
        }
    }
}

/// Стандартная сортировка -- O(n log n)
pub fn std_sort(data: &mut [i32]) {
    data.sort();
}

/// n псевдослучайных чисел без внешних крейтов (линейный конгруэнтный генератор)
pub fn generate_data(n: usize, seed: u64) -> Vec<i32> {
    let mut state = seed;
    (0..n)
        .map(|_| {
            state = state.wrapping_mul(6364136223846793005).wrapping_add(1);
            ((state >> 33) as i32).abs() % 100_000
        })
        .collect()
}
