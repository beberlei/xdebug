--TEST--
GC Stats: No memleak, return empty runs
--INI--
zend.enable_gc=1
xdebug.gc_stats_enable=1
--FILE--
<?php

echo file_get_contents(xdebug_get_gcstats_filename());
--EXPECTF--
## Garbage Collection Report ##
Collected | Efficiency% | Duration | Memory Before | Memory After | Reduction% | Function
----------|-------------|----------|---------------|--------------|------------|---------
