<?php

function islandora_dm_install_objects() {
  $form = drupal_get_form('islandora_dm_install_objects_form');
  return $form;
}

function islandora_dm_install_objects_form() {
  $objects_dir = drupal_get_path('module', 'islandora_dm') . '/etc/fedora_objects';
  
  $files = file_scan_directory($objects_dir, '\.xml$');
  
  $objects = array();
  foreach ($files as $path => $file) {
    $objects[$file->basename] = check_plain($file->basename);
  }
  
  $form = array();
  $form['objects'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Available objects'),
    '#default_value' => $objects,
    '#options' =>  $objects,
    '#description' => t('Select the objects to be (re)installed in Fedora'),
  );
  
  $form['submit'] = array(
      '#type' => 'submit',
      '#name' => 'install',
      '#value' => t('Install selected')
  );

  return $form;
}

function islandora_dm_install_objects_form_submit($form, &$form_state) {
  $objects_dir = drupal_get_path('module', 'islandora_dm') . '/etc/fedora_objects';
  
  $error = FALSE;
  
  if ($form_state['clicked_button']['#name'] == 'install' ) {
    foreach($form_state['values']['objects'] as $object_file => $checked) {
      if ($checked) {
        
        try {
          ingest_object_from_foxml($objects_dir . '/' . $object_file);
        }
        
        catch (Exception $e) {
          $error = TRUE;
          drupal_set_message(t($e->getMessage()), 'error');
        }
      }
    }
  }
  
  if (!$error) {
    drupal_set_message(t('Fedora objects (re)installed successfully.'));
  }
}

function islandora_dm_home() {
  return unimplemented_page('home');
}

function islandora_dm_about() {
  return unimplemented_page('about');
}

function islandora_dm_import() {
  return unimplemented_page('import');
}

function islandora_dm_po_search_form() {
  return unimplemented_page('search');
}

// TODO build real PO view.
function islandora_dm_po_view($po_number) {
  $po = get_fedora_item('islandora-dm:po-' . $po_number);
  $uis = get_purchase_order($po_number);
  
  // failure to retrieve po from fedora is an error.
  if (!$po || !$uis) {
    drupal_set_message(t('Unable to retrieve purchase order ' . $po_number . '.'), 'error');
    return '';
  }
  
  // failure to get fresh uis data is a warning
  if (!$uis) {
    drupal_set_message(t('Unable to retrieve UIS data for purchase order ' . $po_number . '.'), 'warning');
  }
  
  $dminfo = $po->get_datastream_dissemination('dminfo');
  $uisinfo = $po->get_datastream_dissemination('uisinfo');
  
  $dminfo_xml = new SimpleXmlElement($dminfo);
  $uisinfo_xml = new SimpleXmlElement($uisinfo);
 
  
  $rows = array(
    array(t('Date:'), check_plain($uisinfo_xml->poDate), null),
    array(t('Requisition:'), check_plain($uisinfo_xml->requisitionNumber), null),
    array(t('Department:'), check_plain($uisinfo_xml->department->name . ' (' . $uisinfo_xml->department->number .')'), null),
    array(t('Supplier:'), check_plain($uisinfo_xml->supplier->name . ' (' . $uisinfo_xml->supplier->number .')'), null),
    array(t('Status:'), check_plain($dminfo_xml->status), null),
    array(t('Owner:'), check_plain($dminfo_xml->owner), null),
    array(t('UIS status:'), check_plain($uis->status), null),
    array(t('Received Date:'), check_plain($uis->receivedDate), null),
  );
  
  
  
  $page = '';
  $page .= theme('table', null, $rows);
  
  $page .= '<h3>Documents</h3>';

  $docs = get_children('islandora-dm:po-' . $po_number);
  
  foreach ($docs as $doc) {
    $page .= '<div class="thumbnail" style="border: 1px solid #ccc; float: left; margin: 10px 20px 10px 0;"><img src="http://islandora.local:8080/fedora/get/' . $doc . '/tn" width="85" height="110" alt="document image"></div>';
  }
  
  $page .= '<br clear=all>';

  $page .= '<h3>Attachments</h3>';
  $page .= '<p>todo</p>';
  
  return $page;
}

// TODO delete me
function islandora_dm_fedora_view($pid) {
  return get_object_helper()->getQDC($pid);
}

function unimplemented_page($name) {
  $output = <<<END
<p>todo: implement $name page</p>
END;

  return $output;
}



?>