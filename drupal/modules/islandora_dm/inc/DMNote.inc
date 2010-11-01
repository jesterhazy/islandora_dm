<?php

/*
 * DMNote - methods for retrieving, displaying, and manipulating notes.
 */
class DMNote extends DMObject {
  public $xml;
  public $created;
  
  function typeLabel() {
    return 'Note';
  }
  
  function createFedoraItem() {
    parent::createFedoraItem();
    $this->fedora->add_relationship('hasModel', Constants::PID_PO_NOTE_CMODEL, FEDORA_MODEL_URI);
  }
  
  function addNoteDatastream($text, $username) {
    $xml = '<note><text>' . $text . '</text><creator>' . $username . '</creator></note>';    
    $this->fedora->add_datastream_from_string($xml, Constants::DSID_NOTE, Constants::DSLABEL_NOTE, Constants::MIME_TYPE_XML, 'X');
  }
  
  function view() {
    $view = '';
    $view .= '<div class="note">';
    $view .= '<div class="text">' . preg_replace('/\n/', '<br>', check_plain($this->xml->text)) . '</div>';
    $view .= '<div class="caption">' . check_plain($this->created) . ' - ' . check_plain($this->xml->creator) . '</div>';
    $view .= '</div>';
    return $view;
  }
  
  static function createNote($po_number, $text, $username) {
    $po_pid = DMPurchaseOrder::number2pid($po_number);
    try {
        $obj = new DMNote();
        $obj->createFedoraItem();
        $obj->joinCollection($po_pid);
        $obj->addNoteDatastream($text, $username);        
      }
      catch (Exception $e) {
          watchdog(Constants::MODULE_NAME, 'Error while creating fedora object %pid: @e', array('%pid' => $obj->pid(), '@e' => $e->getMessage()), WATCHDOG_WARN);
          $obj = null; 
      }

      return $obj;
  }
  
  static function fetchByPid($pid) {
    $note = new DMNote();
    $note->fedora = fetch_fedora_item($pid);
    $note->xml = datastream_to_xml(Constants::DSID_NOTE, $note->fedora);
    $note->created = format_date(strtotime($note->fedora->objectProfile->objCreateDate));
    return $note;
  }
}

/*
 * Drupal callback handlers
 */
function islandora_dm_add_note_form(&$form_state, $po_number) {
  $form = array();
      
  $form['po_number'] = array(
    '#type' => 'value',
    '#value' => $po_number,
  );

  $form['text'] = array(
    '#type' => 'textarea',
    '#title' => t('Note'),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#name' => 'save',
    '#value' => t('Save'),
  );

  $form['cancel'] = array(
    '#type' => 'submit',
    '#name' => 'cancel',
    '#value' => t('Cancel'),
  );

  return $form;
}

function islandora_dm_add_note_form_validate($form, &$form_state) {
  if ($form_state['clicked_button']['#name'] == 'save' ) {
    if ($form_state['values']['text'] == '') {
      form_set_error('text', t('Note field is required.'));
    }
  }
}

function islandora_dm_add_note_form_submit($form, &$form_state) {
  if ($form_state['clicked_button']['#name'] == 'save' ) {
    global $user;
    $po_number = $form_state['values']['po_number'];
    $text = $form_state['values']['text'];

    $note = DMNote::createNote($po_number, $text, $user->name);
    drupal_set_message(t('Added note %n to purchase order %p.', array('%n' => $note->pid(), '%p' => $po_number)));
  }
  
  $form_state['redirect'] = array('islandora_dm/po/' . $form_state['values']['po_number'] . '/view');
}