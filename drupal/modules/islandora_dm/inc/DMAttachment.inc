<?php

/*
 * DMAttachment - methods for retrieving, displaying, and manipulating notes.
 */
class DMAttachment extends DMNote {
  
  function typeLabel() {
    return 'Attachment';
  }
  
  function createFedoraItem() {
    DMObject::createFedoraItem();
    $this->fedora->add_relationship('hasModel', Constants::PID_PO_ATTACHMENT_CMODEL, FEDORA_MODEL_URI);
  }
  
  function view() {
    $view = '<div class="attachment">';
    $view .= '<div class="file">File: ';
    $view .= $this->attachmentLink();
    $view .= '</div>';
    
    // add note
    $view .= parent::view();
    
    $view .= '</div>';
    return $view;
  }
  
  function attachmentLink() {
    global $base_url;
   
    $ds_info = $this->fedora->get_datastream_info(Constants::DSID_ATTACHMENT);
        
    $label = $ds_info->datastream->label;
    $url = $base_url . '/fedora/repository/' . drupal_urlencode($this->pid()) . '/' . Constants::DSID_ATTACHMENT . '/' . drupal_urlencode($label);
    $link = '<a href="' . $url . '">' . check_plain($label) . '</a>';
    
    return $link;
  }

  function addAttachmentDatastream($file, $username) {
    $label = $file->filename;
    $this->fedora->add_datastream_from_file($file->filepath, Constants::DSID_ATTACHMENT, $label, NULL, 'M');
  }
    
  static function createAttachment($po_number, $file, $text = '', $username) {    
    $po_pid = DMPurchaseOrder::number2pid($po_number);
    try {
        $obj = new DMAttachment();
        $obj->createFedoraItem();
        $obj->joinCollection($po_pid);
        $obj->addAttachmentDatastream($file, $username);
        $obj->addNoteDatastream($text, $username);
      }
      catch (Exception $e) {
          watchdog(Constants::MODULE_NAME, 'Error while creating fedora object %pid: @e', array('%pid' => $obj->pid(), '@e' => $e->getMessage()), WATCHDOG_WARN);
          $obj = null; 
      }
    
      return $obj;
  }
 
  static function fetchByPid($pid) {
    $attachment = new DMAttachment();
    $attachment->fedora = fetch_fedora_item($pid);
    $attachment->noteXml = datastream_to_xml(Constants::DSID_NOTE, $attachment->fedora);
    $attachment->created = format_date(strtotime($attachment->fedora->objectProfile->objCreateDate));    
    return $attachment;
  }
}

/*
 * Drupal callback handlers
 */
function islandora_dm_add_attachment_form(&$form_state, $po_number) {
  $form = array();
  $form['#attributes'] = array('enctype' => 'multipart/form-data');
  $form['po_number'] = array(
    '#type' => 'value',
    '#value' => $po_number,
  );
  
  $form['attachment'] = array(
    '#title' => t('File to attach'),
    '#type'  => 'file',
  );

  $form['description'] = array(
    '#type' => 'textfield',
    '#title' => t('Description'),
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

function islandora_dm_add_attachment_form_submit($form, &$form_state) {
  if ($form_state['clicked_button']['#name'] == 'save' ) {
    global $user;
    $po_number = $form_state['values']['po_number'];
    $description = $form_state['values']['description'];
    
    $file = file_save_upload('attachment');

    if ($file != 0) {
      $attachment = DMAttachment::createAttachment($po_number, $file, $description, $user->name);
      drupal_set_message(t('Added attachment %n to purchase order %p.', array('%n' => $attachment->pid(), '%p' => $po_number)));
    }
    
    else {
      drupal_set_message(t('Failed to save attachment.'), Constants::DRUPAL_MESSAGE_ERROR);
    }    
  }
  
  $form_state['redirect'] = array('islandora_dm/po/' . $form_state['values']['po_number'] . '/view');
}