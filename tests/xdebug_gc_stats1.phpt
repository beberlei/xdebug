--TEST--
GC Stats: No memleak, return empty runs
--INI--
zend.enable_gc=1
--FILE--
<?php

ini_set("xdebug.gc_stats_enable", 1);

$data = xdebug_get_gc_stats();

var_dump($data);
--EXPECTF--
array(0) {
}

