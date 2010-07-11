#!/bin/sh

libdir='./../lib'
cp=..

for i in `ls $libdir/*.jar`
do
	cp=$cp:$i
done

echo 'using classpath: ' . $cp;


java -Xmx1500M -cp  $cp ca.upei.roblib.islandora.dm.jms.Main &
