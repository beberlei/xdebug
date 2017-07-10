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
   | Authors: Benjamin Eberlei <kontakt@beberlei.de>					  |
   +----------------------------------------------------------------------+
 */

#ifndef __XDEBUG_GC_STATS_H__
#define __XDEBUG_GC_STATS_H__

int (*xdebug_old_gc_collect_cycles)(void);
int xdebug_gc_collect_cycles(void);

int xdebug_gc_stats_report_enabled();
void xdebug_gc_stats_show_report();

#endif
