# ==== Purpose ====
#
# Test the --[start|stop]-[position|datetime] options to mysqlbinlog,
# with/without --read-from-remote-server, with one or two binary logs
# as input.
#
# ==== Implementation ====

# TODO: Need to look at making row based version once new binlog client is complete.

CREATE TABLE t1 (a INT);
RESET MASTER;

# We need to set fixed timestamps in this test.
# Use a date in the future to keep a growing timestamp along the
# binlog (including the Start_log_event). This test will work 
# unchanged everywhere, because mysql-test-run has fixed TZ, which it
# exports (so mysqlbinlog has same fixed TZ).
--let $datetime_1= 2031-01-01 12:00:00
eval SET TIMESTAMP= UNIX_TIMESTAMP("$datetime_1");
INSERT INTO t1 VALUES(1);
--let $pos_1= query_get_value(SHOW MASTER STATUS, Position, 1)

--let $datetime_2= 2032-01-01 12:00:00
eval SET TIMESTAMP= UNIX_TIMESTAMP("$datetime_2");
INSERT INTO t1 VALUES(2);
--let $pos_2= query_get_value(SHOW MASTER STATUS, Position, 1)

eval SET TIMESTAMP= UNIX_TIMESTAMP("2033-01-01 12:00:00");
INSERT INTO t1 VALUES(3);

--let $file_1= query_get_value(SHOW MASTER STATUS, File, 1)
FLUSH LOGS;

SET TIMESTAMP= UNIX_TIMESTAMP("2034-01-01 12:00:00");
INSERT INTO t1 VALUES(4);
--let $pos_3= query_get_value(SHOW MASTER STATUS, Position, 1)
--let $datetime_3= 2035-01-01 12:00:00
eval SET TIMESTAMP= UNIX_TIMESTAMP("$datetime_3");
INSERT INTO t1 VALUES(5);

--let $file_2= query_get_value(SHOW MASTER STATUS, File, 1)
FLUSH LOGS;

--let $MYSQLD_DATADIR= `select @@datadir`

--echo
--echo ==== Local ====

--let $start_position= $pos_1
--let $start_datetime= $datetime_1

--let $stop_position= $pos_2
--let $stop_datetime= $datetime_2
--let $extra_options= $MYSQLD_DATADIR/$file_1
--source extra/binlog_tests/mysqlbinlog_start_stop_1.inc

--echo
--echo ==== Local with 2 binlogs on command line ====
# This is to verify that some options apply only to first, or last binlog

--let $stop_position= $pos_3
--let $stop_datetime= $datetime_3
--let $extra_options= $MYSQLD_DATADIR/$file_1 $MYSQLD_DATADIR/$file_2
--source extra/binlog_tests/mysqlbinlog_start_stop_1.inc

--echo
--echo ==== Remote ====

--let $stop_position= $pos_2
--let $stop_datetime= $datetime_2
--let $extra_options= --read-from-remote-server --user=root --host=127.0.0.1 --port=$MASTER_MYPORT $file_1
--source extra/binlog_tests/mysqlbinlog_start_stop_1.inc

--echo
--echo ==== Remote with 2 binlogs on command line ====

--let $stop_position= $pos_3
--let $stop_datetime= $datetime_3
--let $extra_options= --read-from-remote-server --user=root --host=127.0.0.1 --port=$MASTER_MYPORT $file_1 $file_2
--source extra/binlog_tests/mysqlbinlog_start_stop_1.inc

# ==== to-last-log ====

--let $extra_options= --read-from-remote-server --user=root --host=127.0.0.1 --port=$MASTER_MYPORT $file_1
--let $option= --to-last-log
--source extra/binlog_tests/mysqlbinlog_start_stop_2.inc

--echo
--echo ==== clean up ====
DROP TABLE t1;
