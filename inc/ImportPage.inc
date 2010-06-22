<?php

class ImportPage {
  const BTN_IMPORT_ALL = 'import_all';
  const VALID_FILENAME_PATTERN = '/^([0-9]+)-([0-9]+)$/';
  function form() {
    $form = array();

    $form['import_all'] = array(
        '#type' => 'submit',
        '#name' => self::BTN_IMPORT_ALL,
        '#value' => t('Import All')
    );

    $form['import_selected'] = array(
        '#type' => 'submit',
        '#name' => 'import_selected',
        '#value' => t('Import Selected')
    );
    
   $files = $this->getFileOptions();

    $form['files'] = array(
        '#type' => 'checkboxes',
        '#name' => 'files',
        '#options' =>  $files,
        '#theme' => 'islandora_dm_import_files_checkboxes',
        '#description' => t('Select the files to import, or click the <i>Import All</i> button.')
    );
    
    return $form;
  }
  
  function getFileOptions() {
    $files = $this->getFiles();
    $options = array();
    
    foreach($files as $file) {
      $options[$file->basename] = $file;
    }
    
    return $options;
  }
  
  function getFiles() {
    $dir = variable_get('islandora_dm_import_folder_path', null);
    return Files::listFiles($dir);
  }
    
  function themeFileCheckboxes($form) {
    $files = $form['#options'];
    $output = null;
        
    if (empty($files)) {
      $item = array(
        '#type' => 'item', 
        '#name' => 'files', 
        '#value' => t('The import folder is empty.'),
      );
      
      $output = drupal_render($item);
    }
    
    else {
      $header = array('', t('Filename'), t('Size'), t('Last Modified'));
  
      $rows[] = array();
      foreach($files as $file) {
        $checkbox = array(
          '#type' => 'checkbox',
          '#name' => $form['#name'] . '[' . check_plain($file->basename) . ']',
        );
        
        $rows[] = array(
          drupal_render($checkbox),
          check_plain($file->basename),
          Files::formatFilesize($file->size),
          Files::formatFilemtime($file->mtime)
        );
      }

      $output = theme('table', $header, $rows);
    }

    return $output;
  }
  
  function submit($form, &$form_state) {
    $import_all = (self::BTN_IMPORT_ALL == $form_state['clicked_button']['#name']);
    
    $selected_files = array();
    
    foreach ($form_state['values']['files'] as $key => $value) {
      if ($import_all || $value) {
        $selected_files[] = $form['files']['#options'][$key];
      }
    }
    
    $this->importFiles($selected_files);
  }
  
  function importFiles($files = array()) {
    foreach ($files as $file) {
      $this->importFile($file);
    }
  }
  
  function importFile($file) {
    // extract po number
    $this->attachPONumber($file);
    if (!$file->poNumber) {
      drupal_set_message(t('Can\'t import <b>') . $file->basename . t('</b>, because the name not valid.'), 'error');
      return;
    }

    // verify read access
    if (!is_readable($file->filename)) {
      drupal_set_message(t('Can\'t import <b>') . $file->basename . t('</b>, because the file permissions are wrong.'), 'error');
      return;
    }
    
    // look up purchase order (in fedora and uis)
    $po = PurchaseOrder::fetchByNumber($file->poNumber);
    if(!$po->getFedora() && !$po->getUIS()) {
      drupal_set_message(t('Can\'t import <b>') . $file->basename . t('</b>, because purchase order <b>' . $file->poNumber . t('</b> doesn\'t exist.')), 'error');
      return;
    }
    
    // create fedora record if needed
    if (!$po->getFedora()) {
      $fedora = $this->createPurchaseOrderRecord($po);
      
      if ($fedora) {
        drupal_set_message(t('Created purchase order <b>') . $file->poNumber . t('</b>.'));
      }
      
      if (!$fedora) {
        drupal_set_message(t('Can\'t import <b>') . $file->basename . t('</b>, because an error occurred while storing the purchase order data.'), 'error');        
        return;   
      }
    }
    
    // create document record, test success
    $document = $this->createDocumentRecord($po, $file);
    if (!$document) {
      drupal_set_message(t('Can\'t import <b>') . $file->basename . t('</b>, because an error occurred while storing the document.'), 'error');              
      return;
    }
    
    $archived = $this->archiveFile($file);
    if (!$archived) {
      drupal_set_message(t('Imported <b>') . $file->basename . t('</b>, but an error occurred while archiving the file.'), 'warn');
      watchdog(t('islandora_dm'), t('Error archiving file after successful import: ' . $file->basename), null, WATCHDOG_WARN);
      return;
    }
    
    drupal_set_message(t('Added file <b>') . $file->basename . t('</b> to purchase order <b>') . $file->poNumber . t('</b>.'));
  }
  
  protected function createPurchaseOrderRecord($po) {
    // TODO implement
    return true;
  }
  
  protected function createDocumentRecord($po, $file) {
    // TODO implement
    return true;
  }
  
  protected function archiveFile($file) {
    // TODO pull archive dir from settings, and copy/delete file
    return true;
  } 
  
  protected function attachPONumber(&$file) {
    $matches = array();    
    if (preg_match(self::VALID_FILENAME_PATTERN, $file->name, $matches)) {
      $file->poNumber = $matches[1];
    }
  }
}

/*
 * Drupal callback adapters - map drupal callbacks onto class methods.
 */
 
function islandora_dm_import() {
  return drupal_get_form('islandora_dm_import_form');
}

function islandora_dm_import_form() {
  $page = new ImportPage();
  return $page->form();
}

function islandora_dm_import_form_submit($form, &$form_state) {
  $page = new ImportPage();
  return $page->submit($form, $form_state);
}

function theme_islandora_dm_import_files_checkboxes($form) {
  $page = new ImportPage();
  return $page->themeFileCheckboxes($form);
}