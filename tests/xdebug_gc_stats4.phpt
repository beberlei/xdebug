--TEST--
xdebug: show garbage collection report
--INI--
xdebug.gc_stats_enable=1
xdebug.gc_show_report=1
--FILE--
<?php

function foo() {
    bar();
}

function bar() {
    for ($i = 0; $i < 20000; $i++) {
        $a = new stdClass();
        $b = new stdClass();
        $b->a = $a;
        $a->b = $b;
        unset($a, $b);
    }
}

class Garbage {
    public function produce() {
        $foo = new stdClass();

        for ($i = 0; $i < 20000; $i++) {
            $a = new stdClass();
            $b = new stdClass();
            $b->foo = $foo;
            $b->a = $a;
            $a->b = $b;
            unset($a, $b);
        }

        unset($foo);
        gc_collect_cycles();
    }
}

foo();
(new Garbage())->produce();
--EXPECTF--
## Garbage Collection Report ##
Found 9 garbage collection runs in current script.

Collected | Efficiency% | Duration | Memory Before | Memory After | Reduction% | Function
----------|-------------|----------|---------------|--------------|------------|---------
     9998 |     99.98 % |  %s ms |       %d |       %d |    %s % | bar
    10000 |    100.00 % |  %s ms |       %d |       %d |    %s % | bar
    10000 |    100.00 % |  %s ms |       %d |       %d |    %s % | bar
    10000 |    100.00 % |  %s ms |       %d |       %d |    %s % | bar
    10000 |    100.00 % |  %s ms |       %d |       %d |    %s % | Garbage::produce
    10000 |    100.00 % |  %s ms |       %d |       %d |    %s % | Garbage::produce
    10000 |    100.00 % |  %s ms |       %d |       %d |    %s % | Garbage::produce
    10000 |    100.00 % |  %s ms |       %d |       %d |    %s % | Garbage::produce
        3 |      0.03 % |  %s ms |        %d |       %d |     %s % | gc_collect_cycles
