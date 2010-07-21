<?php

class AdminPage {
  function settingsForm() {
    $form = array();
    
    $form['import'] = array(
       '#type' => 'fieldset',
       '#title' => t('Document import'),
       '#collapsible' => TRUE,
     );
     
     $form['import'][Constants::IMPORT_FOLDER_PATH] = array(
        '#type' => 'textfield',
        '#title' => t('Import Path'),
        '#default_value' => variable_get(Constants::IMPORT_FOLDER_PATH, ''),
        '#size' => 80,
        '#maxlength' => 255,
        '#description' => t('Path to the folder that contains scanned documents waiting for import'),
        '#required' => TRUE
      );
      
     $form['import'][Constants::IMPORT_ARCHIVE_PATH] = array(
       '#type' => 'textfield',
       '#title' => t('Archive Path'),
       '#default_value' => variable_get(Constants::IMPORT_ARCHIVE_PATH, ''),
       '#size' => 80,
       '#maxlength' => 255,
       '#description' => t('Path to the folder that used to store scanned documents after import'),
       '#required' => TRUE
     );
      
     $form['import'][Constants::IMPORT_ARCHIVE_DISABLED] = array(
       '#type' => 'checkbox',
       '#title' => t('Disable archiving?'),
       '#default_value' => variable_get(Constants::IMPORT_ARCHIVE_DISABLED, false),
       '#description' => t('Disable archiving of imported files (useful for testing)?'),
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
  
  function objectsForm() {
    $objects_dir = drupal_get_path('module', Constants::MODULE_NAME) . '/etc/foxml';

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
  
  function objectsFormSubmit($form, &$form_state) {
    $objects_dir = drupal_get_path('module', Constants::MODULE_NAME) . '/etc/foxml';

    $error = FALSE;

    if ($form_state['clicked_button']['#name'] == 'install' ) {
      foreach($form_state['values']['objects'] as $object_file => $checked) {
        if ($checked) {

          try {
            ingest_object_from_foxml($objects_dir . '/' . $object_file);
          }

          catch (Exception $e) {
            $error = TRUE;
            drupal_set_message(t($e->getMessage()), Constants::DRUPAL_MESSAGE_ERROR);
          }
        }
      }
    }

    if (!$error) {
      drupal_set_message(t('Fedora objects (re)installed successfully.'));
    }
  }
  
  function jms() {
    $form = array();
        
    $form['process'] = array(
      '#type' => 'fieldset',
      '#title' => t('Start/stop JMS service'),
      '#collapsible' => TRUE,
    );
    
    $process = JMSProcess::createExisting();
    $running = $process->isRunning();
    $status_message = $running ? 'running with PID ' . $process->pid() . '.' : 'not running.';
    
    $form['process']['status']  = array(
      '#type' => 'item',
      '#value' => 'The JMS service is ' . $status_message,
    );
     
    $process_button = $running ? 'stop' : 'start';

    $form['process'][$process_button] = array(
      '#type' => 'submit',
      '#name' => 'jms' . $process_button,
      '#description' => ucfirst($process_button) . 'Stop the JMS service.',
      '#value' => t(ucfirst($process_button)),
    );
    
    $form['settings'] = array(
      '#type' => 'fieldset',
      '#title' => t('JMS client settings'),
      '#collapsible' => TRUE,
    );
    
    $form['settings']['islandora_dm_jms_logfile'] = array(
      '#type' => 'textfield',
      '#title' => t('Log file'),
      '#default_value' => variable_get('islandora_dm_jms_logfile', '/tmp/islandora-dm-jms.log'),
      '#size' => 80,
      '#maxlength' => 255,
      '#description' => t('Location of JMS service logfile (optional).'),
      '#required' => FALSE
    );
    
    $form['settings']['islandora_dm_jms_fedora_url'] = array(
      '#type' => 'textfield',
      '#title' => t('User name'),
      '#default_value' => variable_get('islandora_dm_jms_fedora_url', 'http://localhost:8080/fedora'),
      '#size' => 40,
      '#maxlength' => 255,
      '#description' => t('Base URL for Fedora instance.'),
      '#required' => TRUE
    );
    
    $form['settings']['islandora_dm_jms_fedora_user'] = array(
      '#type' => 'textfield',
      '#title' => t('User name'),
      '#default_value' => variable_get('islandora_dm_jms_fedora_user', 'fedoraAdmin'),
      '#size' => 20,
      '#maxlength' => 20,
      '#description' => t('Fedora user name for JMS service. Requires admin rights.'),
      '#required' => TRUE
    );
    
    $form['settings']['islandora_dm_jms_fedora_pass'] = array(
      '#type' => 'textfield',
      '#title' => t('Password'),
      '#default_value' => variable_get('islandora_dm_jms_fedora_pass', 'fedoraAdmin'),
      '#size' => 20,
      '#maxlength' => 20,
      '#description' => t('Fedora password for JMS service.'),
      '#required' => TRUE
    );
    
    $form = system_settings_form($form);
    $form['#submit'] = array('islandora_dm_admin_jms_submit');
    return $form;
  }
  
  function jmsSubmit($form, &$form_state) {
    $button_name = $form_state['clicked_button']['#name'];
    if ($button_name == 'jmsstart') {
      JMSProcess::start();
    }
    
    else if ($button_name == 'jmsstop') {
      JMSProcess::stop();
    }
    
    else {
      system_settings_form_submit($form, $form_state);
    }
  }
}

/*
 * Drupal callback adapters.
 */
function islandora_dm_admin() {
  $page = new AdminPage();
  return $page->settingsForm();
}

function islandora_dm_admin_objects_form() {
  $page = new AdminPage();
  return $page->objectsForm();
}

function islandora_dm_admin_objects_form_submit($form, &$form_state) {
  $page = new AdminPage();
  return $page->objectsFormSubmit($form, $form_state);
}

function islandora_dm_admin_jms() {
  $page = new AdminPage();
  return $page->jms();
}

function islandora_dm_admin_jms_submit($form, &$form_state) {
  $page = new AdminPage();
  return $page->jmsSubmit($form, $form_state);
}