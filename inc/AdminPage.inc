<?php
define('ISLANDORA_DM_IMPORT_FOLDER_PATH', 'islandora_dm_import_folder_path');
define('ISLANDORA_DM_IMPORT_ARCHIVE_PATH', 'islandora_dm_import_archive_path');

class AdminPage {
  function AdminPage() {
    $form = array();
    
    $form['import'] = array(
       '#type' => 'fieldset',
       '#title' => t('Document import'),
       '#collapsible' => TRUE,
     );
     
     $form['import'][ISLANDORA_DM_IMPORT_FOLDER_PATH] = array(
        '#type' => 'textfield',
        '#title' => t('Path'),
        '#default_value' => variable_get(ISLANDORA_DM_IMPORT_FOLDER_PATH, ''),
        '#size' => 80,
        '#maxlength' => 255,
        '#description' => t('Path to the folder that contains scanned documents waiting for import'),
        '#required' => TRUE
      );
      
      $form['import'][ISLANDORA_DM_IMPORT_ARCHIVE_PATH] = array(
         '#type' => 'textfield',
         '#title' => t('Path'),
         '#default_value' => variable_get(ISLANDORA_DM_IMPORT_ARCHIVE_PATH, ''),
         '#size' => 80,
         '#maxlength' => 255,
         '#description' => t('Path to the folder that used to store scanned documents after import'),
         '#required' => TRUE
       );
    
     $form['po_service'] = array(
       '#type' => 'fieldset',
       '#title' => t('Purchase order service'),
       '#collapsible' => TRUE,
     );

     $form['po_service']['islandora_dm_po_service_url'] = array(
       '#type' => 'textfield',
       '#title' => t('URL'),
       '#default_value' => variable_get('islandora_dm_po_service_url', 'http://localhost/po-service.php'),
       '#size' => 40,
       '#maxlength' => 255,
       '#description' => t('URL for the purchase order web service'),
       '#required' => TRUE
     );

     $form['po_service']['islandora_dm_po_service_user'] = array(
       '#type' => 'textfield',
       '#title' => t('User name'),
       '#default_value' => variable_get('islandora_dm_po_service_user', ''),
       '#size' => 15,
       '#maxlength' => 15,
       '#description' => t('User name for the purchase order web service (optional)'),
       '#required' => FALSE
     );

     $form['po_service']['islandora_dm_po_service_pass'] = array(
       '#type' => 'textfield',
       '#title' => t('Password'),
       '#default_value' => variable_get('islandora_dm_po_service_pass', ''),
       '#size' => 15,
       '#maxlength' => 15,
       '#description' => t('Password for the purchase order web service (optional)'),
       '#required' => FALSE
     );

     return system_settings_form($form);
  }
  
  function installObjectsForm() {
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
  
  function installObjectsFormSubmit($form, &$form_state) {
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
}

/*
 * Drupal callback adapters.
 */
function islandora_dm_admin() {
  $page = new AdminPage();
  return $page->AdminPage();
}

function islandora_dm_install_objects() {
  return drupal_get_form('islandora_dm_install_objects_form');
}

function islandora_dm_install_objects_form() {
  $page = new AdminPage();
  return $page->installObjectsForm();
}

function islandora_dm_install_objects_form_submit($form, &$form_state) {
  $page = new AdminPage();
  return $page->installObjectsFormSubmit($form, $form_state);
}