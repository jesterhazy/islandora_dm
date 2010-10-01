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
   * map of document pids -> array(page pids) containing of details 
   * related (child) documents and their pages.
   */
  protected $documents;
  
  
  /**
   * list of note pids related to this purchase order. sorted by pid, 
   * which should be same as insertion order. 
   */
  protected $notes;
  
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
  
  /**
   * Find the next number to use for pid construction
   */
  function nextNumber($pids) {
    $numbers = empty($pids) ? NULL : array_map(array($this, 'numberFromPid'), $pids);
    return empty($numbers) ? 1 : max($numbers) + 1;
  }
  
  function numberFromPid($pid) {
    return preg_match('/([0-9]+)$/', $pid, $m) ? $m[1] : 0;
  }
  
  function nextNoteNumber() {
    return $this->nextNumber($this->notes);
  }
  
  function nextDocumentNumber() {
    return $this->nextNumber(empty($this->documents) ? NULL : array_keys($this->documents));
  }
  
  function nextPageNumber($document_pid) {
    return $this->nextNumber($this->documents[$document_pid]);
  }
  
  function notePid($note_number) {
    return $this->getPid() . '-n' . $note_number;
  }
  
  function noteTitle($note_number) {
    return $this->getTitle() . ' Note ' . $note_number;
  }
  
  function addDocument($document_pid) {
    if (!isset($this->documents[$document_pid])) {
      $this->documents[$document_pid] = array();
    }
  }
  
  function documentPid($document_number) {
    return $this->getPid() . '-d' . $document_number;
  }
  
  function documentTitle($document_number) {
    return $this->getTitle() . ' Document ' . $document_number;
  }
  
  function pageCount($document_pid) {
    return count($this->documents[$document_pid]);
  }
  
  function addPage($document_pid, $page_pid) {
    $this->documents[$document_pid][] = $page_pid;
  }
  
  function pagePid($document_pid, $page_number) {
    return $this->documentPid($this->numberFromPid($document_pid)) . '-p' . $page_number;
  }
  
  function pageTitle($document_pid, $page_number) {
    return $this->documentTitle($this->numberFromPid($document_pid)) . ' Page ' . $page_number;
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
      array(array('data' => t('Department:'), 'class' => 'label'), check_plain($this->getDepartment() . ' (' . $this->uisDatastream->idept .')'), null),
      array(array('data' => t('Supplier:'), 'class' => 'label'), check_plain($this->getSupplier() . ' (' . $this->uisDatastream->supplier->isup .')'), null),
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
        $note = Note::fetchByPid($note_pid);
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
    
    if ($this->dispositionDatastream) {
      $ds = $this->dispositionDatastream;
      
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
    
    if ($this->classificationDatastream) {
      $ds = $this->classificationDatastream;
      
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
      
      // load notes
      $po->notes = fetch_notes_pids($po->getPid());
      
      // load documents/pages map
      $po->documents = fetch_documents($po->getPid());
    }
    
    return $po;
  }
  
  protected static function pidToNumber($pid) {
    return str_replace('islandora-dm:po-', '', $pid);
  }
}

