1. solr configuration

on test.islandlives.ca, important solr locations are:
index: /usr/local/fedora/gsearch_solr/solr/data
schema: /usr/local/fedora/tomcat/bin/solr/conf/schema.xml
conf: /usr/local/fedora/tomcat/bin/solr/conf/solrconfig.xml
xslt: /usr/local/fedora/tomcat/webapps/fedoragsearch/WEB-INF/classes/config/index/gsearch_solr/demoFoxmlToSolr.xslt
log: (catalina.out)

- add these to <fields> section of active solr schema.xml file (<solr_home>/conf/schema.xml):
<!-- islandora-dm fields-->
<dynamicField name="islandora-dm.date.*" type="date" indexed="true" stored="true"/>
<dynamicField name="islandora-dm.sort.*" type="string" indexed="true" stored="true"/>
<dynamicField name="*" type="text_fgs" indexed="true" stored="true" multiValued="true"/>

2. gsearch configuration

- add xslt fragment in gsearch-solr-config.txt to the active gsearch foxmlToSolr.xslt file.

3. fedora_repository

- need to add islandora-dm: pid space to collections list in admin settings.

4. fedora.fcfg

- enable messaging

5. po service

mock version:
http://island.local/mock_po_service/po-service.php

real version (last known url):

http://fairy.cs.upei.ca:18080/UIS_Web/po/xml/129183


