<?php

class ImportPage {
  const BTN_IMPORT_ALL = 'import_all';
  const NUMBERED_FILENAME_PATTERN = '/^ *([0-9]+) +([0-9\-]{10}) +\(([0-9]+)\) +([0-9]+) *$/';
  const UNNUMBERED_FILENAME_PATTERN = '/^ *([0-9\-]{10}) +\(([0-9]+)\) +([0-9]+) *$/';
  
  function __construct() {
  }
  
  function numberedForm() {
    $form = array();
    
    $files = $this->getNumberedImportFiles();
    if (empty($files)) {
      $form['no_files'] = array(
        '#type' => 'item', 
        '#value' => t('The are no valid files in the import directory.'),
      );
    }
    
    else {
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

      // TODO: group table by date/po/batch
      // TODO: add js to help users add/remove complete docs/batches

      $form['files'] = array(
         '#type' => 'checkboxes',
         '#name' => 'files',
         '#options' =>  $files,
         '#theme' => 'islandora_dm_import_files_checkboxes',
         '#description' => t('Select the files to import, or click the <i>Import All</i> button.')
       );
    }
    
    return $form;
  }
  
  function unnumberedForm() {
    $form = array();
    
    $files = $this->getUnnumberedImportFiles();
    
    if (empty($files)) {
      $form['no_files'] = array(
        '#type' => 'item', 
        '#value' => t('The are no valid files in the import directory.'),
      );
    }
    
    else {
      $form['step1'] = array(
        '#type' => 'item', 
        '#title' => t('Step 1: Enter purchase order number(s)'),
      );
      
      $form['po_numbers'] = array(
         '#type' => 'textfield',
         '#description' => t('Enter one or more purchase order numbers, separated by commas or spaces.'),
         '#size' => 80,
         '#maxlength' => 255,
      );
      
      $form['step2'] = array(
        '#type' => 'item', 
        '#title' => t('Step 2: Select files'),
      );
      
      $form['files'] = array(
         '#type' => 'checkboxes',
         '#name' => 'files',
         '#options' =>  $files,
         '#theme' => 'islandora_dm_import_files_checkboxes',
         '#description' => t('Select the files to import. All of the files you select will be attached to all of the purchase orders indicated above. Dates and batch numbers will be used to organize the files into documents.')
       );
       
       $form['step3'] = array(
         '#type' => 'item', 
         '#title' => t('Step 3: Start the import'),
       );
         
      $form['import_selected'] = array(
        '#type' => 'submit',
        '#name' => 'import_selected',
        '#value' => t('Import Selected')
      );
    }
    
    return $form;
  }
  
  function getFiles() { 
    $dir = variable_get(Constants::IMPORT_FOLDER_PATH, null); 
    $files = Files::listFiles($dir); 
    $this->parseFilenames($files);
    usort($files, array($this, 'compareFiles'));
    return $files;
  }
  
  function getNumberedImportFiles() {
    $numbered = array();
    $files = $this->getFiles();
    
    $i = 1;
    foreach ($files as $file) {
      if ($file->numbered) {
       $numbered[$i++] = $file; 
      }
    }
    
    return $numbered;
  }
  
  function getUnnumberedImportFiles() {
    $unnumbered = array();
    $files = $this->getFiles();
    
    $i = 1;
    foreach ($files as $file) {
      if ($file->unnumbered) {
       $unnumbered[$i++] = $file; 
      }
    }
    
    return $unnumbered;
  }
  
  function getInvalidImportFiles() {
    $invalid = array();
    $files = $this->getFiles();
    
    foreach ($files as $file) {
      if ($file->invalid) {
       $invalid[] = $file; 
      }
    }
    
    return $invalid;
  }

  function compareFiles($f1, $f2) {
    $result = 0;

    if ($result == 0) {
         $result = strcmp($f1->scanDate, $f2->scanDate);
    }

    if ($result == 0) {
         $result = strcmp($f1->poNumber, $f2->poNumber);
    }

    if ($result == 0) {
        $result = $f1->scanBatch - $f2->scanBatch;
    }

    if ($result == 0) {
         $result = $f1->scanPage - $f2->scanPage;
    }

    if ($result == 0) {
         $result = strcmp($f1->name, $f2->name);
    }
   
    return $result;
  }
    
  function themeFileCheckboxes($form) {
    $files = $form['#options'];
    $output = null;
    
    $header = array('', t('Filename'), t('Size'), t('Last Modified'));

    $i = 1; // 1-based to compensate for bad drupal checkbox posts
    $rows[] = array();
    foreach($files as $file) {
      $checkbox = array(
        '#type' => 'checkbox',
        '#name' => $form['#name'] . '[' . $i++ . ']',
      );
      
      $rows[] = array(
        drupal_render($checkbox),
        check_plain($file->basename),
        Files::formatFilesize($file->size),
        Files::formatFilemtime($file->mtime)
      );
    }

    $output = theme('table', $header, $rows);

    return $output;
  }
  
