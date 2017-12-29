--TEST--
GC Stats: stats about trigger garbage collection automatically
--INI--
zend.enable_gc=1
--FILE--
<?php

ini_set("xdebug.gc_stats_enable", 1);
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

$data = xdebug_get_gc_stats();

var_dump(count($data) >= 3);
--EXPECTF--
bool(true)
