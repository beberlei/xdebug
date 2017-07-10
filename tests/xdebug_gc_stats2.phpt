--TEST--
GC Stats: run gc_collect_cyles(); and collect stats
--INI--
zend.enable_gc=1
--FILE--
<?php

ini_set("xdebug.gc_stats_enable", 1);

for ($i = 0; $i < 100; $i++) {
    $a = new stdClass();
    $b = new stdClass();
    $b->a = $a;
    $a->b = $b;
    unset($a, $b);
}
gc_collect_cycles();

$data = xdebug_get_gc_stats();

var_dump($data);
--EXPECTF--
array(1) {
  [0]=>
  array(6) {
    ["collected"]=>
    int(200)
    ["duration"]=>
    int(%d)
    ["memory_before"]=>
    int(%d)
    ["memory_after"]=>
    int(%d)
    ["function"]=>
    string(17) "gc_collect_cycles"
    ["stack"]=>
    array(1) {
      [0]=>
      array(3) {
        ["file"]=>
        string(%d) "%s"
        ["line"]=>
        int(12)
        ["function"]=>
        string(17) "gc_collect_cycles"
      }
    }
  }
}

