$factorial(num: u64) -> u64 {
    $res! = 1;
    $n! = num;
    @n > 1 {
        res = res * num;
        n = n - 1;
    }
    res
}

$f5 = factorial(5);

