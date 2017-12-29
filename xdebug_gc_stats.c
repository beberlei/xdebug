/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2017 Derick Rethans                               |
   +----------------------------------------------------------------------+
   | This source file is subject to version 1.0 of the Xdebug license,    |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://xdebug.derickrethans.nl/license.php                           |
   | If you did not receive a copy of the Xdebug license and are unable   |
   | to obtain it through the world-wide-web, please send a note to       |
   | xdebug@derickrethans.nl so we can mail you a copy immediately.       |
   +----------------------------------------------------------------------+
   | Authors: Benjamin Eberlei <kontakt@beberlei.de>                      |
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
    xdebug_gc_run *run;
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

    run = emalloc(sizeof(xdebug_gc_run));
    run->function_name = NULL;
    run->class_name = NULL;

    run->collected = GC_G(collected) - collected;
    run->duration = xdebug_get_utime() - start;
    run->memory_before = memory;
    run->memory_after = zend_memory_usage(0);

    if (execute_data && execute_data->func) {
        current_function = execute_data->func;

        if (current_function->common.function_name) {
            run->function_name = zend_string_copy(current_function->common.function_name);
        }

        if (current_function->common.scope && current_function->common.scope->name) {
            run->class_name = zend_string_copy(current_function->common.scope->name);
        }
    }

    zend_fetch_debug_backtrace(&(run->stack), 0, DEBUG_BACKTRACE_IGNORE_ARGS, 0 TSRMLS_CC);

    xdebug_llist_insert_next(XG(gc_runs), XDEBUG_LLIST_TAIL(XG(gc_runs)), run);

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
    zval *collected, *duration, *memory_before, *memory_after, *function, *class;
    xdebug_llist_element *le;
    xdebug_gc_run *run = NULL;

    php_printf("## Garbage Collection Report ##\n");
    php_printf("Found %d garbage collection runs in current script.\n\n", xdebug_llist_count(XG(gc_runs)));

    php_printf("Collected | Efficiency%% | Duration | Memory Before | Memory After | Reduction%% | Function\n");
    php_printf("----------|-------------|----------|---------------|--------------|------------|---------\n");

    for (le = XDEBUG_LLIST_HEAD(XG(gc_runs)); le != NULL; le = XDEBUG_LLIST_NEXT(le)) {
        run = XDEBUG_LLIST_VALP(le);

        if (!run->function_name) {
            php_printf(
                "%9d | %9.2f %% | %5.2f ms | %13d | %12d | %8.2f %% | -\n",
                run->collected,
                (run->collected / 10000.0) * 100.0,
                run->duration / 1000.0,
                run->memory_before,
                run->memory_after,
                (1 - (float)run->memory_after) / (float)run->memory_before * 100.0
            );
        } else if (!run->class_name && run->function_name) {
            php_printf(
                "%9d | %9.2f %% | %5.2f ms | %13d | %12d | %8.2f %% | %s\n",
                run->collected,
                (run->collected / 10000.0) * 100.0,
                run->duration / 1000.0,
                run->memory_before,
                run->memory_after,
                ZSTR_VAL(run->function_name)
            );
        } else if (run->class_name && run->function_name) {
            php_printf(
                "%9d | %9.2f %% | %5.2f ms | %13d | %12d | %8.2f %% | %s::%s\n",
                run->collected,
                (run->collected / 10000.0) * 100.0,
                run->duration / 1000.0,
                run->memory_before,
                run->memory_after,
                ZSTR_VAL(run->class_name),
                ZSTR_VAL(run->function_name)
            );
        }
    }
}
