<?php

/**
 * Defines menu for the module.
 */
function islandora_dm_menu() {
	$items = array();

	// admin page
	$items['admin/settings/islandora_dm'] = array(
		'title' => 'Islandora Document Management',
		'description' => 'Configure the Islandora Document Management module.',
		'page callback' => 'drupal_get_form',
		'page arguments' => array('islandora_dm_admin'),
		'access arguments' => array(Constants::PERMISSION_ADMIN),
	);

	$items['admin/settings/islandora_dm/settings'] = array(
		'type' => MENU_DEFAULT_LOCAL_TASK,
		'weight' => -1,
		'title' => 'Settings',
		'access arguments' => array(Constants::PERMISSION_ADMIN),
	);

	$items['admin/settings/islandora_dm/objects'] = array(
		'type' => MENU_LOCAL_TASK,
		'title' => 'Fedora objects',
		'description' => 'Install the fedora objects used by this module.',
		'page callback' => 'drupal_get_form',
		'page arguments' => array('islandora_dm_admin_objects_form'),
		'access arguments' => array(Constants::PERMISSION_ADMIN),
	);

	// home page
	$items[Constants::MODULE_NAME] = array (
		'title' => 'Islandora Document Management',
		'page callback' => 'islandora_dm_home',
		'access arguments' => array(Constants::PERMISSION_VIEW),
	);

	// about page
	$items['islandora_dm/about'] = array (
		'title' => 'About',
		'page callback' => 'islandora_dm_about',
		'access arguments' => array(Constants::PERMISSION_VIEW),
	);

	// import documents
	$items['islandora_dm/import'] = array (
		'title' => 'Import Documents',
		'description' => 'Import documents.',
		'page callback' => 'drupal_goto',
		'page arguments' => array('islandora_dm/import/files'),
		'access arguments' => array(Constants::PERMISSION_EDIT),
	);

	$items['islandora_dm/import/files'] = array (
		'title' => 'Import Scanned Files',
		'description' => 'Import scanned files from the import folder.',
		'page callback' => 'drupal_get_form',
		'page arguments' => array('islandora_dm_import_files_form'),
		'access arguments' => array(Constants::PERMISSION_EDIT),
		'weight' => 1,
	);
	
	$items['islandora_dm/import/link_documents'] = array (
		'title' => 'Link Documents',
		'description' => 'Link imported documents to purchase orders.',
		'page callback' => 'drupal_get_form',
		'page arguments' => array('islandora_dm_import_link_documents_form'),
		'access arguments' => array(Constants::PERMISSION_EDIT),
		'weight' => 2,
	);
	
	$items['islandora_dm/import/incomplete'] = array (
		'title' => 'Incomplete Documents',
		'description' => 'View (and fix) documents which have not been completely ingested.',
		'page callback' => 'drupal_get_form',
		'page arguments' => array('islandora_dm_import_incomplete_form'),
		'access arguments' => array(Constants::PERMISSION_ADMIN),
		'weight' => 3,
	);
	
	// search
	$items['islandora_dm/search'] = array (
		'title' => 'Search',
		'page callback' => 'islandora_dm_search_page',
		'access arguments' => array(Constants::PERMISSION_VIEW),
	);
	
	// document view
	$items['islandora_dm/document/%/view'] = array (
		'type' => MENU_CALLBACK,
		'title' => 'Document !id',
		'title arguments' => array('!id' => 2),
		'page callback' => 'islandora_dm_document_view',
		'page arguments' => array(2),
		'access arguments' => array(Constants::PERMISSION_VIEW),
	);

	// purchase order view
	$items['islandora_dm/po/%/view'] = array (
		'type' => MENU_CALLBACK,
		'title' => 'Purchase Order !id',
		'title arguments' => array('!id' => 2),
		'page callback' => 'islandora_dm_po_view',
		'page arguments' => array(2),
		'access arguments' => array(Constants::PERMISSION_VIEW),
	);

	// add note to purchase order
	$items['islandora_dm/po/%/add_note'] = array (
		'type' => MENU_CALLBACK,
		'title' => 'Add Note to Purchase Order !id',
		'title arguments' => array('!id' => 2),
		'page callback' => 'drupal_get_form',
		'page arguments' => array('islandora_dm_add_note_form', 2, null),
		'access arguments' => array(Constants::PERMISSION_EDIT),
	);
	
	// add note to document (from the po view)
	$items['islandora_dm/po/%/document/%/add_note'] = array (
		'type' => MENU_CALLBACK,
		'title' => 'Add Note to Document !id',
		'title arguments' => array('!id' => 4),
		'page callback' => 'drupal_get_form',
		'page arguments' => array('islandora_dm_add_note_form', 2, 4),
		'access arguments' => array(Constants::PERMISSION_EDIT),
	);
	
	// add note to document (from the document view)
	$items['islandora_dm/document/%/add_note'] = array (
		'type' => MENU_CALLBACK,
		'title' => 'Add Note to Document !id',
		'title arguments' => array('!id' => 2),
		'page callback' => 'drupal_get_form',
		'page arguments' => array('islandora_dm_add_note_form', null, 2),
		'access arguments' => array(Constants::PERMISSION_EDIT),
	);
	
	// delete note
	$items['islandora_dm/%/%/note/%/delete'] = array (
	  'type' => MENU_CALLBACK,
	  'title' => 'Delete Note !id',
	  'title arguments' => array('!id' => 4),
	  'page callback' => 'islandora_dm_delete_note',
	  'page arguments' => array(4, 1, 2),
	  'access arguments' => array(Constants::PERMISSION_EDIT),
	);
	
	// update purchase order from uis
	$items['islandora_dm/po/%/update'] = array (
		'type' => MENU_CALLBACK,
		'title' => 'Update Purchase Order !id',
		'title arguments' => array('!id' => 2),
		'page callback' => 'islandora_dm_update_uis_data',
		'page arguments' => array(2),
		'access arguments' => array(Constants::PERMISSION_EDIT),
	);

	return $items;
}


function unimplemented_page($name) {
	$output = <<<END
<p>todo: implement $name page</p>
END;

	return $output;
}

function islandora_dm_home() {
	return unimplemented_page('home');
}

function islandora_dm_about() {	
	return unimplemented_page('about');
}

?>
