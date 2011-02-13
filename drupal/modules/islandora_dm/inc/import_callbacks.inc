<?php

function islandora_dm_import_files_form() {
	$imp = new Import();
	$form = array();

	$files = $imp->getFilesWithFormKeys();
	if (empty($files)) {
		$form['no_files'] = array(
			'#type' => 'item', 
			'#value' => t('The are no files in the import directory.'),
		);
	}

	else {
		$form['import_all'] = array(
			'#type' => 'submit',
			'#name' => Import::BTN_IMPORT_ALL,
			'#value' => t('Import All')
		);

		$form['import_selected'] = array(
			'#type' => 'submit',
			'#name' => 'import_selected',
			'#value' => t('Import Selected')
		);

		$form['files'] = array(
			'#type' => 'checkboxes',
			'#name' => 'files',
			'#options' => $files,
			'#theme' => 'islandora_dm_import_files_checkboxes',
			'#description' => t('Select the files to import, or click the <i>Import All</i> button.')
		);
	}
	
	return $form;
}

function theme_islandora_dm_import_files_checkboxes($form) {
	$files = $form['#options'];
	$output = null;
	
	$header = array('', t('Filename'), t('Size'), t('Last Modified'));

	$rows[] = array();
	foreach($files as $key => $file) {
		$checkbox = array(
			'#type' => 'checkbox',
			'#name' => $form['#name'] . '[' . $key . ']',
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

function islandora_dm_import_files_form_submit($form, &$form_state) {
	$imp = new Import();
	$imp->importFiles($form, $form_state);
}

function islandora_dm_import_link_documents_form() {
	$imp = new Import();
	$form = array();
	
	$documents = $imp->loadUnlinkedDocuments();
	
	if (empty($documents)) {
		$form['no_documents'] = array(
			'#type' => 'item', 
			'#value' => t('There are no unlinked documents.'),
		);
	}

	else {
		foreach($documents as $document) {
			$key = 'd-' . $document->pidNumber();
			$form[$key]['start'] = array(
				'#value' => $imp->unlinkedDocumentMarkupStart($document));
		
			$form[$key][$key . '-numbers'] = array(
				'#type' => 'textfield',
				'#title' => t('Purchase Order Number(s)'),
				'#description' => t('Enter one or more purchase order numbers, separated by commas or spaces.'),
				);
			
			$form[$key][$key . '-submit'] = array(
				'#type' => 'submit',
				'#name' => $key . '-submit',
				'#value' => t('Save'),
				);
			
			$form[$key]['end'] = array(
				'#value' => $imp->unlinkedDocumentMarkupEnd($document));
		}
	}
	
	return $form;
}

function islandora_dm_import_link_documents_form_validate($form, &$form_state) {
	$imp = new Import();
	$document_number = $imp->getLinkFormDocumentNumber($form_state);
	$purchase_order_number_field = $imp->getLinkFormPurchaseOrderNumbers($form_state);
	
	if (empty($purchase_order_number_field)) {
		form_set_error('error', t('One or more purchase order numbers required.'));
	}
	else {
		$numbers = $imp->splitLinkFormPurchaseOrderNumbers($purchase_order_number_field);
		$i = 0;
		foreach ($numbers as $number) {
			if (!preg_match('/^[0-9]+$/', $number)) {
				form_set_error('error_' . $i, t('Invalid purchase order number: @s.', array('@s' => $number)));
				$i++;
			}
		}
	}
}

function islandora_dm_import_link_documents_form_submit($form, &$form_state) {
	$imp = new Import();
	$imp->linkDocuments($form, $form_state);
}

function islandora_dm_import_incomplete_form() {
	$imp = new Import();
	$form = array();
	
	$form['foo'] = array(
			'#value' => '<p>Documents shown here include pages that have not finished all the processing that occurs on import, and which may not display correctly as a result. Use the options on this page to add them to the queue for re-processing.</p><p>Since the processing takes time, don\'t schedule documents that have just been imported, and don\'t schedule the same documents multiple times -- that will only slow the system down.</p>',
		);

	$options = $imp->incompleteDocumentsOptions();
	if (empty($options)) {
		$form['no_documents'] = array(
			'#type' => 'item', 
			'#value' => t('There are no incomplete documents.'),
		);
	}

	else {
		$form[Import::BTN_RESCHEDULE_ALL] = array(
			'#type' => 'submit',
			'#name' => Import::BTN_RESCHEDULE_ALL,
			'#value' => t('Reschedule All')
		);

		$form['reschedule_selected'] = array(
			'#type' => 'submit',
			'#name' => 'reschedule_selected',
			'#value' => t('Reschedule Selected')
		);

		$form['documents'] = array(
			'#type' => 'checkboxes',
			'#name' => 'documents',
			'#options' => $options,
			'#theme' => 'islandora_dm_import_incomplete_documents_checkboxes',
			'#description' => t('Select the documents to reschedule, or click the <i>Reschedule All</i> button.')
		);
	}
	
	return $form;
}

function theme_islandora_dm_import_incomplete_documents_checkboxes($form) {
	$documents = $form['#options'];
	$output = null;
	
	$header = array('', t('Document'), t('Pages'), t('Import Date'));

	$rows[] = array();
	foreach($documents as $k => $v) {
		$checkbox = array(
			'#type' => 'checkbox',
			'#name' => $form['#name'] . '[' . $k . ']',
		);
		
		$rows[] = array(
			drupal_render($checkbox),
			check_plain($v->title()),
			$v->pageCount(),
			$v->createdDate()
		);
	}

	$output = theme('table', $header, $rows);

	return $output;
}

function islandora_dm_import_incomplete_form_submit($form, &$form_state) {
	$imp = new Import();
	$imp->rescheduleIncomplete($form, $form_state);
}

