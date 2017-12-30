--TEST--
GC Stats: stats about trigger garbage collection automatically
--INI--
zend.enable_gc=1
xdebug.gc_stats_enable=1
--FILE--
<?php

gc_enable();

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

foo();

echo file_get_contents(xdebug_get_gcstats_filename());

--EXPECTF--
## Garbage Collection Report ##
Collected | Efficiency% | Duration | Memory Before | Memory After | Reduction% | Function
----------|-------------|----------|---------------|--------------|------------|---------
    10000 |    100.00 % |  %s ms |       %d |       %d |   %s % | bar
    10000 |    100.00 % |  %s ms |       %d |       %d |   %s % | bar
    10000 |    100.00 % |  %s ms |       %d |       %d |   %s % | bar
