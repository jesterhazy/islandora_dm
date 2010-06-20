<?php

/*
 * PurchaseOrder - methods for retrieving, displaying, and manipulating purchase orders.
 */
 
class PurchaseOrder {
  
  /**
   * The purchase order number.
   */
  protected $number;
  
  /**
   * Fedora_Item object containing the fedora data
   */
  protected $fedora; 
  
  /**
   * SimpleXMLElement object containing up to date UIS data
   */
  protected $uis;
  
  /**
   * SimpleXMLElement object containing dminfoDatastream from fedora
   */
  protected $dminfoDatastream;
  
  /**
   * SimpleXMLElement object containing uisinfoDatastream from fedora
   */
  protected $uisinfoDatastream;
  
  /**
   * array of strings containing pids of related (child) documents
   */
  protected $documentPids;
  
  /**
   * Constructor. Requires a po number.
   */
  function PurchaseOrder($po_number) {
    $this->number = $po_number;
  }
  
  function getDate() {
    return $this->uisinfoDatastream->poDate;
  }
  
  function getDepartment() {
    return $this->uisinfoDatastream->department->name;
  }
  
  protected function getPid() {
    return 'islandora-dm:po-' . $this->number;
  }
  
  function getSupplier() {
    return $this->uisinfoDatastream->supplier->name;
  }
  
  function getRequisitionNumber() {
    return $this->uisinfoDatastream->requisitionNumber;
  }
  
  function getStatus() {
    $status = ($this->uis) ? $this->uis->status : $this->dminfoDatastream->status;
    $status = ucfirst(strtolower($status));
    return $status;
  }
  
  function getOwner() {
    return $this->dminfoDatastream->owner;
  }
  
  function getReceivedDate() {
    return $this->uis->receivedDate;
  }
  
  function getFedora() {
    return $this->fedora;
  }
  
  function getUIS() {
    return $this->uis;
  }
  
  /**
   * Create a themed drupal view of the purchase order. Used to create the detailed, single-po view page.
   */
  function getView() {
    $view = '';
    
    $this->addViewErrors();

    // return an empty view (with errors) if fedora is empty.
    if (!$this->fedora) {
      return $view;
    }
 
    // construct view
    $rows = array(
      array(t('Date:'), check_plain($this->getDate()), null),
      array(t('Requisition:'), check_plain($this->getRequisitionNumber()), null),
      array(t('Department:'), check_plain($this->getDepartment() . ' (' . $this->uisinfoDatastream->department->number .')'), null),
      array(t('Supplier:'), check_plain($this->getSupplier() . ' (' . $this->uisinfoDatastream->supplier->number .')'), null),
      array(t('Owner:'), check_plain($this->getOwner()), null),
      array(t('Status:'), check_plain($this->getStatus()), null),
      array(t('Received Date:'), check_plain($this->getReceivedDate()), null),
    );
    
    $view .= theme('table', null, $rows);

    $view .= '<h3>Documents</h3>';

    foreach ($this->documentPids as $document_pid) {
      $view .= '<div class="thumbnail left">' . $this->getDocumentThumbnailImgTag($document_pid) . '</div>';
    }

    $view .= '<br clear=all>';

    $view .= '<h3>Attachments</h3>';
    $view .= '<p>todo</p>';

    return $view;
  }
  
  /**
   * Create a themed drupal view of the purchase order for use in search result lists.
   */
  function getSearchResultView() {
    $view = '';
    
    $this->addViewErrors();

    // return an empty view (with errors) if fedora is empty.
    if (!$this->fedora) {
      return $view;
    }
    
    drupal_add_js(drupal_get_path('module', 'islandora_dm') . '/js/search.js');
    
    $view .= '<div class="po clickable">';
    $view .= '<div class="po-number">' . t('PO #') . ' <a href="' . base_path() . 'islandora_dm/po/' . $this->number . '/view">' . $this->number . '</a></div>';

    if (!empty($this->documentPids)) {
      $document_pid = $this->documentPids[0];
      $view .= '<div class="thumbnail right">' . $this->getDocumentThumbnailImgTag($document_pid) . '</div>';
    }
    
    $view .= '<table><tbody>';
    $view .= '<tr><td>' . t('Date:') . '</td><td>' . check_plain($this->getDate()) . '</td></tr>';
    $view .= '<tr><td>' . t('Department:') . '</td><td>' . check_plain($this->getDepartment()) . '</td></tr>';
    $view .= '<tr><td>' . t('Supplier:') . '</td><td>' . check_plain($this->getSupplier()) . '</td></tr>';
    $view .= '<tr><td>' . t('Requisition:') . '</td><td>' . check_plain($this->getRequisitionNumber()) . '</td></tr>';
    $view .= '<tr><td>' . t('Status:') . '</td><td>' . check_plain($this->getStatus()) . '</td></tr>';
    $view .= '<tr><td>' . t('Owner:') . '</td><td>' . check_plain($this->getOwner()) . '</td></tr>';
    $view .= '</tbody></table>';
    $view .= '</div>';
    
    return $view;
  }
  
  function getDocumentThumbnailImgTag($document_pid) {
    // TODO pull url components from settings
    return '<img src="http://islandora.local:8080/fedora/get/' . $document_pid . '/tn" width="85" height="110" alt="' . t('document image') . '">';
  }
  
  function addViewErrors() {
    // empty fedora is an error.
    if (!$this->fedora) {
      drupal_set_message(t('Unable to retrieve purchase order ' . $this->number . '.'), 'error');
    }
    
    // empty uis data is a warning
    // only set if fedora was ok
    else if (!$this->uis) {
      drupal_set_message(t('Unable to retrieve UIS data for purchase order ' . $this->number . '.'), 'warning');
    }
  }
  
  static function fetchByPID($pid) {
    $po_number = self::pidToNumber($pid);
    return self::fetchByNumber($po_number);
  }
  
  static function fetchByNumber($po_number) {
    $po = new PurchaseOrder($po_number);
    
    // fetch from fedora
    $po->fedora = fetch_fedora_item($po->getPid());
    
    // fetch from uis (for import, or to access volatile fields)
    $po->uis = fetch_uis_po($po_number);

    // only continue populating if fedora fetch succeeded
    if ($po->fedora) {
      // convert fedora streams to xml
      $po->dminfoDatastream = self::datastreamToXml('dminfo', $po->fedora);
      $po->uisinfoDatastream = self::datastreamToXml('uisinfo', $po->fedora); 
      
      // load document ids
      $po->documentPids = fetch_document_pids($po->getPid());
    }
    
    return $po;
  }
  
  protected static function pidToNumber($pid) {
    return str_replace('islandora-dm:po-', '', $pid);
  }
  
  protected static function datastreamToXml($datastream_id, $fedora_item) {
    $ds = $fedora_item->get_datastream_dissemination($datastream_id);
    return new SimpleXmlElement($ds);
  }
}

