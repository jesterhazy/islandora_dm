<?php

/*
 * Callbacks for admin forms.
 */
function islandora_dm_admin() {
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
		'#default_value' => variable_get(Constants::IMPORT_ARCHIVE_DISABLED, FALSE),
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

function islandora_dm_admin_objects_form() {
	$form = array();
	$form['cmodels'] = array(
		'#type' => 'checkbox',
		'#title' => t('Content Models'),
		'#default_value' => 1,
		'#description' => t('Install or re-install the content models and default collection.'),
	);
	
	$form['examples'] = array(
		'#type' => 'checkbox',
		'#title' => t('Examples'),
		'#default_value' => 1,
		'#description' => t('Install or re-install the example objects.'),
	);

	$form['submit'] = array(
		'#type' => 'submit',
		'#name' => 'install',
		'#value' => t('Install selected')
	);

	return $form;
}

function islandora_dm_admin_objects_form_submit($form, &$form_state) {
	$objects_dir = drupal_get_path('module', Constants::MODULE_NAME) . '/etc/foxml';		
	$error = FALSE;

	try {
		if ($form_state['clicked_button']['#name'] == 'install' ) {
			if ($form_state['values']['cmodels']) {
				$cmodels_dir = $objects_dir . '/cmodels';
				$files = file_scan_directory($cmodels_dir, '\.xml$');

				foreach($files as $file) {
					ingest_object_from_foxml($file->filename);
				}
			}

			if ($form_state['values']['examples']) {
				$examples_dir = $objects_dir . '/examples';
				$files = file_scan_directory($examples_dir, '\.xml$');

				$letters = array('a', 'b', 'c');
	
				foreach($files as $file) {
					$item = ingest_object_from_foxml($file->filename);
		
					// filenames format for example page objects is example-p1.xml
					if (preg_match('/p-example-[0-9]+$/', $file->name)) {	
						$page_number = substr($file->name, -1);
						$letter = $letters[$page_number - 1];
						$item->add_datastream_from_file($examples_dir . '/jp2lossless/' . $letter . '.jp2', Constants::DSID_JP2_ARCHIVE, Constants::DSLABEL_JP2_ARCHIVE, Constants::MIME_TYPE_JP2_ARCHIVE);
						$item->add_datastream_from_file($examples_dir . '/jp2/' . $letter . '.jp2', Constants::DSID_JP2, Constants::DSLABEL_JP2, Constants::MIME_TYPE_JP2);
						$item->add_datastream_from_file($examples_dir . '/tn/' . $letter . '.jpg', Constants::DSID_TN, Constants::DSLABEL_TN, Constants::MIME_TYPE_TN);				
					}						
				}
			}
		}			
	}
	
	catch (Exception $e) {
		$error = TRUE;
		drupal_set_message(t($e->getMessage()), Constants::DRUPAL_MESSAGE_ERROR);
	}
	
	if (!$error) {
		drupal_set_message(t('Fedora objects (re)installed successfully.'));
	}
}


function ingest_object_from_foxml($file) {
	$pid = pid_from_file($file);
	
	// purge the item if it exists
	try {
		$item = new Fedora_Item($pid);
		
		if ($item->exists()) {
			$item->purge();
		}
	}
	
	catch (Exception $e) {
		// bury - means item didn't exist (good)
	}
 
	$item = Fedora_Item::ingest_from_FOXML_file($file);
	return $item;
}

function pid_from_file($file) {
	/* 
	* makes the unchecked assumption that filename matches: /path/<pid>.xml,
	* and that the pid inside the file actually matches the filename.
	*/
	return 'islandora-dm:' . basename($file, '.xml');
}
