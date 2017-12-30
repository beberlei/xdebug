--TEST--
GC Stats: class with garbage
--INI--
zend.enable_gc=1
xdebug.gc_stats_enable=1
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

echo file_get_contents(xdebug_get_gcstats_filename());
--EXPECTF--
## Garbage Collection Report ##
Collected | Efficiency% | Duration | Memory Before | Memory After | Reduction% | Function
----------|-------------|----------|---------------|--------------|------------|---------
    10000 |    100.00 % |  %s ms |       %d |       %d |   %s % | bar
    10000 |    100.00 % |  %s ms |       %d |       %d |   %s % | bar
    10000 |    100.00 % |  %s ms |       %d |       %d |   %s % | bar
    10000 |    100.00 % |  %s ms |       %d |       %d |   %s % | Garbage::produce
    10000 |    100.00 % |  %s ms |       %d |       %d |   %s % | Garbage::produce
    10000 |    100.00 % |  %s ms |       %d |       %d |   %s % | Garbage::produce
    10000 |    100.00 % |  %s ms |       %d |       %d |   %s % | Garbage::produce
    10000 |    100.00 % |  %s ms |       %d |       %d |   %s % | Garbage::produce
        0 |      0.00 % |  %s ms |        %d |       %d |  %s % | gc_collect_cycles
