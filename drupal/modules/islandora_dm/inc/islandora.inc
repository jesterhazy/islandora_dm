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
  
  watchdog(t(Constants::MODULE_NAME), t('Unable to retrieve ' . $pid . '.'), null, WATCHDOG_ERROR);
  return NULL;
}

function fetch_notes_pids($po_pid) {
  return fetch_member_pids($po_pid, Constants::PID_PO_NOTE_CMODEL);
}

function fetch_documents($po_pid) {
  $documents = array();
  $doc_pids = fetch_document_pids($po_pid);
  foreach($doc_pids as $pid) {
    $documents[$pid] = fetch_page_pids($pid);
  }
  
  return $documents;
}

function fetch_document_pids($po_pid) {
  return fetch_member_pids($po_pid, Constants::PID_PO_DOCUMENT_CMODEL);
}

function fetch_page_pids($document_pid) {
  return fetch_member_pids($document_pid, Constants::PID_PO_PAGE_CMODEL);
}

function fetch_member_pids($collection_pid, $cmodel = NULL) {
  $member_pids = get_related_items_as_array($collection_pid, 'isMemberOf', NULL, NULL, NULL, $cmodel);  
  return $member_pids;
}

function create_purchase_order_fedora_item($po) {
  try {
    $fedora = create_fedora_item_with_dc($po->getPid(), $po->getTitle());
    
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
    watchdog(Constants::MODULE_NAME, 'Error while creating fedora object %pid: @e', array('%pid' => $po->getPid(), '@e' => $e), WATCHDOG_WARN);
  }

  return $fedora;
}

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

function create_document_fedora_item($po) {
  try {
    // this pid strategy is a potential concurrency problem. 
    // it could be replaced with call to fedora's getNextPID service
    $document_number = $po->documentCount() + 1;  
    $pid = $po->documentPid($document_number);
    $title = $po->documentTitle($document_number);
    
    $fedora = create_fedora_item_with_dc($pid, $title);

    // add relationhips
    $fedora->add_relationship('isMemberOf', $po->getPid());
    $fedora->add_relationship('hasModel', Constants::PID_PO_DOCUMENT_CMODEL, FEDORA_MODEL_URI);
  }
  
  catch (Exception $e) {
    $fedora = null;
    watchdog(Constants::MODULE_NAME, 'Error while creating fedora object %pid: @e', array('%pid' => $po->getPid(), '@e' => $e), WATCHDOG_WARN);    
  }
  
  return $fedora;
}

function create_note_fedora_item($po, $text, $user) {
  try {
    // this pid strategy is a potential concurrency problem. 
    // it could be replaced with call to fedora's getNextPID service
    $note_number = $po->noteCount() + 1;  
    $pid = $po->notePid($note_number);
    $title = $po->noteTitle($note_number);
    
    $fedora = create_fedora_item_with_dc($pid, $title);

    // add relationhips
    $fedora->add_relationship('isMemberOf', $po->getPid());
    $fedora->add_relationship('hasModel', Constants::PID_PO_NOTE_CMODEL, FEDORA_MODEL_URI);
    
    // add note text
    $fedora->add_datastream_from_string($text, Constants::DSID_NOTE_TEXT, Constants::DSLABEL_NOTE_TEXT, Constants::MIME_TYPE_TEXT);
    
    // add creator id 
    // shouldn't have to do this, but fedora's creator info is mysteriously inaccessible
    $fedora->add_datastream_from_string($user, Constants::DSID_NOTE_CREATOR, Constants::DSLABEL_NOTE_CREATOR, Constants::MIME_TYPE_TEXT);
  }
  
  catch (Exception $e) {
    $fedora = null;
    watchdog(Constants::MODULE_NAME, 'Error while creating fedora object %pid: @e', array('%pid' => $po->getPid(), '@e' => $e), WATCHDOG_WARN);    
  }
  
  return $fedora;
}

function create_page_fedora_item($po, $document_pid, $file) {  
  try {
    // this pid strategy is a potential concurrency problem. 
    // it could be replaced with call to fedora's getNextPID service
    $page_number = $po->pageCount($document_pid) + 1;
    $pid = $po->pagePid($document_pid, $page_number);    
    $title = $po->pageTitle($document_pid, $page_number);
    
    $fedora = create_fedora_item_with_dc($pid, $title);

    // add relationhips
    $fedora->add_relationship('isMemberOf', $document_pid);
    $fedora->add_relationship('hasModel', Constants::PID_PO_PAGE_CMODEL, FEDORA_MODEL_URI);
    
    // create image datastream
    $fedora->add_datastream_from_file($file->filename, Constants::DSID_TIFF, Constants::DSLABEL_TIFF, Constants::MIME_TYPE_TIFF);
  }
  
  catch (Exception $e) {
    $fedora = null;
    watchdog(Constants::MODULE_NAME, 'Error while creating fedora object %pid: @e', array('%pid' => $po->getPid(), '@e' => $e), WATCHDOG_WARN);    
  }
  
  return $fedora;
} 

function datastream_to_xml($datastream_id, $fedora_item) {
  $ds = $fedora_item->get_datastream_dissemination($datastream_id);
  return new SimpleXmlElement($ds);
}