  function numberedFormSubmit($form, &$form_state) {  
    $import_all = (self::BTN_IMPORT_ALL == $form_state['clicked_button']['#name']);
    $selected_files = $this->getSelectedFiles($form, $form_state, $import_all);
    $this->importNumberedFiles($selected_files);
  }
  
  function getSelectedFiles($form, &$form_state, $import_all = FALSE) {
    $selected_files = array();
    
    $i = 1; // 1-based to compensate for bad drupal checkbox posts
    foreach ($form['files']['#options'] as $file) {
      if ($import_all || $form_state['values']['files'][$i]) {
        
        // verify read access
        if (is_readable($file->filename)) {
          $selected_files[] = $file;
        }
        else {
          drupal_set_message(
            t("Can't import @s, because the file permissions are wrong.", 
              array('@s' => $file->basename)), 
            Constants::DRUPAL_MESSAGE_ERROR);        
        }
      }
      
      $i++;
    }
    
    return $selected_files;
  }
  
  function unnumberedFormValidate($form, &$form_state) {
    if (empty($form_state['values']['po_numbers'])) {
      form_set_error('po_numbers', t('One or more purchase order numbers required.'));
    }
    
    else {
      $po_numbers = preg_split('/[, ]+/', $form_state['values']['po_numbers']);
      $i = 0;
      foreach ($po_numbers as $po_number) {
        if (!preg_match('/^[0-9]+$/', $po_number)) {
          form_set_error("po_numbers_$i", t('Invalid purchase order number: @s.', array('@s' => $po_number)));
          $i++;
        }
      }
    }
    
    $selected_files = $this->getSelectedFiles($form, $form_state);
    if (empty($selected_files)) {
      form_set_error('files', t('No files selected.'));
    }
  }
  
  function unnumberedFormSubmit($form, &$form_state) {  
    $selected_files = $this->getSelectedFiles($form, $form_state);
    $po_numbers = preg_split('/[, ]+/', $form_state['values']['po_numbers']);    
    $this->importPurchaseOrders($po_numbers, $selected_files);
  }
  
  function importNumberedFiles($files = array()) {
    // group by poNumber
    $po_files = array();
    foreach($files as $file) {
      $po_files[$file->poNumber][] = $file;
    }
    
    // import each po
    foreach($po_files as $k => $v) {
      try {
        $this->importPurchaseOrders(array($k), $v);
      }
      catch (Exception $e) {
        watchdog(Constants::MODULE_NAME, 'Error importing file @file: @ex', array('@file' => $file->filename, '@ex' => $e->getMessage()), WATCHDOG_WARN);
      }
    }
  }
  
  function importPurchaseOrders($po_numbers, $files) {
    $po_pids = array();
    
    // import the pos
    foreach($po_numbers as $po_number) {
      $ok = $this->importPurchaseOrder($po_number);
      
      if (!$ok) {
        break;
      }
      
      $po_pids[] = DMPurchaseOrder::number2pid($po_number);
    }
    
    // if all po imports succeed, import documents
    $fail_count = count($po_numbers) - count($po_pids);
    if ($fail_count == 0) {
      $this->importDocuments($po_pids, $files);
    }
    
    else {
      drupal_set_message(
        t('Import failed for @x purchase order(s). @y file(s) skipped.',
          array('@x' => $fail_count, '@y' => count($files))),
        Constants::DRUPAL_MESSAGE_ERROR);
    }
  }
  
  function importPurchaseOrder($po_number) {
    $ok = FALSE;
    $fedora = DMPurchaseOrder::fetchFedora($po_number);
    
    if ($fedora) {
      $ok = TRUE; // record already in fedora - no need to import.
    }
    
    else {
      $uis = DMPurchaseOrder::fetchUIS($po_number);

      if ($uis) {
          $po = DMPurchaseOrder::import($po_number, $uis);

          if ($po) {
            drupal_set_message(t('Imported purchase order @s.', array('@s' => $po_number)));
            $ok = TRUE;
          }

          else {
            drupal_set_message(
              t('Import failed for purchase order @s', array('@s' => $po_number)), 
              Constants::DRUPAL_MESSAGE_ERROR);
          }
      }

      else {
        drupal_set_message(
          t("Import failed, because purchase order @s doesn't exist.", 
            array('@s' => $po_number)), 
          Constants::DRUPAL_MESSAGE_ERROR);
      }
    }
    
    return $ok;
  }
  
  function importDocuments($po_pids, $files) {    
    // group by document (which is date + batch)
    $doc_files = array();
    foreach($files as $file) {
      $k = $file->scanDate . $file->scanBatch;
      $doc_files[$k][] = $file;
    }
    
    // import each document
    foreach($doc_files as $k => $v) {
      $this->importDocument($po_pids, $v);
    }
  }
  
