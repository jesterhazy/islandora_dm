1. solr configuration

- make solr schema.xml has dynamic indexing enabled for islandora-dm* (or just *):

  <dynamicField name="*" type="text_fgs" indexed="true"  stored="true" multiValued="true"/>
  
2. gsearch configuration

- see foxmlToSolr.xslt.

3. fedora_repository

- need to add islandora-dm: pid space to collections list in admin settings.

4. fedora.fcfg

- enable messaging