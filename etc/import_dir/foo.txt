1. solr configuration

- make solr schema.xml has dynamic indexing enabled for islandora-dm* (or just *):

  <dynamicField name="*" type="text_fgs" indexed="true"  stored="true" multiValued="true"/>
  
2. gsearch configuration

- see foxmlToSolr.xslt.