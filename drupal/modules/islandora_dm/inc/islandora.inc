<?php 

/*
 * Functions for accessing the Fedora repository (via the Fedora_Repository module) are 
 * contained in this file. The main goal is to simplify the Fedora_Repository API, and 
 * isolate the Document Management module from changes in that API.
 */

module_load_include('inc', 'Fedora_Repository', 'api/fedora_item');
module_load_include('inc', 'Fedora_Repository', 'api/fedora_collection');
module_load_include('inc', 'Fedora_Repository', 'api/dublin_core');

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
  try {
    $item = new Fedora_Item($pid);
    if ($item->exists()) {
      return $item;
    }
  }
  
  catch (exception $e) {
    // bury
  }
  
  watchdog(Constants::MODULE_NAME, t('Unable to retrieve ' . $pid . '.'), null, WATCHDOG_ERROR);
  return NULL;
}

/* TODO move to DMPurchaseOrder or DMNote? */
function fetch_notes($po_pid) {
  return fetch_member_pids($po_pid, Constants::PID_PO_NOTE_CMODEL);
}

/* TODO move to DMPurchaseOrder or DMDocument? */
function fetch_documents($po_pid) {
  $documents = array();
  $doc_pids = fetch_document_pids($po_pid);
  foreach($doc_pids as $pid) {
    $documents[$pid] = fetch_page_pids($pid);
  }
  
  return $documents;
}

/* TODO move to DMPurchaseOrder or DMDocument? */
function fetch_document_pids($po_pid) {
  return fetch_member_pids($po_pid, Constants::PID_PO_DOCUMENT_CMODEL);
}

/* TODO move to DMPurchaseOrder or DMPage? */
function fetch_page_pids($document_pid) {
  return fetch_member_pids($document_pid, Constants::PID_PO_PAGE_CMODEL);
}

/* TODO move to DMObject? */
function fetch_member_pids($collection_pid, $cmodel = NULL) {
  $member_pids = get_related_items_as_array($collection_pid, 'isMemberOf', NULL, NULL, NULL, $cmodel);
  natsort($member_pids);
  return $member_pids;
}

/* TODO move to DMPurchaseOrder */
function add_collection_datastreams($fedora_item) {
  $collection = fetch_po_collection();
  
  if ($collection) {
    add_classification($collection, $fedora_item);
    add_disposition($collection, $fedora_item);
  }
  
  else {
    throw new Exception('error retrieving collection');
  }
}

/* TODO move to DMPurchaseOrder */
function add_classification($collection, $fedora_item) {
  $xml = datastream_to_xml(Constants::DSID_DEFAULT_CLASSIFICATION, $collection);
  
  global $user;
  $xml->depositorOfRecord = $user->name;
  $xml->dateOfDeposit = date('Y-m-d');
  $ds = $xml->asXML();
  
  $fedora_item->add_datastream_from_string($ds, 
    Constants::DSID_CLASSIFICATION, 
    Constants::DSLABEL_CLASSIFICATION, 
    Constants::MIME_TYPE_XML, 'X');
}

/* TODO move to DMPurchaseOrder */
function add_disposition($collection, $fedora_item) {
  $xml = datastream_to_xml(Constants::DSID_DEFAULT_DISPOSITION, $collection);
  
  global $user;
  $xml->dispositionDecisionUser = $user->name;
  
  $dispositionDate = date('Y-m-d', mktime(0, 0, 0, date('m') + $xml->retentionPeriod, date('d'),   date('Y')));
  $xml->dispositionDate = $dispositionDate;
  $ds = $xml->asXML();
  
  $fedora_item->add_datastream_from_string($ds, 
    Constants::DSID_DISPOSITION, 
    Constants::DSLABEL_DISPOSITION, 
    Constants::MIME_TYPE_XML, 'X');
}

/* TODO move to DMPurchaseOrder */
function copy_collection_datastream($collection, $fedora_item, $collection_dsid, $item_dsid, $item_dslabel, $mimetype = Constants::MIME_TYPE_XML, $control_group = 'X') {  
  $ds = $collection->get_datastream_dissemination($collection_dsid);
    
  if (!empty($ds)) {
    $fedora_item->add_datastream_from_string($ds, $item_dsid, $item_dslabel, $mimetype, $control_group);
  }
  
  else {
    throw new Exception('failed to copy ' . $collection_dsid . ' to ' . $fedora_item->pid);
  }
}

/* TODO move to DMPurchaseOrder? */
function fetch_po_collection() {
  $collection = fetch_fedora_item(Constants::PID_PO_COLLECTION);
  return $collection->exists() ? $collection : null;
}

/* TODO move to DMObject? */
function create_fedora_item_with_dc($pid, $title) {
  // determine owner
  global $user;
  $owner = $user->name;
  
  // create and store fedora item
  $foxml = Fedora_Item::create_object_FOXML($pid, 'A', $title, $owner);  
  $fedora = Fedora_Item::ingest_from_FOXML($foxml);
  
  if (!$fedora->exists()) {
    throw new Exception('failed to create fedora object. permissions problem?');
  }
  
  // create and attach dublin core stream
  $dc = new Dublin_Core($fedora);
  $dc->set_element(Constants::DC_TITLE, array($title));
  $dc->set_element(Constants::DC_IDENTIFIER, array($pid));
  $dc->save();
  
  return $fedora;
}

/* TODO move to DMObject? */
function datastream_to_xml($datastream_id, $fedora_item) {
  $ds = $fedora_item->get_datastream_dissemination($datastream_id);
  return new SimpleXmlElement($ds);
}

/* TODO move to DMObject? */
function generate_pid() {
  return Fedora_Item::get_next_PID_in_namespace(Constants::PID_NAMESPACE);
}