  function importDocument($po_pids, $files) {
    
    $document = DMDocument::import($po_pids);
    
    if ($document) {
      drupal_set_message(
        t('Added document @d to purchase order(s) @p.', 
          array('@d' => $document->pid(), '@p' => implode(', ', $po_pids))));
            
      // import pages
      $this->importPages($po_pids, $document->pid(), $files);
    }
    
    else {
      drupal_set_message(
        t('Can\'t create document for @s, because an error occurred while storing the document.', 
          array('@s' =>  implode(', ', $po_pids))),
        Constants::DRUPAL_MESSAGE_ERROR);
    }
  }

  function importPages($po_pids, $document_pid, $files) {   
    foreach($files as $file) {
      $this->importPage($po_pids, $document_pid, $file);
    }
  }
  
  function importPage($po_pids, $document_pid, $file) {
    $page = DMPage::import($po_pids, $document_pid, $file);

    if (!$page) {
      drupal_set_message(
        t("Can't create page for @s, because an error occurred while storing the page.", 
          array('@s' => $file->basename)), 
        Constants::DRUPAL_MESSAGE_ERROR);

      return;
    }

    drupal_set_message(
      t('Added page @p to document @d.', 
        array('@p' => $file->basename, '@d' => $document_pid)));

    // archive the file
    if (!variable_get(Constants::IMPORT_ARCHIVE_PATH, false)) {
      $archived = $this->archiveFile($file);
      if (!$archived) {
        drupal_set_message(
          t("Imported @s, but an error occurred while archiving the file.", 
            array('@s' => $file->basename)), 
          Constants::DRUPAL_MESSAGE_ERROR);
      }
    }
  }


  protected function getArchiveDir() {
    $base_dir = variable_get(Constants::IMPORT_ARCHIVE_PATH, null);
    $date = date(Constants::IMPORT_ARCHIVE_DATE_FORMAT);
    $archive_dir = $base_dir . '/' . $date;
    return $archive_dir;
  }

  protected function archiveFile($file) {
    $archive_dir = $this->getArchiveDir();
    $md = mkdir($archive_dir);

    if ($md) {
      $dest = $archive_dir . '/' . $file->basename;

      $cp = copy($file->filename, $dest);

      if ($cp) {
        $del = unlink($file->filename);
      }
    }

    $error = $md ? 'make archive directory for' : $cp ? 'copy' : $del ? 'delete' : null;

    if ($error) {
      watchdog(Constants::MODULE_NAME, t('Failed to ' . $error . ' import file: ' . $file->basename), null, WATCHDOG_WARN);
    }

    return $cp && $del;
  }

  protected function parseFilenames(&$files) {
    foreach ($files as $file) {
      $this->parseFilename($file);
    }
  }

  protected function parseFilename(&$file) {
    if (preg_match(self::NUMBERED_FILENAME_PATTERN, $file->name, $m)) {
      $file->numbered = TRUE;
      $file->poNumber = $m[1];
      $file->scanDate = $m[2];
      $file->scanBatch = $m[3] + 0;
      $file->scanPage = $m[4] + 0;
    }
    elseif (preg_match(self::UNNUMBERED_FILENAME_PATTERN, $file->name, $m)) {
      $file->unnumbered = TRUE;
      $file->scanDate = $m[1];
      $file->scanBatch = $m[2] + 0;
      $file->scanPage = $m[3] + 0;
    }
    else {
      $file->invalid = TRUE;
    }
  }
  
  function viewInvalidFiles() {
    $invalid_files = $this->getInvalidImportFiles();
    
    if (empty($invalid_files)) {
        $output = t("<p>There are no files with invalid filename in the import folder.</p>\n");
      }
      else {
        $output = t("<p>@n file(s) have invalid filenames, and cannot be imported:</p>\n", array('@n' => count($invalid_files)));
        
        $output .= "<ul>\n";
        foreach ($invalid_files as $file) {
          $output .= "<li>" . Files::extractBasename($file) . "</li>\n";
        }
        
        $output .= "</ul>\n";
      }
    
    return $output;
  }
}

/*
 * Drupal callback adapters - map drupal callbacks onto class methods.
 */
function islandora_dm_import_numbered_form() {
  $page = new ImportPage();
  return $page->numberedForm();
}

function islandora_dm_import_numbered_form_submit($form, &$form_state) {
  $page = new ImportPage();
  return $page->numberedFormSubmit($form, $form_state);
}

function islandora_dm_import_unnumbered_form() {
  $page = new ImportPage();
  return $page->unnumberedForm();
}

function islandora_dm_import_invalid() {
  $page = new ImportPage();
  return $page->viewInvalidFiles();
}

function islandora_dm_import_unnumbered_form_submit($form, &$form_state) {
  $page = new ImportPage();
  return $page->unnumberedFormSubmit($form, $form_state);
}

function islandora_dm_import_unnumbered_form_validate($form, &$form_state) {
  $page = new ImportPage();
  return $page->unnumberedFormValidate($form, $form_state);
}

function theme_islandora_dm_import_files_checkboxes($form) {
  $page = new ImportPage();
  return $page->themeFileCheckboxes($form);
}

