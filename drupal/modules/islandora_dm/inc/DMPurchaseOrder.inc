<?php

/*
 * DMPurchaseOrder - methods for retrieving, displaying, and manipulating purchase orders.
 */
class DMPurchaseOrder extends DMObject {
  const PID_PREFIX = 'islandora-dm:po-';
  
  /**
   * The purchase order number.
   */
  protected $number;
  
  /**
   * SimpleXMLElement object containing up to date UIS data
   */
  protected $uis;
  
  /**
   * SimpleXMLElement object containing dmXml from fedora
   */
  protected $dmXml;
  
  /**
   * SimpleXMLElement object containing uisXml from fedora
   */
  protected $uisXml;
  
  /**
   * SimpleXMLElement object containing classificationXml from fedora
   */
  protected $classificationXml;
  
  /**
   * SimpleXMLElement object containing dispositionXml from fedora
   */
  protected $dispositionXml;

  /**
   * map of document pids -> array(page pids) containing of details 
   * related (child) documents and their pages.
   */
  protected $documents;
  
  
  /**
   * list of note pids related to this purchase order. sorted by pid, 
   * which should be same as insertion order. 
   */
  protected $notes;
  
  function typeLabel() {
    return 'Purchase Order';
  }
  
  function generatePid() {
    return self::number2pid($this->number);
  }
  
  function generateTitle($pid) {
    return $this->typeLabel() . ' ' . self::pid2number($pid);
  }
  
  function createFedoraItem() {
    parent::createFedoraItem();
    $this->fedora->add_relationship('hasModel', Constants::PID_PO_CMODEL, FEDORA_MODEL_URI);
  }
  
  function addDMDatastream() {
    $xml = '<dm><owner>unassigned</owner></dm>';
    $this->fedora->add_datastream_from_string($xml, Constants::DSID_DM, Constants::DSLABEL_DM, Constants::MIME_TYPE_XML, 'X');
  }
  
  function addUISDatastream() {
    $xml = $this->uis->asXML();
    $this->fedora->add_datastream_from_string($xml, Constants::DSID_UIS, Constants::DSLABEL_UIS, Constants::MIME_TYPE_XML, 'X');
  }
  
  function getDate() {
    return substr($this->uisXml->podate, 0, strpos($this->uisXml->podate, 'T'));
  }
  
  function getDepartment() {
    return $this->uisXml->dept;
  }
  
  function getSupplier() {
    return $this->uisXml->supplier->name;
  }
  
  function getRequisitionNumber() {
    return substr($this->uisXml->re, strpos($this->uisXml->re, '#') + 1);
  }
  
  function getStatus() {
    return 'processed';
  }
  
  function getOwner() {
    return $this->dmXml->owner;
  }
  
  /**
   * Create a themed drupal view of the purchase order. Used to create the detailed, single-po view page.
   */
  function getView() {
    $view = "\n\n";
    $view = '<div class="islandora-dm-po-view">';
    
    $this->addErrorMessages();

    // return an empty view (with errors) if fedora is empty.
    if (!$this->fedora) {
      return $view;
    }
     
    // construct view
    $rows = array(
      array(array('data' => t('Date:'), 'class' => 'label'), check_plain($this->getDate()), null),
      array(array('data' => t('Requisition:'), 'class' => 'label'), check_plain($this->getRequisitionNumber()), null),
      array(array('data' => t('Department:'), 'class' => 'label'), check_plain($this->getDepartment() . ' (' . $this->uisXml->idept .')'), null),
      array(array('data' => t('Supplier:'), 'class' => 'label'), check_plain($this->getSupplier() . ' (' . $this->uisXml->supplier->isup .')'), null),
      array(array('data' => t('Owner:'), 'class' => 'label'), check_plain($this->getOwner()), null),
      array(array('data' => t('Status:'), 'class' => 'label'), check_plain($this->getStatus()), null),
    );
        
    $view .= theme('table', null, $rows);
    
    $view .= $this->addClassificationView();
    $view .= $this->addDispositionView();
    $view .= $this->addNotesView();
    $view .= $this->addDocumentsView();
    
    $view .= "\n\n";
    $view .= '<h3>Attachments</h3>';
    $view .= '<p>todo</p>';


    $view .= "</div>";    
    $view .= "\n\n";
    
    return $view;
  }
  
  function pageCount($document_pid) {
    return count($this->documents[$document_pid]);
  }
  
  function addDocumentsView() {
    $view = "\n\n";
    $view .= '<h3>Documents</h3>';
    
    foreach (array_keys($this->documents) as $document_pid) {
      $pages = $this->pageCount($document_pid);
      $view .= '<div class="document">';
      $view .= '<div class="thumbnail">';
      $view .= $this->getDocumentThumbnailImgTag($document_pid);
      $view .= '</div><div class="caption">';
      $view .= $pages . ' page' . ($pages != 1 ? 's' : '');
      $view .= '</div></div>';
    }

    $view .= '<br clear=all>';
    
    return $view;
  }
  
