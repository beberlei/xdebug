/*
   +----------------------------------------------------------------------+
   | Xdebug																  |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2017 Derick Rethans								  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 1.0 of the Xdebug license,	  |
   | that is bundled with this package in the file LICENSE, and is		  |
   | available at through the world-wide-web at							  |
   | http://xdebug.derickrethans.nl/license.php							  |
   | If you did not receive a copy of the Xdebug license and are unable   |
   | to obtain it through the world-wide-web, please send a note to		  |
   | xdebug@derickrethans.nl so we can mail you a copy immediately.		  |
   +----------------------------------------------------------------------+
   | Authors: Benjamin Eberlei <kontakt@beberlei.de>					  |
   +----------------------------------------------------------------------+
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_xdebug.h"
#include "xdebug_gc_stats.h"
#include "zend_builtin_functions.h"
#include "SAPI.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

int xdebug_gc_collect_cycles(void)
{
	int ret;
	uint32_t collected;
	zval run, stack;
	zend_execute_data *execute_data;
	zend_function *current_function;
	long int memory;
	double start;

	if (!XG(gc_stats_enabled)) {
		return xdebug_old_gc_collect_cycles();
	}

	collected = GC_G(collected);
	start = xdebug_get_utime();
	memory = zend_memory_usage(0);
	execute_data = EG(current_execute_data);

	ret = xdebug_old_gc_collect_cycles();

	array_init(&run);

	add_assoc_long(&run, "collected", GC_G(collected) - collected);
	add_assoc_long(&run, "duration", xdebug_get_utime() - start);
	add_assoc_long(&run, "memory_before", memory);
	add_assoc_long(&run, "memory_after", zend_memory_usage(0));

	if (execute_data && execute_data->func) {
		current_function = execute_data->func;

		if (current_function->common.function_name) {
			add_assoc_str_ex(&run, "function", sizeof("function")-1, current_function->common.function_name);
		}

		if (current_function->common.scope && current_function->common.scope->name) {
			add_assoc_str_ex(&run, "class", sizeof("class")-1, current_function->common.scope->name);
		}
	}

	zend_fetch_debug_backtrace(&stack, 0, DEBUG_BACKTRACE_IGNORE_ARGS, 0 TSRMLS_CC);
	add_assoc_zval(&run, "stack", &stack);

	zend_hash_next_index_insert(Z_ARRVAL(XG(gc_runs)), &run);

	return ret;
}

int xdebug_gc_stats_report_enabled()
{
	if (sapi_module.name && strcmp(sapi_module.name, "cli") == 0 && XG(gc_show_report) == 1) {
		return 1;
	}

	return 0;
}

void xdebug_gc_stats_show_report()
{
	zval *run, *collected, *duration, *memory_before, *memory_after, *function, *class;

	php_printf("## Garbage Collection Report ##\n");
	php_printf("Found %d garbage collection runs in current script.\n\n", zend_hash_num_elements(Z_ARRVAL(XG(gc_runs))));

	php_printf("Collected | Efficiency%% | Duration | Memory Before | Memory After | Reduction%% | Function\n");
	php_printf("----------|-------------|----------|---------------|--------------|------------|---------\n");

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(XG(gc_runs)), run) {
        if (!(collected = zend_hash_str_find(Z_ARRVAL_P(run), "collected", sizeof("collected") - 1))) {
            continue;
        }

        if (!(duration = zend_hash_str_find(Z_ARRVAL_P(run), "duration", sizeof("duration") - 1))) {
            continue;
        }

        if (!(memory_before = zend_hash_str_find(Z_ARRVAL_P(run), "memory_before", sizeof("memory_before") - 1))) {
            continue;
        }

        if (!(memory_after = zend_hash_str_find(Z_ARRVAL_P(run), "memory_after", sizeof("memory_after") - 1))) {
            continue;
        }

        function = zend_hash_str_find(Z_ARRVAL_P(run), "function", sizeof("function") - 1);
        class = zend_hash_str_find(Z_ARRVAL_P(run), "class", sizeof("class") - 1);

        if (!function) {
            php_printf(
                "%9d | %9.2f %% | %5.2f ms | %13d | %12d | %8.2f %% | -\n",
                Z_LVAL_P(collected),
                (Z_LVAL_P(collected) / 10000.0) * 100.0,
                Z_LVAL_P(duration) / 1000.0,
                Z_LVAL_P(memory_before),
                Z_LVAL_P(memory_after),
                (1 - (float)Z_LVAL_P(memory_after) / (float)Z_LVAL_P(memory_before)) * 100.0
            );
        } else if (!class && function) {
            php_printf(
                "%9d | %9.2f %% | %5.2f ms | %13d | %12d | %8.2f %% | %s\n",
                Z_LVAL_P(collected),
                (Z_LVAL_P(collected) / 10000.0) * 100.0,
                Z_LVAL_P(duration) / 1000.0,
                Z_LVAL_P(memory_before),
                Z_LVAL_P(memory_after),
                (1 - (float)Z_LVAL_P(memory_after) / (float)Z_LVAL_P(memory_before)) * 100.0,
                Z_STRVAL_P(function)
            );
        } else if (class && function) {
            php_printf(
                "%9d | %9.2f %% | %5.2f ms | %13d | %12d | %8.2f %% | %s::%s\n",
                Z_LVAL_P(collected),
                (Z_LVAL_P(collected) / 10000.0) * 100.0,
                Z_LVAL_P(duration) / 1000.0,
                Z_LVAL_P(memory_before),
                Z_LVAL_P(memory_after),
                (1 - (float)Z_LVAL_P(memory_after) / (float)Z_LVAL_P(memory_before)) * 100.0,
                Z_STRVAL_P(class),
                Z_STRVAL_P(function)
            );
        }
	} ZEND_HASH_FOREACH_END();
}
