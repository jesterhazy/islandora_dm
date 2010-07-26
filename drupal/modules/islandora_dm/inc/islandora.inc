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
  $item = new Fedora_Item($pid);
  
  if (!$item->exists()) {
    watchdog(t(Constants::MODULE_NAME), t('Unable to retrieve ' . $pid . '.'), null, WATCHDOG_ERROR);
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
    $fedora->add_relationship('isMemberOf', Constants::PID_PO_COLLECTION);
    $fedora->add_relationship('hasModel', Constants::PID_PO_CMODEL, FEDORA_MODEL_URI);

    // create dm stream
    $fedora->add_datastream_from_string('<dm><owner>unassigned</owner></dm>', Constants::DSID_DM, Constants::DSLABEL_DM, Constants::MIME_TYPE_XML, 'X');

    // create po stream
    $fedora->add_datastream_from_string($po->getUIS()->asXML(), Constants::DSID_UIS, Constants::DSLABEL_UIS, Constants::MIME_TYPE_XML, 'X');
  
    add_collection_datastreams($fedora);
  }
  
  catch (Exception $e) {
    $fedora = null;
    watchdog(Constants::MODULE_NAME, 'Error while creating fedora object %pid: @e', array('%pid' => $po->getPID(), '@e' => $e), WATCHDOG_WARN);
  }

  return $fedora;
}

function add_collection_datastreams($fedora_item) {
  $collection = fetch_po_collection();
  
  if ($collection) {
    add_policy($collection, $fedora_item);
    add_classification($collection, $fedora_item);
    add_disposition($collection, $fedora_item);
  }
  
  else {
    throw new Exception('error retrieving collection');
  }
}

function add_policy($collection, $fedora_item) {
  copy_collection_datastream($collection, 
    $fedora_item, 
    Constants::DSID_DEFAULT_POLICY, 
    Constants::DSID_POLICY, 
    Constants::DSLABEL_POLICY);
}

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

function copy_collection_datastream($collection, $fedora_item, $collection_dsid, $item_dsid, $item_dslabel, $mimetype = Constants::MIME_TYPE_XML, $control_group = 'X') {  
  $ds = $collection->get_datastream_dissemination($collection_dsid);
    
  if (!empty($ds)) {
    $fedora_item->add_datastream_from_string($ds, $item_dsid, $item_dslabel, $mimetype, $control_group);
  }
  
  else {
    throw new Exception('failed to copy ' . $collection_dsid . ' to ' . $fedora_item->pid);
  }
}

function fetch_po_collection() {
  $collection = fetch_fedora_item(Constants::PID_PO_COLLECTION);
  return $collection->exists() ? $collection : null;
}

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

function create_document_fedora_item($po, $po_fedora_item, $file) {
  try {
    // this pid strategy is a potential concurrency problem. 
    // it could be replaced with call to fedora's getNextPID service
    $documentNumber = $po->documentCount() + 1;  
    $pid = $po->getPID() . '-doc-' . $documentNumber;
    $title = $po->getTitle() . ' Document ' . $documentNumber;
    
    $fedora = create_fedora_item_with_dc($pid, $title);

    // add relationhips
    $fedora->add_relationship('isMemberOf', $po->getPID());
    $fedora->add_relationship('hasModel', Constants::PID_PO_DOCUMENT_CMODEL, FEDORA_MODEL_URI);
    
    // create image datastream
    $fedora->add_datastream_from_file($file, Constants::DSID_TIFF, Constants::DSLABEL_TIFF, Constants::MIME_TYPE_TIFF);

    // copy access policy from po object
    $policy = $po_fedora_item->get_datastream_dissemination(Constants::DSID_POLICY);
    
    if (!empty($policy)) {
      $fedora->add_datastream_from_string($policy, Constants::DSID_POLICY, Constants::DSLABEL_POLICY, Constants::MIME_TYPE_XML, 'X');
    }
  }
  
  catch (Exception $e) {
    $fedora = null;
    watchdog(Constants::MODULE_NAME, 'Error while creating fedora object %pid: @e', array('%pid' => $po->getPID(), '@e' => $e), WATCHDOG_WARN);    
  }
  
  return $fedora;
}

function datastream_to_xml($datastream_id, $fedora_item) {
  $ds = $fedora_item->get_datastream_dissemination($datastream_id);
  return new SimpleXmlElement($ds);
}
