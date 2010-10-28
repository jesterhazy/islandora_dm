<?php

/*
 * DMDocument - methods for retrieving, displaying, and manipulating documents.
 */
class DMDocument extends DMObject {
  function typeLabel() {
    return 'Document';
  }
  
  function createFedoraItem() {
    parent::createFedoraItem();
    $this->fedora->add_relationship('hasModel', Constants::PID_PO_DOCUMENT_CMODEL, FEDORA_MODEL_URI);
  }
  
  static function import($po_pids) {
    try {
      $obj = new DMDocument();
      $obj->createFedoraItem();
      $obj->joinCollection($po_pids);
    }
    catch (Exception $e) {
      watchdog(Constants::MODULE_NAME, 
        'Error while creating fedora object %pid: @e', 
        array('%pid' => $obj->pid(), '@e' => $e->getMessage()), 
        WATCHDOG_WARN);
      $obj = null;
    }
    
    return $obj;
  }
}