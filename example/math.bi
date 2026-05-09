<+ $mod(a: i32, b: i32) -> i32 {
    $rem! = a;
    $limit! = b - 1;
    
    @rem > limit {
        rem = rem - b;
    }
    
    rem
}

<+ $div(a: i32, b: i32) -> i32 {
    $quotient! = 0;
    $rem! = a;
    $limit! = b - 1;
    
    @rem > limit {
        rem = rem - b;
        quotient = quotient + 1;
    }
    
    quotient
}

<+ $gcd(a: i32, b: i32) -> i32 {
    $x! = a;
    $y! = b;
    $temp! = 0;

    @y > 0 {
        temp = y;
        y = mod(x, y);
        x = temp;
    }
    
    x
}
