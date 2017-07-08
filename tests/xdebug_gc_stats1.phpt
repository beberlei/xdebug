--TEST--
xdebug: No memleak, return empty runs
--FILE--
<?php

ini_set("xdebug.gc_stats_enable", 1);

$data = xdebug_gc_stats();

var_dump($data);
--EXPECTF--
array(0) {
}

