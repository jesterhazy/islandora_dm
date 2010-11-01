<?php

/**
 * Define menu items for this module
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
      'page callback' => 'drupal_get_form',
      'page arguments' => array('islandora_dm_import_numbered_form'),
      'access arguments' => array(Constants::PERMISSION_EDIT),
  );
  
  // import numbered documents
  $items['islandora_dm/import/numbered'] = array (
      'type' => MENU_DEFAULT_LOCAL_TASK,
      'title' => 'Numbered Files',
      'description' => 'Import files which have a purchase order number in the file name.',
      'weight' => -1,
      'access arguments' => array(Constants::PERMISSION_EDIT),
  );
  
  // import unnumbered documents
  $items['islandora_dm/import/unnumbered'] = array (
      'type' => MENU_LOCAL_TASK,
      'title' => 'Unnumbered Files',
      'description' => 'Import files which do not have purchase order number in the file name.',
      'weight' => 2,
      'page callback' => 'drupal_get_form',
      'page arguments' => array('islandora_dm_import_unnumbered_form'),
      'access arguments' => array(Constants::PERMISSION_EDIT),
  );
  
  // import invalid documents
  $items['islandora_dm/import/invalid'] = array (
      'type' => MENU_LOCAL_TASK,
      'title' => 'Invalid Files',
      'description' => 'View files in the import folder which have invalid file names.',
      'weight' => 3,
      'page callback' => 'islandora_dm_import_invalid',
      'access arguments' => array(Constants::PERMISSION_EDIT),
  );
  
  // search
  $items['islandora_dm/search'] = array (
      'title' => 'Search',
      'page callback' => 'islandora_dm_search',
      'access arguments' => array(Constants::PERMISSION_VIEW),
  );
  
  // my purchase orders
  $items['islandora_dm/search/owner/mine'] = array (
      'title' => 'My Purchase Orders',
      'page callback' => 'islandora_dm_search_owner_mine',
      'access arguments' => array(Constants::PERMISSION_VIEW),
      'weight' => 1,
  );
  
  // unassigned purchase orders
  $items['islandora_dm/search/owner/unassigned'] = array (
      'title' => 'Unassigned',
      'page callback' => 'islandora_dm_search_owner_unassigned',
      'access arguments' => array(Constants::PERMISSION_VIEW),
      'weight' => 2,
  );
  
  // processed
  $items['islandora_dm/search/status/processed'] = array (
      'title' => 'Processed',
      'page callback' => 'islandora_dm_search_status_processed',
      'access arguments' => array(Constants::PERMISSION_VIEW),
      'weight' => 3,
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
      'page arguments' => array('islandora_dm_add_note_form', 2),
      'access arguments' => array(Constants::PERMISSION_EDIT),
  );
  
  // add note to purchase order
  $items['islandora_dm/po/%/claim'] = array (
      'type' => MENU_CALLBACK,
      'title' => 'Claim Ownership of Purchase Order !id',
      'title arguments' => array('!id' => 2),
      'page callback' => 'DMPurchaseOrder::claim',
      'page arguments' => array(2),
      'access arguments' => array(Constants::PERMISSION_EDIT),
  );
  
  // add note to purchase order
  $items['islandora_dm/po/%/update'] = array (
      'type' => MENU_CALLBACK,
      'title' => 'Update Purchase Order !id',
      'title arguments' => array('!id' => 2),
      'page callback' => 'DMPurchaseOrder::update',
      'page arguments' => array(2),
      'access arguments' => array(Constants::PERMISSION_EDIT),
  );
  
  return $items;
}

?>