  function addNotesView() {
    $view = "\n\n";
    $view .= '<div class="notes"><h3>Notes</h3>';
    
    if(!empty($this->notes)) {
      foreach ($this->notes as $note_pid) {
        $note = DMNote::fetchByPid($note_pid);
        $view .= $note->view();
      }
    }
    
    $url = base_path() . 'islandora_dm/po/' . $this->number . '/add_note';
    $view .= '<div class="button"><a href="' . $url . '"><button>' . t('Add Note') .'</button></a></div>';
    $view .= '</div>';
    return $view;
  }
  
  function addDispositionView() {
    $view = "\n\n";
    
    if ($this->dispositionXml) {
      $ds = $this->dispositionXml;
      
      $view .= '<h3>Disposition</h3>';

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
  
  function addClassificationView() {
    $view = "\n\n";
    
    if ($this->classificationXml) {
      $ds = $this->classificationXml;
      
      $view .= '<h3>Classification</h3>';

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
    $view = "\n\n";
    
    $this->addErrorMessages();

    // return an empty view (with errors) if fedora is empty.
    if (!$this->fedora) {
      return $view;
    }
    
    drupal_add_js(drupal_get_path('module', Constants::MODULE_NAME) . '/js/search.js');
    
    $view .= '<div class="po clickable">';
    $view .= '<div class="po-number">' . t('PO #') . ' <a href="' . base_path() . 'islandora_dm/po/' . $this->number . '/view">' . $this->number . '</a></div>';

    if (!empty($this->documents)) {
      $document_pid = array_shift(array_keys($this->documents));
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
    // setup placeholder
    $tn_url = base_path() . drupal_get_path('module', Constants::MODULE_NAME)  . '/img/tn-placeholder.gif';
  
    $page_pids = $this->documents[$document_pid];
      
    if (!empty($page_pids)) {
      $page_pid = $page_pids[0];
      $page = fetch_fedora_item($page_pid);
  
      $ds_list = $page->get_datastreams_list_as_array();

        if (!empty($ds_list[Constants::DSID_TN])) {
          global $base_url;
          $tn_url = $base_url . '/fedora/repository/' . $page_pid . '/tn';
        }  
    }
  
    return '<img src="' . $tn_url . '" width="85" height="110" alt="' . t('document image') . '">';
  }
  
  function addErrorMessages() {
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
  
  static function number2pid($po_number) {
    return self::PID_PREFIX . $po_number;
  }
  
  static function pid2number($pid) {
    return str_replace(self::PID_PREFIX, '', $pid);    
  }
  
  /**
   * Retrieve and return the fedora record for this $po_number,
   * or NULL if the item is not found.
   */
  static function fetchFedora($po_number) {
    $pid = self::number2pid($po_number);
    $fedora = fetch_fedora_item($pid);
    return $fedora;
  }
  
  /**
   * Retrieve and return the uis record for this $po_number,
   * or NULL if the item is not found.
   */
  static function fetchUIS($po_number) {
    return fetch_uis_po($po_number);
  }
  
  static function fetchByPID($pid) {
    $po_number = self::pid2Number($pid);    
    return self::fetchByNumber($po_number);
  }
  
  static function fetchByNumber($po_number) {
    $po = new DMPurchaseOrder();
    $po->number = $po_number;
    
    // get fedora record
    $po->fedora = self::fetchFedora($po_number);
    
    // get uis record
    $po->uis = self::fetchUIS($po_number);
    
    // only continue populating if fedora fetch succeeded
    if ($po->fedora) {
      // convert fedora streams to xml
      $po->dmXml = datastream_to_xml('dm', $po->fedora);
      $po->uisXml = datastream_to_xml('uis', $po->fedora);
      $po->classificationXml = datastream_to_xml('classification', $po->fedora); 
      $po->dispositionXml = datastream_to_xml('disposition', $po->fedora);
      
      // load notes pids
      $po->notes = fetch_notes($po->pid());
      
      // load documents/pages pid map
      $po->documents = fetch_documents($po->pid());
    }
    
    return $po;
  }
  
  static function import($po_number, $uis) {
    try {
      $obj = new DMPurchaseOrder();
      $obj->number = $po_number;
      $obj->uis = $uis;
      $obj->createFedoraItem();      
      $obj->joinCollection(Constants::PID_PO_COLLECTION);
      $obj->addDMDatastream();
      $obj->addUISDatastream();
      
      // TODO objectify
      add_collection_datastreams($obj->fedora);
    }
    
    catch (Exception $e) {
      watchdog(Constants::MODULE_NAME, 'Error while creating fedora object %pid: @e', array('%pid' => $obj->pid(), '@e' => $e), WATCHDOG_WARN);
      $obj = null;
    }
    
    return $obj;
  }
}