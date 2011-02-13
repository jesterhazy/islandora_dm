1. solr configuration


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


