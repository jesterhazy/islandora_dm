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
   * SimpleXMLElement object containing dmDatastream from fedora
   */
  protected $dmDatastream;
  
  /**
   * SimpleXMLElement object containing uisDatastream from fedora
   */
  protected $uisDatastream;
  
  /**
   * SimpleXMLElement object containing classificationDatastream from fedora
   */
  protected $classificationDatastream;
  
  /**
   * SimpleXMLElement object containing dispositionDatastream from fedora
   */
  protected $dispositionDatastream;

  
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
    return substr($this->uisDatastream->podate, 0, strpos($this->uisDatastream->podate, 'T'));
  }
  
  function getDepartment() {
    return $this->uisDatastream->dept;
  }
  
  function getPid() {
    return 'islandora-dm:po-' . $this->number;
  }
  
  function getTitle() {
    return t('Purchase Order ') . $this->number;
  }
  
  function getSupplier() {
    return $this->uisDatastream->supplier->name;
  }
  
  function getRequisitionNumber() {
    return substr($this->uisDatastream->re, strpos($this->uisDatastream->re, '#') + 1);
  }
  
  function getStatus() {
    return 'processed';
  }
  
  function getOwner() {
    return $this->dmDatastream->owner;
  }
  
  function getFedora() {
    return $this->fedora;
  }
  
  function getUIS() {
    return $this->uis;
  }
  
  function documentCount() {
    return count($this->documentPids);
  }
    
  /**
   * Create a themed drupal view of the purchase order. Used to create the detailed, single-po view page.
   */
  function getView() {
    $view = '<div class="islandora-dm-po-view">';
    
    $this->addViewErrors();

    // return an empty view (with errors) if fedora is empty.
    if (!$this->fedora) {
      return $view;
    }
 
    // construct view
    $rows = array(
      array(array('data' => t('Date:'), 'class' => 'label'), check_plain($this->getDate()), null),
      array(array('data' => t('Requisition:'), 'class' => 'label'), check_plain($this->getRequisitionNumber()), null),
      array(array('data' => t('Department:'), 'class' => 'label'), check_plain($this->getDepartment() . ' (' . $this->uisDatastream->idept .')'), null),
      array(array('data' => t('Supplier:'), 'class' => 'label'), check_plain($this->getSupplier() . ' (' . $this->uisDatastream->supplier->isup .')'), null),
      array(array('data' => t('Owner:'), 'class' => 'label'), check_plain($this->getOwner()), null),
      array(array('data' => t('Status:'), 'class' => 'label'), check_plain($this->getStatus()), null),
    );
        
    $view .= theme('table', null, $rows);
    
    $view .= $this->classificationView();
    
    $view .= $this->dispositionView();
    
    $view .= '<h3>Documents</h3>';

    foreach ($this->documentPids as $document_pid) {
      $view .= '<div class="thumbnail left">' . $this->getDocumentThumbnailImgTag($document_pid) . '</div>';
    }

    $view .= '<br clear=all>';

    $view .= '<h3>Attachments</h3>';
    $view .= '<p>todo</p>';

    $view .= "</div>";
    return $view;
  }
  
  function dispositionView() {
    $view = '';
    
    if ($this->dispositionDatastream) {
      $ds = $this->dispositionDatastream;
      
      $view = '<h3>Disposition</h3>';

      $rows = array(
        array(array('data' => t('Disposition Action:'), 'class' => 'label'), check_plain($ds->dispositionAction), null),
        array(array('data' => t('Retention Period:'), 'class' => 'label'), check_plain($ds->retentionPeriod), null),
        array(array('data' => t('Disposition Date:'), 'class' => 'label'), check_plain($ds->dispositionDate), null),
        array(array('data' => t('Disposition Reason:'), 'class' => 'label'), check_plain($ds->dispositionReason), null),
        array(array('data' => t('Disposition Decision User:'), 'class' => 'label'), check_plain($ds->dispositionDecisionUser), null),
      );

      $view .= theme('table', null, $rows);
    }
  
    return $view;
  }
  
  function classificationView() {
    $view = '';
    
    if ($this->classificationDatastream) {
      $ds = $this->classificationDatastream;
      
      $view = '<h3>Classification</h3>';

      $rows = array(
        array(array('data' => t('Office of Record:'), 'class' => 'label'), check_plain($ds->officeOfRecord), null),
        array(array('data' => t('Unit of Record:'), 'class' => 'label'), check_plain($ds->unitOfRecord), null),
        array(array('data' => t('Depositor of Record:'), 'class' => 'label'), check_plain($ds->depositorOfRecord), null),
        array(array('data' => t('Date of Deposit:'), 'class' => 'label'), check_plain($ds->dateOfDeposit), null),
        array(array('data' => t('Classification Code:'), 'class' => 'label'), check_plain($ds->classificationCode), null),
      );

      $view .= theme('table', null, $rows);
    }
  
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
    
    drupal_add_js(drupal_get_path('module', Constants::MODULE_NAME) . '/js/search.js');
    
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
    $doc = fetch_fedora_item($document_pid);
    $ds_list = $doc->get_datastreams_list_as_array();
    
    $tn_url = base_path() . drupal_get_path('module', Constants::MODULE_NAME)  . '/img/tn-placeholder.gif';
    
    if (!empty($ds_list['tn'])) {
      global $base_url;
      $tn_url = $base_url . '/fedora/repository/' . $document_pid . '/tn';
    }
    
    return '<img src="' . $tn_url . '" width="85" height="110" alt="' . t('document image') . '">';
  }
  
  function addViewErrors() {
    // empty fedora is an error.
    if (!$this->fedora) {
      drupal_set_message(t('Unable to retrieve purchase order ' . $this->number . '.'), Constants::DRUPAL_MESSAGE_ERROR);
    }
    
    // empty uis data is a warning
    // only set if fedora was ok
    else if (!$this->uis) {
      drupal_set_message(t('Unable to retrieve UIS data for purchase order ' . $this->number . '.'), Constants::DRUPAL_MESSAGE_WARNING);
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
      $po->dmDatastream = datastream_to_xml('dm', $po->fedora);
      $po->uisDatastream = datastream_to_xml('uis', $po->fedora);
      $po->classificationDatastream = datastream_to_xml('classification', $po->fedora); 
      $po->dispositionDatastream = datastream_to_xml('disposition', $po->fedora);
      
      // load document ids
      $po->documentPids = fetch_document_pids($po->getPid());
    }
    
    return $po;
  }
  
  protected static function pidToNumber($pid) {
    return str_replace('islandora-dm:po-', '', $pid);
  }
}

