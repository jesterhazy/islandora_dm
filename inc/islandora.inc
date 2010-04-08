<?php 

/*
 * Functions for accessing the Fedora repository (via the Fedora_Repository module) are 
 * contained in this file. The main goal is to simplify the Fedora_Repository API, and 
 * isolate the Document Management module from changes in that API.
 */

module_load_include('php', 'Fedora_Repository', 'ObjectHelper');
module_load_include('php', 'Fedora_Repository', 'api/fedora_item');
module_load_include('php', 'Fedora_Repository', 'api/fedora_collection');

function get_object_helper() {  
  $objectHelper = new ObjectHelper();
  return $objectHelper;
}

function ingest_object_from_foxml($file) {
  $pid = pid_from_file($file);
  
  // purge the item if it exists
  $item = new Fedora_Item($pid);
  if (fedora_item_exists($item)) {
    $item->purge();
  }
    
  $item = Fedora_Item::ingest_from_FOXML_file($file);
  
  if (!fedora_item_exists($item)) {
    throw new Exception('Ingest failed for ' . $pid . '.');
  }
  
  return $item;
}

/*
 * Logic copied from static method in Fedora_Item, but without object reload.
 * Core logic ought to be made available as instance method in Fedora_Item.
 */
function fedora_item_exists($item) {
  return (!empty($item->object_profile));
}

function pid_from_file($file) {
  
  /* 
   * makes the unchecked assumption that filename matches: /path/<pid>.xml,
   * and that the pid inside the file actually matches the filename.
   */
  return 'islandora-dm:' . basename($file, '.xml');
}

function get_fedora_item($pid) {
  $item = new Fedora_Item($pid);
  
  if (!fedora_item_exists($item)) {
    watchdog(t('islandora_dm'), t('Unable to retrieve ' . $pid . '.'), null, WATCHDOG_ERROR);
    return null;
  }
  
  return $item;
}

function get_children($pid) {
  // TODO may need to allow filtering by content model (depending on how attachments are handled)
  $child_pids = get_related_items_as_array($pid, 'isMemberOf');
  return $child_pids;
}

?>