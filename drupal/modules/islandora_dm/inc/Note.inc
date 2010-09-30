<?php

class Note {
  protected $fedora;
  
  function Note($fedora_item) {
    $this->fedora = $fedora_item;
  }
  
  function text() {
    $text = $this->fedora->get_datastream_dissemination(Constants::DSID_NOTE_TEXT);
    return $text;
  }
  
  function creator() {
    $creator = $this->fedora->get_datastream_dissemination(Constants::DSID_NOTE_CREATOR);
    return $creator;
  }
  
  function created() {
    $created = format_date(strtotime($this->fedora->objectProfile->objCreateDate));
    return $created;
  }
  
  function view() {
    $view = '';
    $view .= '<div class="note">';
    $view .= '<div class="text">' . preg_replace('/\n/', '<br>', check_plain($this->text())) . '</div>';
    $view .= '<div class="caption">' . check_plain($this->created()) . ' - ' . check_plain($this->creator()) . '</div>';
    $view .= '</div>';
    return $view;
  }
  
  function addNoteForm($po_number) {
    $form = array();
        
    $form['po_number'] = array(
      '#type' => 'value',
      '#value' => $po_number,
    );

    $form['text'] = array(
      '#type' => 'textarea',
      '#title' => t('Note'),
      '#required' => TRUE,
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
  
  function addNoteFormSubmit($form, &$form_state) {
    global $user;
    $po_number = $form_state['values']['po_number'];
    $text = $form_state['values']['text'];
    
    $po = PurchaseOrder::fetchByNumber($po_number);
    $note = create_note_fedora_item($po, $text, $user->name);
    drupal_set_message(t('Added note %n to purchase order %p.', array('%n' => $po->nextNoteNumber(), '%p' => $po_number)));
    
    $form_state['redirect'] = array('islandora_dm/po/' . $form_state['values']['po_number'] . '/view');
  }
  
  static function fetchByPid($pid) {
    $fedora_item = fetch_fedora_item($pid);
    $note = new Note($fedora_item);
    return $note;
  }
}

function islandora_dm_add_note_form(&$form_state, $po_number) {
  return Note::addNoteForm($po_number);
}

function islandora_dm_add_note_form_submit($form, &$form_state) {
  return Note::addNoteFormSubmit($form, &$form_state);
}