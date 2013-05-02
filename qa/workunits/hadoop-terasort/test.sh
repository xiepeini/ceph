#!/bin/sh -e

echo "starting hadoop-terasort test"

# bail if $TESTDIR is not set as this test will fail in that scenario
[ -z $TESTDIR] && { echo "\$TESTDIR needs to be set, but is not. Exiting."; exit 1; }

command0="export JAVA_HOME=/usr/lib/jvm/default-java"
command1="$TESTDIR/apache_hadoop/bin/hadoop dfs -mkdir /tests"
#command2="$TESTDIR/apache_hadoop/bin/hadoop dfs -mkdir /tests/terasort_data"
command3="$TESTDIR/apache_hadoop/bin/hadoop jar $TESTDIR/apache_hadoop/build/hadoop-*examples*jar teragen 20000000 /tests/terasort_data"
command4="$TESTDIR/apache_hadoop/bin/hadoop dfs -rmr /tests/terasort_data"
#command1="mkdir -p $TESTDIR/hadoop_input"
#command2="wget http://ceph.com/qa/hadoop_input_files.tar -O $TESTDIR/hadoop_input/files.tar"
#command3="cd $TESTDIR/hadoop_input"
#command4="tar -xf $TESTDIR/hadoop_input/files.tar"
#command5="$TESTDIR/apache_hadoop/bin/hadoop fs -mkdir wordcount_input"
#command6="$TESTDIR/apache_hadoop/bin/hadoop fs -put $TESTDIR/hadoop_input/*txt wordcount_input/"
#command7="$TESTDIR/apache_hadoop/bin/hadoop jar $TESTDIR/apache_hadoop/build/hadoop-example*jar wordcount wordcount_input wordcount_output"
#command8="rm -rf $TESTDIR/hadoop_input"


#print out the command
echo "----------------------"
echo $command0
echo $command1
echo $command2
echo $command3
echo $command4
echo "----------------------"

#now execute the command
$command0
$command1
$command2
$command3
#$command4

echo "completed hadoop-terasort test"
exit 0
