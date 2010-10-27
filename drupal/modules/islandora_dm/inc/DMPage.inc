<?php

/*
 * DMPage - methods for retrieving, displaying, and manipulating pages.
 */
class DMPage extends DMObject {
  function typeLabel() {
    return 'Page';
  }
  
  function createFedoraItem() {
    parent::createFedoraItem();
    $this->fedora->add_relationship('hasModel', Constants::PID_PO_PAGE_CMODEL, FEDORA_MODEL_URI);
  }
  
  function addTiffFile($file) {
    // create image datastream
    $this->fedora->add_datastream_from_file($file->filename, Constants::DSID_TIFF, Constants::DSLABEL_TIFF, Constants::MIME_TYPE_TIFF);
  }
  
  static function import($po_pids, $document_pid, $file) {
    try {
      $obj = new DMPage();
      $obj->createFedoraItem();
      $obj->joinCollection($document_pid);
      $obj->joinCollection($po_pids);
      $obj->addTiffFile($file);
    }
    catch (Exception $e) {
        watchdog(Constants::MODULE_NAME, 'Error while creating fedora object %pid: @e', array('%pid' => $obj->pid(), '@e' => $e), WATCHDOG_WARN);
        $obj = null; 
    }
    
    return $obj;
  }
}