+> "math" => math;

$collatz(n: i32) -> i32 {
    $steps! = 0;
    $current! = n;

    @ current > 1 {
        $remainder! = math.mod(current, 2);
        
        current = #remainder > 0 {
            (current * 3) + 1
        } ! {
            math.div(current, 2)
        };
        
        steps = steps + 1;
    }
    
    steps
}


$test_gcd! = math.gcd(48, 18);
$test_collatz! = collatz(27); 

