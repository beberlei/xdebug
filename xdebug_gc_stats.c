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

	execute_data = EG(current_execute_data);

	collected = GC_G(collected);
	start = xdebug_get_utime();
	memory = zend_memory_usage(0);

	ret = xdebug_old_gc_collect_cycles();

	run = xdmalloc(sizeof(xdebug_gc_run));
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

	xdebug_gc_stats_print_run(run);

	xdebug_gc_stats_run_free(run);

	return ret;
}

void xdebug_gc_stats_run_free(xdebug_gc_run *run)
{
	if (run) {
		if (run->function_name) {
			zend_string_release(run->function_name);
		}
		if (run->class_name) {
			zend_string_release(run->class_name);
		}
		zval_ptr_dtor(&(run->stack));
		xdfree(run);
	}
}

int xdebug_gc_stats_init(char* fname, char *script_name)
{
	char *filename = NULL;

	if (fname && strlen(fname)) {
		filename = xdstrdup(fname);
	} else {
		if (!strlen(XG(gc_stats_output_name)) ||
			xdebug_format_output_filename(&fname, XG(gc_stats_output_name), script_name) <= 0)
		{
			return FAILURE;
		}

		if (IS_SLASH(XG(gc_stats_output_dir)[strlen(XG(gc_stats_output_dir)) - 1])) {
			filename = xdebug_sprintf("%s%s", XG(gc_stats_output_dir), fname);
		} else {
			filename = xdebug_sprintf("%s%c%s", XG(gc_stats_output_dir), DEFAULT_SLASH, fname);
		}
		xdfree(fname);
	}

	XG(gc_stats_file) = xdebug_fopen(filename, "w", NULL, &XG(gc_stats_filename));
	xdfree(filename);

	if (!XG(gc_stats_file)) {
		return FAILURE;
	}

	fprintf(XG(gc_stats_file), "## Garbage Collection Report ##\n");

	fprintf(XG(gc_stats_file), "Collected | Efficiency%% | Duration | Memory Before | Memory After | Reduction%% | Function\n");
	fprintf(XG(gc_stats_file), "----------|-------------|----------|---------------|--------------|------------|---------\n");

	fflush(XG(gc_stats_file));

	return SUCCESS;
}

void xdebug_gc_stats_print_run(xdebug_gc_run *run)
{
	if (!XG(gc_stats_file)) {
		return;
	}

	if (!run->function_name) {
		fprintf(XG(gc_stats_file),
			"%9lu | %9.2f %% | %5.2f ms | %13lu | %12lu | %8.2f %% | -\n",
			run->collected,
			(run->collected / 10000.0) * 100.0,
			run->duration / 1000.0,
			run->memory_before,
			run->memory_after,
			(1 - (float)run->memory_after) / (float)run->memory_before * 100.0
		);
	} else if (!run->class_name && run->function_name) {
		fprintf(XG(gc_stats_file),
			"%9lu | %9.2f %% | %5.2f ms | %13lu | %12lu | %8.2f %% | %s\n",
			run->collected,
			(run->collected / 10000.0) * 100.0,
			run->duration / 1000.0,
			run->memory_before,
			run->memory_after,
			(1 - (float)run->memory_after) / (float)run->memory_before * 100.0,
			ZSTR_VAL(run->function_name)
		);
	} else if (run->class_name && run->function_name) {
		fprintf(XG(gc_stats_file),
			"%9lu | %9.2f %% | %5.2f ms | %13lu | %12lu | %8.2f %% | %s::%s\n",
			run->collected,
			(run->collected / 10000.0) * 100.0,
			run->duration / 1000.0,
			run->memory_before,
			run->memory_after,
			(1 - (float)run->memory_after) / (float)run->memory_before * 100.0,
			ZSTR_VAL(run->class_name),
			ZSTR_VAL(run->function_name)
		);
	}

	fflush(XG(gc_stats_file));
}
