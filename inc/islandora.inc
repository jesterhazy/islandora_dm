<?php 

/*
 * Functions for accessing the Fedora repository (via the Fedora_Repository module) are 
 * contained in this file. The main goal is to simplify the Fedora_Repository API, and 
 * isolate the Document Management module from changes in that API.
 */

define('ISLANDORA_DM_PID_PO_COLLECTION', 'islandora-dm:po-collection');
define('ISLANDORA_DM_PID_PO_CMODEL', 'islandora-dm:po-cmodel');
define('ISLANDORA_DM_PID_PO_DOCUMENT_CMODEL', 'islandora-dm:po-document-cmodel');
define('ISLANDORA_DM_DATASTREAM_LABEL_DM', 'islandora-dm metadata');
define('ISLANDORA_DM_DATASTREAM_LABEL_UIS', 'islandora-dm purchase order data');
define('ISLANDORA_DM_DATASTREAM_ID_DM', 'dm');
define('ISLANDORA_DM_DATASTREAM_ID_UIS', 'uis');
define('ISLANDORA_DM_DSID_TIFF', 'tiff');
define('ISLANDORA_DM_DSLABEL_TIFF', 'tiff image');
define('ISLANDORA_DM_MIME_TYPE_TIFF', 'image/tiff');
define('ISLANDORA_DM_DC_TITLE', 'dc:title');
define('ISLANDORA_DM_DC_IDENTIFIER', 'dc:identifier');

module_load_include('inc', 'Fedora_Repository', 'ObjectHelper');
module_load_include('inc', 'Fedora_Repository', 'api/fedora_item');
module_load_include('inc', 'Fedora_Repository', 'api/fedora_collection');
module_load_include('inc', 'Fedora_Repository', 'api/dublin_core');

function get_object_helper() {  
  $objectHelper = new ObjectHelper();
  return $objectHelper;
}

function ingest_object_from_foxml($file) {
  $pid = pid_from_file($file);
  
  // purge the item if it exists
  $item = new Fedora_Item($pid);
  if ($item->exists()) {
    $item->purge();
  }
    
  $item = Fedora_Item::ingest_from_FOXML_file($file);
  
  if (!$item->exists()) {
    throw new Exception('Ingest failed for ' . $pid . '.');
  }
  
  return $item;
}

function pid_from_file($file) {
  
  /* 
   * makes the unchecked assumption that filename matches: /path/<pid>.xml,
   * and that the pid inside the file actually matches the filename.
   */
  return 'islandora-dm:' . basename($file, '.xml');
}

function fetch_fedora_item($pid) {
  $item = new Fedora_Item($pid);
  
  if (!$item->exists()) {
    watchdog(t('islandora_dm'), t('Unable to retrieve ' . $pid . '.'), null, WATCHDOG_ERROR);
    return null;
  }
  
  return $item;
}

function fetch_document_pids($pid) {
  // TODO may need to allow filtering by content model (depending on how attachments are handled)
  $child_pids = get_related_items_as_array($pid, 'isMemberOf');  
  return $child_pids;
}

function create_purchase_order_fedora_item($po) {
  try {
    $fedora = create_fedora_item_with_dc($po->getPID(), $po->getTitle());
   
    // add relationhips
    $fedora->add_relationship('isMemberOf', ISLANDORA_DM_PID_PO_COLLECTION);
    $fedora->add_relationship('hasModel', ISLANDORA_DM_PID_PO_CMODEL, FEDORA_MODEL_URI);

    // create dm stream
    $fedora->add_datastream_from_string('<dm><owner>unassigned</owner></dm>', ISLANDORA_DM_DATASTREAM_ID_DM, ISLANDORA_DM_DATASTREAM_LABEL_DM, 'text/xml', 'X');

    // create po stream
    $fedora->add_datastream_from_string($po->getUIS()->asXML(), ISLANDORA_DM_DATASTREAM_ID_UIS, ISLANDORA_DM_DATASTREAM_LABEL_UIS, 'text/xml', 'X');
  }
  
  catch (Exception $e) {
    $fedora = null;
    watchdog(t('islandora_dm'), t('Error while creating fedora object ')  . $po->getPID() . t(': ' . $e), null, WATCHDOG_WARN);
  }
  
  return $fedora;
}

function create_fedora_item_with_dc($pid, $title) {
  // create and store fedora item
  $foxml = Fedora_Item::create_object_FOXML($pid, 'A', $title);
  $fedora = Fedora_Item::ingest_from_FOXML($foxml);
  
  // create and attach dublin core stream
  $dc = new Dublin_Core($fedora);
  $dc->set_element(ISLANDORA_DM_DC_TITLE, array($title));
  $dc->set_element(ISLANDORA_DM_DC_IDENTIFIER, array($pid));
  $dc->save();
  
  return $fedora;
}

function create_document_fedora_item($po, $fedora, $file) {
  try {
    // this pid strategy is a potential concurrency problem. 
    // it could be replaced with call to fedora's getNextPID service
    $documentNumber = $po->documentCount() + 1;  
    $pid = $po->getPID() . '-doc-' . $documentNumber;
    $title = $po->getTitle() . ' Document ' . $documentNumber;
    
    $fedora = create_fedora_item_with_dc($pid, $title);

    // add relationhips
    $fedora->add_relationship('isMemberOf', $po->getPID());
    $fedora->add_relationship('hasModel', ISLANDORA_DM_PID_PO_DOCUMENT_CMODEL, FEDORA_MODEL_URI);
    
    // create image datastream
    $fedora->add_datastream_from_file($file, ISLANDORA_DM_DSID_TIFF, ISLANDORA_DM_DSLABEL_TIFF, ISLANDORA_DM_MIME_TYPE_TIFF);
  }
  
  catch (Exception $e) {
    $fedora = null;
    watchdog(t('islandora_dm'), t('Error while creating fedora object ')  . $pid . t(': ' . $e), null, WATCHDOG_WARN);
  }
  
  return $fedora;
}

?>