################################################################
# File:         /etc/init.d/crxdbg                             #
# Author:       krakjoe                                        #
# Purpose:      Daemonize crxdbg automatically on boot         #
# chkconfig:    2345    07 09                                  #
# description:  Starts, stops and restarts crxdbg daemon       #
################################################################
LOCKFILE=/var/lock/subsys/crxdbg
PIDFILE=/var/run/crxdbg.pid
STDIN=4000
STDOUT=8000
################################################################
# Either set path to crxdbg here or rely on crxdbg in ENV/PATH #
################################################################
if [ "x${CRXDBG}" == "x" ]; then
	CRXDBG=$(which crxdbg 2>/dev/null)
fi
################################################################
# Options to pass to crxdbg upon boot                          #
################################################################
OPTIONS=
LOGFILE=/var/log/crxdbg.log
################################################################
#     STOP EDITING STOP EDITING STOP EDITING STOP EDITING      #
################################################################
. /etc/rc.d/init.d/functions
RETVAL=1
################################################################
insanity()
{
	if [ "x${CRXDBG}" == "x" ]; then
		CRXDBG=$(which crxdbg 2>>/dev/null)
		if [ $? != 0 ]; then
			echo -n $"Fatal: cannot find crxdbg ${CRXDBG}"
			echo_failure
			echo
			return 1
		fi
	else
		if [ ! -x ${CRXDBG} ]; then
			echo -n $"Fatal: cannot execute crxdbg ${CRXDBG}"
			echo_failure
			echo
			return 1
		fi
	fi

	return 0
}

start()
{
	insanity

	if [ $? -eq 1 ]; then
		return $RETVAL
	fi

	echo -n $"Starting: crxdbg ${OPTIONS} on ${STDIN}/${STDOUT} "
	nohup ${CRXDBG} -l${STDIN}/${STDOUT} ${OPTIONS} 2>>${LOGFILE} 1>/dev/null </dev/null &
	PID=$!
	RETVAL=$?
	if [ $RETVAL -eq 0 ]; then
		echo $PID > $PIDFILE
		echo_success
	else
		echo_failure
	fi
	echo
	[ $RETVAL = 0 ] && touch ${LOCKFILE}
	return $RETVAL
}

stop()
{
	insanity

	if [ $? -eq 1 ]; then
		return $RETVAL
	fi

	if [ -f ${LOCKFILE} ] && [ -f ${PIDFILE} ]
	then
		echo -n $"Stopping: crxdbg ${OPTIONS} on ${STDIN}/${STDOUT} "
		kill -s TERM $(cat $PIDFILE)
		RETVAL=$?
		if [ $RETVAL -eq 0 ]; then
			echo_success
		else
			echo_failure
		fi
		echo
		[ $RETVAL = 0 ] && rm -f ${LOCKFILE} ${PIDFILE}
	else
		echo -n $"Error: crxdbg not running"
		echo_failure
		echo
		[ $RETVAL = 1 ]
	fi
	return $RETVAL
}
##################################################################
case "$1" in
	start)
	start
	;;
	stop)
	stop
	;;
	status)
	status $CRXDBG
	;;
	restart)
	$0 stop
	$0 start
	;;
	*)
	echo "usage: $0 start|stop|restart|status"
	;;
esac
###################################################################
exit $RETVAL
